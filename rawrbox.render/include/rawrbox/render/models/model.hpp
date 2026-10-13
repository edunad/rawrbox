#pragma once

#include <rawrbox/engine/static.hpp>
#include <rawrbox/render/lights/manager.hpp>
#include <rawrbox/render/models/animation.hpp>
#include <rawrbox/render/models/animations/blender.hpp>
#include <rawrbox/render/models/animations/sampler.hpp>
#include <rawrbox/render/models/base.hpp>
#include <rawrbox/render/models/instance.hpp>
#include <rawrbox/render/models/skeleton.hpp>
#include <rawrbox/render/models/utils/animation.hpp>
#include <rawrbox/render/models/utils/optimization.hpp>
#include <rawrbox/render/static.hpp>

#include <limits>
#include <unordered_set>

namespace rawrbox {

	template <typename M = rawrbox::MaterialUnlit>
		requires(std::derived_from<M, rawrbox::MaterialBase>)
	class Model : public rawrbox::ModelBase<M> {

	protected:
		// ANIMATION ---
		std::vector<rawrbox::Animation*> _animations = {};

		std::unordered_map<uint32_t, std::vector<rawrbox::Mesh<typename M::vertexBufferType>*>> _animatedMeshes = {}; // For quick lookup
		std::unordered_map<std::string, std::unique_ptr<rawrbox::AnimationSampler>> _playingAnimations = {};

		std::unordered_map<rawrbox::Skeleton*, std::unique_ptr<rawrbox::AnimationBlender>> _blenders = {};
		std::unordered_map<uint32_t, rawrbox::TransformBlend> _vertexBlends = {};
		std::unordered_map<uint32_t, std::pair<rawrbox::Matrix4x4, rawrbox::Matrix4x4>> _vertexSpaces = {};
		// ------------

		std::vector<std::unique_ptr<rawrbox::Mesh<typename M::vertexBufferType>>> _meshes = {};
		rawrbox::BBOX _bbox = {};
		std::vector<rawrbox::BBOX> _meshBounds = {};
		std::vector<std::vector<std::pair<uint32_t, rawrbox::BBOX>>> _boneBounds = {};

		// OCCLUSION ---
		Diligent::RefCntAutoPtr<Diligent::IBuffer> _cullMeshes;
		std::vector<rawrbox::Instance> _cullData = {};

		std::vector<rawrbox::BBOX> _frameBounds = {};
		std::vector<uint8_t> _inFrustum = {};
		// -----

		uint64_t _animationFrame = std::numeric_limits<uint64_t>::max();

		// LIGHTS ---
		std::vector<rawrbox::LightBase> _lights = {};
		// -----

		bool _canMerge = true;

		// ANIMATIONS ----
		virtual rawrbox::AnimationSampler* playAnimation(size_t index, rawrbox::Animation* animation, std::function<void(const std::string&)> onComplete = nullptr) {
			if (animation == nullptr || animation->empty()) return nullptr;

			if constexpr (!supportsBones<typename M::vertexBufferType>) {
				for (const auto& part : animation->getParts()) {
					if (part.type != rawrbox::AnimationType::SKELETON) continue;
					this->_logger->warn("Animation '{}' has a skeleton, but model does not support bones!", animation->name);
					break;
				}
			}

			auto sampler = std::make_unique<rawrbox::AnimationSampler>(index, animation, onComplete);
			auto* ptr = sampler.get();

			this->_playingAnimations[animation->name] = std::move(sampler);
			return ptr;
		}

		rawrbox::AnimationBlender* getBlender(rawrbox::Skeleton* skeleton) {
			auto fnd = this->_blenders.find(skeleton);
			if (fnd != this->_blenders.end()) return fnd->second.get();

			auto blender = std::make_unique<rawrbox::AnimationBlender>(skeleton);
			auto* ptr = blender.get();

			this->_blenders[skeleton] = std::move(blender);
			return ptr;
		}

		void collectAnimation(const rawrbox::AnimationSampler& sampler) {
			const float weight = sampler.getWeight();
			if (weight <= 0.F) return;

			const auto& parts = sampler.getAnimation()->getParts();
			for (size_t i = 0; i < parts.size(); i++) {
				const auto& part = parts[i];
				const auto& locals = sampler.getLocalOutput(i);

				switch (part.type) {
					case rawrbox::AnimationType::VERTEX:
						for (size_t track = 0; track < part.meshes.size(); track++) {
							if (!this->_animatedMeshes.contains(part.meshes[track])) continue;

							rawrbox::Vector3f pos = {};
							rawrbox::Vector4f rotation = {};
							rawrbox::Vector3f scale = {};

							rawrbox::AnimationUtils::getTransform(locals[track / 4], track % 4, pos, rotation, scale);
							this->_vertexBlends[part.meshes[track]].add(pos, rotation, scale, weight);
							if (track < part.parents.size()) this->_vertexSpaces[part.meshes[track]] = {part.parents[track], part.offsets[track]};
						}
						break;
					case rawrbox::AnimationType::SKELETON:
						if constexpr (supportsBones<typename M::vertexBufferType>) {
							this->getBlender(part.skeleton)->addLayer(locals, weight, sampler.getJointWeights(i), part.additive);
						}
						break;
				}
			}
		}

		void applyAnimations() {
			// Vertex ----
			for (auto& vertex : this->_vertexBlends) {
				auto fnd = this->_animatedMeshes.find(vertex.first);
				if (fnd == this->_animatedMeshes.end()) continue;

				const auto space = this->_vertexSpaces.find(vertex.first);
				const rawrbox::Matrix4x4 mtx = space == this->_vertexSpaces.end() ? vertex.second.toMatrix() : space->second.first * vertex.second.toMatrix() * space->second.second;
				for (auto* mesh : fnd->second) {
					mesh->matrix = mtx;
				}
			}
			// -------------

			// Skeleton ----
			if constexpr (supportsBones<typename M::vertexBufferType>) {
				for (auto& blender : this->_blenders) {
					if (blender.second->empty() && !blender.second->hasIK()) continue;
					this->applySkeleton(*blender.first, blender.second->blend());
				}
			}
			// -------------
		}

		void applySkeleton(const rawrbox::Skeleton& skeleton, const ozz::vector<ozz::math::Float4x4>& output) {
			const size_t skinJoints = skeleton.getNumSkinJoints();
			if (skinJoints > RB_RENDER_MAX_BONES_PER_MODEL) return;

			std::array<rawrbox::Matrix4x4, RB_RENDER_MAX_BONES_PER_MODEL> palette = {};
			for (size_t i = 0; i < skinJoints; i++) {
				const auto joint = static_cast<size_t>(skeleton.getJointIndex(i));
				palette[i] = rawrbox::AnimationUtils::toMatrix(output[joint]) * skeleton.getInverseBindMatrix(i);
			}

			for (auto& mesh : this->_meshes) {
				if (mesh == nullptr || mesh->skeleton != &skeleton) continue;
				std::copy(palette.begin(), palette.begin() + static_cast<std::ptrdiff_t>(skinJoints), mesh->boneTransforms.begin());
			}
		}

		void tickAnimations() {
			if (this->_animationFrame == rawrbox::FRAME) return;
			this->_animationFrame = rawrbox::FRAME;

			// Animation blending ----
			for (auto& blender : this->_blenders) {
				blender.second->reset();
			}

			this->_vertexBlends.clear();
			// -------------------

			std::vector<std::unique_ptr<rawrbox::AnimationSampler>> finished = {};
			for (auto it = this->_playingAnimations.begin(); it != this->_playingAnimations.end();) {
				auto& sampler = it->second;
				if (sampler == nullptr) {
					it = this->_playingAnimations.erase(it);
					continue;
				}

				const bool ended = sampler->tick(rawrbox::DELTA_TIME);

				sampler->sample();
				this->collectAnimation(*sampler);

				if (ended) {
					finished.push_back(std::move(sampler));
					it = this->_playingAnimations.erase(it);
					continue;
				}

				++it;
			}

			this->applyAnimations();

			for (auto& sampler : finished) {
				sampler->complete();
			}

			finished.clear();
		}
		// --------------

		void updateLights() {
			// Update lights ---
			if constexpr (supportsNormals<typename M::vertexBufferType>) {
				for (auto& mesh : this->meshes()) {
					// auto p = rawrbox::MathUtils::applyRotation(meshPos + this->getPos(), this->getAngle()); // TODO

					for (auto light : mesh->lights) {
						if (light == nullptr) continue;
						light->setOffsetPos(mesh->getPos() + this->getPos());
					}
				}
			}
		}

		// BLEND SHAPES ---
		void applyBlendShapes() override {
			rawrbox::ModelBase<M>::applyBlendShapes();

			this->flattenMeshes(false, false); // No need to optimize & sort, it's just vertex changes
			rawrbox::ModelBase<M>::updateBuffers();
		}
		// --------------

	public:
		Model(size_t vertices = 0, size_t indices = 0) : rawrbox::ModelBase<M>(vertices, indices) {};
		Model(const Model&) = delete;
		Model(Model&&) = delete;
		Model& operator=(const Model&) = delete;
		Model& operator=(Model&&) = delete;
		~Model() override {
			this->_playingAnimations.clear();
			this->_blenders.clear();
			this->_vertexBlends.clear();
			this->_animatedMeshes.clear();
			this->_animations.clear();
			this->_meshes.clear();
			this->_lights.clear();

			this->_boneBounds.clear();

			this->_inFrustum.clear();
			this->_frameBounds.clear();
			this->_cullData.clear();

			RAWRBOX_DESTROY(this->_cullMeshes);
		}

		virtual void flattenMeshes(bool merge = true, bool sort = true) {
			this->_mesh->clear();

			// Merge same meshes to reduce calls
			if (this->_canMerge && merge) {
				this->merge();
			}
			// ----------------------

			// Flatten meshes for buffers
			for (auto& mesh : this->_meshes) {
				if (mesh == nullptr || mesh->empty()) continue;

				// Fix start index ----
				mesh->baseIndex = static_cast<uint32_t>(this->_mesh->indices.size());
				mesh->baseVertex = static_cast<uint32_t>(this->_mesh->vertices.size());
				// --------------------

				// Append vertices
				this->_mesh->vertices.insert(this->_mesh->vertices.end(), mesh->vertices.begin(), mesh->vertices.end());
				this->_mesh->indices.insert(this->_mesh->indices.end(), mesh->indices.begin(), mesh->indices.end());
				// -----------------
			}
			// --------

			// Sort alpha
			if (sort) {
				std::stable_sort(this->_meshes.begin(), this->_meshes.end(), [](auto& a, auto& b) {
					return !a->isTransparent() && b->isTransparent();
				});
			}
			// --------

			// Shadow bounds ---
			this->_meshBounds.clear();
			for (auto& mesh : this->_meshes) {
				this->_meshBounds.push_back(mesh == nullptr ? rawrbox::BBOX::infinite() : ModelBase<M>::getVertexBounds(*mesh));
			}
			// --------

			// CALCULATE SKINNED BOUNDS ----
			this->_boneBounds.clear();
			this->_boneBounds.resize(this->_meshes.size());

			if constexpr (supportsBones<typename M::vertexBufferType>) {
				for (size_t i = 0; i < this->_meshes.size(); i++) {
					auto& mesh = this->_meshes[i];
					if (mesh == nullptr || mesh->skeleton == nullptr) continue;

					constexpr float high = std::numeric_limits<float>::max();
					std::vector<rawrbox::BBOX> boxes(RB_RENDER_MAX_BONES_PER_MODEL + 1, {{high, high, high}, {-high, -high, -high}, {}});

					for (const auto& vertex : mesh->vertices) {
						bool weighted = false;

						for (size_t k = 0; k < RB_MAX_BONES_PER_VERTEX; k++) {
							if (vertex.bone_weights[k] == 0) continue;

							boxes[vertex.bone_indices[k]].expand(vertex.position);
							weighted = true;
						}

						if (!weighted) boxes.back().expand(vertex.position);
					}

					for (size_t bone = 0; bone < boxes.size(); bone++) {
						if (boxes[bone].min.x <= boxes[bone].max.x) this->_boneBounds[i].emplace_back(static_cast<uint32_t>(bone), boxes[bone]);
					}
				}
			}
			// --------
		}

		virtual void setMergeable(bool status) { this->_canMerge = status; }

		// Note: this can be quite slow, use sparingly
		virtual void optimize(float complexity_threshold = 0.9F) {
			for (auto& mesh : this->_meshes) {
				if (mesh == nullptr || mesh->empty()) continue;

				size_t oldVerts = mesh->vertices.size();
				size_t oldInds = mesh->indices.size();

				rawrbox::MeshOptimization::optimize(mesh->vertices, mesh->indices);
				rawrbox::MeshOptimization::simplify(mesh->vertices, mesh->indices, complexity_threshold);

				if (oldVerts != mesh->vertices.size() || oldInds != mesh->indices.size()) {
					this->_logger->debug("Optimized mesh for rendering ({} -> {}), ideally you should optimize the model on a external tool.", fmt::styled(oldVerts, fmt::fg(fmt::color::cyan)), fmt::styled(mesh->vertices.size(), fmt::fg(fmt::color::cyan)));
				}
			}
		}

		virtual void merge() {
			const size_t old = this->_meshes.size();

			for (size_t i = 0; i < this->_meshes.size(); i++) {
				auto& target = this->_meshes[i];
				if (target == nullptr) continue;

				for (size_t j = i + 1; j < this->_meshes.size(); j++) {
					auto& other = this->_meshes[j];
					if (other == nullptr || !target->canMerge(*other)) continue;

					target->merge(*other);
					other.reset();
				}
			}

			std::erase(this->_meshes, nullptr);
			if (old != this->_meshes.size() && !this->isUploaded()) this->_logger->debug("Merged mesh for rendering ({} -> {})", fmt::styled(old, fmt::fg(fmt::color::cyan)), fmt::styled(this->_meshes.size(), fmt::fg(fmt::color::cyan))); // Only do it once
		}

		void updateBuffers() override {
			if (!this->isUploaded()) RAWRBOX_CRITICAL("Model is not uploaded!");
			if (!this->isDynamic()) RAWRBOX_CRITICAL("Model is not dynamic!");

			this->flattenMeshes();
			rawrbox::ModelBase<M>::updateBuffers();

			const auto meshes = static_cast<uint32_t>(this->_meshes.size());
			this->createCullBuffers(meshes, meshes * 2U);
		}

		// ANIMATIONS ----
		virtual bool blendAnimation(const std::string& name, float weight) {
			auto fnd = this->_playingAnimations.find(name);
			if (fnd == this->_playingAnimations.end() || fnd->second == nullptr) return false;

			fnd->second->setWeight(weight);
			return true;
		}

		virtual std::vector<rawrbox::AnimationSampler*> playAnimation(bool loop = false, std::function<void(const std::string&)> onComplete = nullptr) {
			std::vector<rawrbox::AnimationSampler*> anims = {};

			this->stopAllAnimations();
			for (size_t i = 0; i < this->_animations.size(); i++) {
				auto* anim = this->playAnimation(i, this->_animations[i], onComplete);
				if (anim == nullptr) continue;

				anim->setLoop(loop);
				anims.emplace_back(anim);
			}

			return anims;
		}

		virtual rawrbox::AnimationSampler* playAnimation(const std::string& name, bool loop = false, std::function<void(const std::string&)> onComplete = nullptr, float weight = 1.F) {
			for (size_t i = 0; i < this->_animations.size(); i++) {
				rawrbox::Animation* anim = this->_animations[i];
				if (anim == nullptr || anim->name != name) continue;

				if (this->isAnimationPlaying(name)) return nullptr; // Already playing

				auto* playingAnim = this->playAnimation(i, anim, onComplete);
				if (playingAnim == nullptr) return nullptr;

				playingAnim->setLoop(loop);
				playingAnim->setWeight(weight);
				return playingAnim;
			}

			this->_logger->warn("Failed to find animation {}", name);
			return nullptr;
		}

		virtual rawrbox::AnimationSampler* playSingleAnimation(const std::string& name, bool loop = false, std::function<void(const std::string&)> onComplete = nullptr) {
			this->stopAllAnimations();
			return this->playAnimation(name, loop, onComplete);
		}

		template <typename T>
			requires(std::derived_from<T, rawrbox::AnimationIKBase>)
		bool addIK(const std::string& joint, const T& ik) {
			if constexpr (!supportsBones<typename M::vertexBufferType>) {
				return false;
			} else {
				bool found = false;
				std::unordered_set<rawrbox::Skeleton*> done = {};

				for (auto& mesh : this->_meshes) {
					if (mesh == nullptr || mesh->skeleton == nullptr || !done.insert(mesh->skeleton).second) continue;

					rawrbox::Matrix4x4 toModel = this->getMatrix() * mesh->matrix;
					toModel.inverse();

					T local = ik;
					local.transform(toModel);

					if (this->getBlender(mesh->skeleton)->addIK(joint, local)) found = true;
				}

				if (!found) this->_logger->warn("Failed to setup IK on '{}'", joint);
				return found;
			}
		}

		virtual void removeIK(const std::string& joint) {
			for (auto& blender : this->_blenders) {
				blender.second->removeIK(joint);
			}
		}

		virtual void clearIK() {
			for (auto& blender : this->_blenders) {
				blender.second->clearIK();
			}
		}

		[[nodiscard]] virtual const rawrbox::AnimationIKBase* getIK(const std::string& joint) const {
			for (const auto& blender : this->_blenders) {
				const auto* ik = blender.second->getIK(joint);
				if (ik != nullptr) return ik;
			}

			return nullptr;
		}

		virtual void stopAllAnimations() {
			this->_playingAnimations.clear();
		}

		virtual bool hasAnimation(const std::string& name) {
			return std::any_of(this->_animations.begin(), this->_animations.end(), [&name](const rawrbox::Animation* anim) {
				return anim != nullptr && anim->name == name;
			});
		}

		virtual bool stopAnimation(const std::string& name) {
			auto fnd = this->_playingAnimations.find(name);
			if (fnd == this->_playingAnimations.end()) return false;

			this->_playingAnimations.erase(fnd);
			return true;
		}

		virtual const std::vector<rawrbox::Animation*>& getAnimations() {
			return this->_animations;
		}

		virtual bool isAnimationPlaying(const std::string& name) {
			return this->_playingAnimations.contains(name);
		}
		// --------------

		// LIGHTS ------
		template <typename T = rawrbox::LightBase, typename... CallbackArgs>
			requires(std::derived_from<T, rawrbox::LightBase>)
		T* addLight(std::optional<size_t> parentIndex, CallbackArgs&&... args) {
			if constexpr (supportsNormals<typename M::vertexBufferType>) {
				auto* parent = this->_meshes.back().get();
				if (parentIndex.has_value() && parentIndex.value() < this->_meshes.size()) {
					parent = this->_meshes[parentIndex.value()].get();
				}

				auto light = rawrbox::LIGHTS::add<T>(std::forward<CallbackArgs>(args)...);
				light->setOffsetPos(parent->getPos() + this->getPos());
				parent->lights.push_back(light);

				return dynamic_cast<T*>(light);
			} else {
				return nullptr;
			}
		}
		// -----
		void setPos(const rawrbox::Vector3f& pos) override {
			rawrbox::ModelBase<M>::setPos(pos);
			this->updateLights();
		}

		void setAngle(const rawrbox::Vector4f& angle) override {
			rawrbox::ModelBase<M>::setAngle(angle);
			this->updateLights();
		}

		void setEulerAngle(const rawrbox::Vector3f& angle) override {
			rawrbox::ModelBase<M>::setEulerAngle(angle);
			this->updateLights();
		}

		void setScale(const rawrbox::Vector3f& size) override {
			rawrbox::ModelBase<M>::setScale(size);
			this->updateLights();
		}

		[[nodiscard]] virtual rawrbox::BBOX getBBOX() { return this->_bbox * this->getScale(); }
		[[nodiscard]] virtual size_t totalMeshes() const {
			return this->_meshes.size();
		}
		[[nodiscard]] virtual bool empty() const {
			return this->_meshes.empty();
		}

		virtual void removeMeshIf(const std::function<bool(const rawrbox::Mesh<typename M::vertexBufferType>&)>& pred) {
			std::unordered_set<const rawrbox::Mesh<typename M::vertexBufferType>*> removed = {};

			std::erase_if(this->_meshes, [&pred, &removed](const auto& mesh) {
				if (mesh == nullptr || !pred(*mesh)) return false;

				removed.insert(mesh.get());
				return true;
			});

			if (removed.empty()) return;
			this->_meshBounds.clear();

			for (auto it = this->_animatedMeshes.begin(); it != this->_animatedMeshes.end();) {
				std::erase_if(it->second, [&removed](auto* mesh) { return removed.contains(mesh); });
				it = it->second.empty() ? this->_animatedMeshes.erase(it) : std::next(it);
			}

			for (auto& shape : this->_blend_shapes) {
				if (removed.contains(shape.second->mesh)) shape.second->mesh = nullptr;
			}
		}

		virtual void removeMeshByName(const std::string& id) {
			this->removeMeshIf([&id](const auto& mesh) {
				return mesh.getName() == id;
			});
		}

		virtual void removeMesh(size_t index) {
			if (index >= this->_meshes.size()) return;

			const auto* target = this->_meshes[index].get();
			this->removeMeshIf([target](const auto& mesh) {
				return &mesh == target;
			});
		}

		virtual rawrbox::Mesh<typename M::vertexBufferType>* addMesh(rawrbox::Mesh<typename M::vertexBufferType> mesh) {
			this->_bbox.combine(mesh.getBBOX());
			mesh.owner = this;

			auto& a = this->_meshes.emplace_back(std::make_unique<rawrbox::Mesh<typename M::vertexBufferType>>(mesh));
			return a.get();
		}

		virtual rawrbox::Mesh<typename M::vertexBufferType>* addMesh(size_t index, rawrbox::Mesh<typename M::vertexBufferType> mesh) {
			if (index >= this->_meshes.size()) RAWRBOX_CRITICAL("Index out of bounds");

			this->_bbox.combine(mesh.getBBOX());
			mesh.owner = this;

			this->_meshes[index] = std::make_unique<rawrbox::Mesh<typename M::vertexBufferType>>(mesh);
			return this->_meshes[index].get();
		}

		virtual rawrbox::Mesh<typename M::vertexBufferType>* getMeshByName(const std::string& id) {
			auto fnd = std::find_if(this->_meshes.begin(), this->_meshes.end(), [&id](auto& mesh) { return mesh->getName() == id; });
			if (fnd == this->_meshes.end()) return nullptr;

			return (*fnd).get();
		}

		virtual rawrbox::Mesh<typename M::vertexBufferType>* getMesh(size_t id = 0) {
			if (!this->hasMesh(id)) return nullptr;
			return this->_meshes[id].get();
		}

		virtual bool hasMesh(size_t index) {
			return index < this->_meshes.size();
		}

		[[nodiscard]] uint32_t getID(int index = 0) const override {
			return this->_meshes[index]->getID();
		}

		void setID(uint32_t id, int index = -1) override {
			for (size_t i = 0; i < this->_meshes.size(); i++) {
				if (index != -1 && i != static_cast<size_t>(index)) continue;
				this->_meshes[i]->setID(id);
			}
		}

		virtual void setCulling(Diligent::CULL_MODE cull, int index = -1) {
			for (size_t i = 0; i < this->_meshes.size(); i++) {
				if (index != -1 && i != static_cast<size_t>(index)) continue;
				this->_meshes[i]->setCulling(cull);
			}
		}

		virtual void setWireframe(bool wireframe, int index = -1) {
			for (size_t i = 0; i < this->_meshes.size(); i++) {
				if (index != -1 && i != static_cast<size_t>(index)) continue;
				this->_meshes[i]->setWireframe(wireframe);
			}
		}

		virtual void setTexture(rawrbox::TextureBase* tex, int index = -1) {
			for (size_t i = 0; i < this->_meshes.size(); i++) {
				if (index != -1 && i != static_cast<size_t>(index)) continue;
				this->_meshes[i]->setTexture(tex);
			}
		}

		void setColor(const rawrbox::Color& color, int index = -1) override {
			for (size_t i = 0; i < this->_meshes.size(); i++) {
				if (index != -1 && i != static_cast<size_t>(index)) continue;
				this->_meshes[i]->setColor(color);
			}
		}

		[[nodiscard]] const rawrbox::Color& getColor(int index = 0) const override {
			return this->_meshes[index]->getColor();
		}

		virtual std::vector<std::unique_ptr<rawrbox::Mesh<typename M::vertexBufferType>>>& meshes() {
			return this->_meshes;
		}

		void upload(rawrbox::UploadType type = rawrbox::UploadType::STATIC) override {
			this->flattenMeshes(); // Merge and optimize meshes for drawing
			ModelBase<M>::upload(type);

			const auto meshes = static_cast<uint32_t>(this->_meshes.size());
			this->createCullBuffers(meshes, meshes * 2U);
		}

		// SHADOWS ---
		[[nodiscard]] rawrbox::BBOX getShadowBounds() override {
			if (this->_meshBounds.size() != this->_meshes.size()) return rawrbox::BBOX::infinite();

			rawrbox::BBOX bounds = {};
			bool empty = true;

			for (size_t i = 0; i < this->_meshes.size(); i++) {
				auto& mesh = this->_meshes[i];
				if (mesh == nullptr || mesh->empty()) continue;

				const auto meshBounds = this->_meshBounds[i].transform(this->getMatrix() * mesh->getMatrix());
				if (empty) {
					bounds = meshBounds;
				} else {
					bounds.combine(meshBounds);
				}

				empty = false;
			}

			return empty ? rawrbox::BBOX::infinite() : bounds;
		}

		void drawShadow(const rawrbox::ShadowDraw& draw) override {
			if (!this->canCastShadows()) return;
			this->tickAnimations();

			if (draw.bind) {
				this->bindBuffers();
				this->_material->resetUniformBinds();
			}

			// CPU CULL ----
			for (size_t i = 0; i < this->_meshes.size(); i++) {
				auto& mesh = this->_meshes[i];
				if (mesh == nullptr || mesh->empty() || !this->canCastShadows(*mesh)) continue;

				const auto matrix = this->getMatrix() * mesh->getMatrix();
				const auto bounds = i < this->_meshBounds.size() ? this->_meshBounds[i].transform(matrix) : rawrbox::BBOX::infinite();
				if (draw.isVisible && !draw.isVisible(bounds)) continue;

				if (draw.bind) this->bindShadowMesh(*mesh, matrix);
				draw.submit({bounds, mesh->totalIndex, mesh->baseIndex, mesh->baseVertex, 1U, rawrbox::ShadowDraw::ALL_VIEWS});
			}
			// ------------
		}
		// -----------

	protected:
		[[nodiscard]] virtual bool isCullable(size_t index) const {
			return index < this->_meshBounds.size() && this->_meshes[index]->isCullable();
		}

		// CULL  ---
		void createCullBuffers(const uint32_t items, const uint32_t draws) override {
			rawrbox::ModelBase<M>::createCullBuffers(items, draws);
			RAWRBOX_DESTROY(this->_cullMeshes);

			this->_cullData.assign(items, {});
			this->_frameBounds.assign(items, {});
			this->_inFrustum.assign(items, 1U);

			for (uint32_t i = 0; i < items; i++) {
				auto& mesh = this->_meshes[i];
				if (mesh == nullptr) continue;

				for (uint32_t pass = 0; pass < 2; pass++) {
					const size_t record = (static_cast<size_t>(pass) * items + i) * this->DRAW_ARGS;

					this->_cullArgs[record] = mesh->totalIndex;
					this->_cullArgs[record + 2] = mesh->baseIndex;
					this->_cullArgs[record + 3] = mesh->baseVertex;
				}
			}

			Diligent::BufferDesc itemsDesc;
			itemsDesc.Name = "RawrBox::Buffer::Cull::Meshes";
			itemsDesc.Usage = Diligent::USAGE_DEFAULT;
			itemsDesc.Mode = Diligent::BUFFER_MODE_STRUCTURED;
			itemsDesc.BindFlags = Diligent::BIND_SHADER_RESOURCE;
			itemsDesc.ElementByteStride = sizeof(rawrbox::Instance);
			itemsDesc.Size = static_cast<uint64_t>(std::max(items, 1U)) * sizeof(rawrbox::Instance); // Needs at least 1

			rawrbox::RENDERER->device()->CreateBuffer(itemsDesc, nullptr, &this->_cullMeshes);
			if (this->_cullMeshes == nullptr) RAWRBOX_CRITICAL("Failed to create culling items buffer");

			rawrbox::RENDERER->context()->UpdateBuffer(this->_argsBuffer, 0, this->_cullArgs.size() * sizeof(uint32_t), this->_cullArgs.data(), Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
		}

		virtual void cullMeshes(rawrbox::CullPass mode) {
			const auto total = static_cast<uint32_t>(this->_cullData.size());
			if (total == 0) return;

			// CPU FRUSTUM CULL ----
			if (mode != rawrbox::CullPass::LATE) {
				for (uint32_t i = 0; i < total; i++) {
					auto* mesh = i < this->_meshes.size() ? this->_meshes[i].get() : nullptr;

					auto& item = this->_cullData[i];
					item = {};
					item.data.w = 1U;

					if (mesh == nullptr || mesh->empty() || !this->isCullable(i)) continue;

					const auto& bounds = this->_frameBounds[i];
					if (bounds.isInfinite() || this->_inFrustum[i] == 0U || mesh->isTransparent()) continue;

					item.setMatrix(mesh->getMatrix() * rawrbox::Matrix4x4::mtxSRT(bounds.size * 0.5F, {0.F, 0.F, 0.F, 1.F}, (bounds.min + bounds.max) * 0.5F));
					item.data.w = 0U;
				}

				rawrbox::RENDERER->context()->UpdateBuffer(this->_cullMeshes, 0, static_cast<uint64_t>(total) * sizeof(rawrbox::Instance), this->_cullData.data(), Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
			}
			// ----------------

			this->cullObjects(this->_cullMeshes, total, {{-1.F, -1.F, -1.F}, {1.F, 1.F, 1.F}, {2.F, 2.F, 2.F}}, mode);
		}
		// -----------

		// DRAW (NO TRANSPARENT) ---
		virtual void drawMeshes(rawrbox::CullPass pass) {
			const auto total = this->_cullData.size();

			for (size_t i = 0; i < std::min(total, this->_meshes.size()); i++) {
				auto& mesh = this->_meshes[i];
				if (mesh == nullptr || mesh->empty() || this->_inFrustum[i] == 0U) continue; // Bone, skip it.

				const bool occluded = pass != rawrbox::CullPass::FRUSTUM && this->isCullable(i) && !mesh->isTransparent() && !this->_frameBounds[i].isInfinite();

				if (pass == rawrbox::CullPass::EARLY && mesh->isTransparent()) continue;
				if (pass == rawrbox::CullPass::LATE && !occluded && !mesh->isTransparent()) continue;

				const auto matrix = this->getMatrix() * mesh->getMatrix();

				// Bind pipelines ----
				this->_material->bindPipeline(*mesh);
				// -------------------

				// Update uniforms -----
				bool buffersUpdated = this->_material->bindVertexUniforms(*mesh);
				buffersUpdated |= this->_material->bindVertexSkinnedUniforms(*mesh);
				buffersUpdated |= this->_material->bindPixelUniforms(*mesh);

				rawrbox::RENDERER->getPassCamera()->setModelTransform(matrix);
				// -----------

				// DRAW -------
				auto flags = Diligent::DRAW_FLAG_VERIFY_ALL;
				if (!buffersUpdated) flags |= Diligent::DRAW_FLAG_DYNAMIC_RESOURCE_BUFFERS_INTACT;

				if (occluded) {
					Diligent::DrawIndexedIndirectAttribs attribs;
					attribs.pAttribsBuffer = this->_argsBuffer;
					attribs.DrawArgsOffset = ((pass == rawrbox::CullPass::LATE ? total : 0U) + i) * this->DRAW_ARGS * sizeof(uint32_t);
					attribs.IndexType = Diligent::VT_UINT32;
					attribs.Flags = flags;
					attribs.AttribsBufferStateTransitionMode = Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION;

					rawrbox::RENDERER->context()->DrawIndexedIndirect(attribs);
					continue;
				}

				Diligent::DrawIndexedAttribs DrawAttrs;
				DrawAttrs.IndexType = Diligent::VT_UINT32;
				DrawAttrs.FirstIndexLocation = mesh->baseIndex;
				DrawAttrs.BaseVertex = mesh->baseVertex;
				DrawAttrs.NumIndices = mesh->totalIndex;
				DrawAttrs.Flags = flags;

				rawrbox::RENDERER->context()->DrawIndexed(DrawAttrs);
				// -----------
			}
		}
		// -----------

	public:
		void draw() override {
			ModelBase<M>::draw();
			this->tickAnimations();

			// FRUSTUM ---
			const auto viewProj = rawrbox::RENDERER->getPassCamera()->getViewProjMtx();
			const auto total = std::min(this->_meshes.size(), this->_inFrustum.size());

			std::ranges::fill(this->_inFrustum, 1U);

			for (size_t i = 0; i < total; i++) {
				auto& mesh = this->_meshes[i];
				if (mesh == nullptr || mesh->empty() || !this->isCullable(i)) continue;

				auto& bounds = this->_frameBounds[i];
				bounds = this->_meshBounds[i];

				const auto& bones = this->_boneBounds[i];
				for (size_t bone = 0; bone < bones.size(); bone++) {
					const auto& [joint, box] = bones[bone];
					const auto posed = joint < RB_RENDER_MAX_BONES_PER_MODEL ? box.transform(mesh->boneTransforms[joint]) : box;

					if (bone == 0) {
						bounds = posed;
					} else {
						bounds.combine(posed);
					}
				}

				this->_inFrustum[i] = bounds.inFrustum(viewProj * this->getMatrix() * mesh->getMatrix()) ? 1U : 0U;
			}
			// -----------

			// OCCLUSION ---
			auto* occlusion = rawrbox::RENDERER->occlusion();
			if (!occlusion->isRecording()) {
				this->drawMeshes(rawrbox::CullPass::FRUSTUM);
				return;
			}

			this->cullMeshes(rawrbox::CullPass::EARLY);
			this->drawMeshes(rawrbox::CullPass::EARLY);

			occlusion->queue(*this);
			// -----------
		}

		void drawLate() override {
			ModelBase<M>::draw();

			this->cullMeshes(rawrbox::CullPass::LATE);
			this->drawMeshes(rawrbox::CullPass::LATE);
		}
	};
} // namespace rawrbox