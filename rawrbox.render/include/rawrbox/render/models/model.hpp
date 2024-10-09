#pragma once

#include <rawrbox/engine/static.hpp>
#include <rawrbox/render/lights/manager.hpp>
#include <rawrbox/render/models/animations/skeleton.hpp>
#include <rawrbox/render/models/animations/vertex.hpp>
#include <rawrbox/render/models/base.hpp>
#include <rawrbox/render/models/utils/optimization.hpp>
#include <rawrbox/render/static.hpp>

namespace rawrbox {

	template <typename M = rawrbox::MaterialUnlit>
		requires(std::derived_from<M, rawrbox::MaterialBase>)
	class Model : public rawrbox::ModelBase<M> {

	protected:
		// ANIMATION ---
		std::vector<ozz::animation::Animation*> _animations = {};

		std::unordered_map<size_t, std::vector<rawrbox::Mesh<typename M::vertexBufferType>*>> _vertexAnimations = {}; // For quick lookup
		std::unordered_map<std::string, std::unique_ptr<rawrbox::AnimationSampler>> _playingAnimations = {};
		// ------------

		std::vector<std::unique_ptr<rawrbox::Mesh<typename M::vertexBufferType>>> _meshes = {};
		rawrbox::BBOX _bbox = {};

		// LIGHTS ---
		std::vector<rawrbox::LightBase> _lights = {};
		// -----

		bool _canMerge = true;

		// ANIMATIONS ----
		virtual rawrbox::AnimationSampler* playAnimation(size_t index, ozz::animation::Animation* animation, std::function<void(const std::string&)> onComplete = nullptr) {
			if (animation == nullptr) return nullptr;

			auto name = animation->name();
			switch (animation->type) {
				case ozz::animation::VERTEX:
					this->_playingAnimations[name] = std::make_unique<rawrbox::AnimationVertexSampler>(index, animation, onComplete);
					return this->_playingAnimations[name].get();
				case ozz::animation::SKELETON:
					if constexpr (supportsBones<typename M::vertexBufferType>) {
						this->_playingAnimations[name] = std::make_unique<rawrbox::AnimationSkeletonSampler>(index, animation, onComplete);
						return this->_playingAnimations[name].get();
					} else {
						this->_logger->warn("Failed to play animation {}, model does not support bones", name);
					}
					break;
				case ozz::animation::UNKNOWN:
				default:
					this->_logger->warn("Unknown animation type, skipping");
					break;
			}

			return nullptr;
		}

		void sampleAnimations(rawrbox::AnimationSampler* sample) const {
			switch (sample->getType()) {
				case ozz::animation::VERTEX:
					this->processVertexAnimation(dynamic_cast<rawrbox::AnimationVertexSampler*>(sample));
					break;

				case ozz::animation::SKELETON:
					if constexpr (supportsBones<typename M::vertexBufferType>) {
						this->processSkeletonAnimations(dynamic_cast<rawrbox::AnimationSkeletonSampler*>(sample));
					} else {
						CRITICAL_RAWRBOX("Failed to play animation {}, model does not support bones", sample->getAnimation()->name());
					}
					break;

				default:
				case ozz::animation::UNKNOWN:
					CRITICAL_RAWRBOX("Unknown animation type");
			}
		}

		void processVertexAnimation(rawrbox::AnimationVertexSampler* sample) const {
			if (sample == nullptr) return;

			const auto& outputs = sample->getOutput();
			for (size_t i = 0; i < outputs.size(); i++) {
				const auto& output = outputs[i];

				auto fnd = this->_vertexAnimations.find(sample->getIndex());
				if (fnd == this->_vertexAnimations.end()) return;

				for (auto& mesh : fnd->second) {
					rawrbox::Vector3f position = {ozz::math::GetX(output.translation.x), ozz::math::GetX(output.translation.y), ozz::math::GetX(output.translation.z)};
					rawrbox::Vector4f rotation = {ozz::math::GetX(output.rotation.x), ozz::math::GetX(output.rotation.y), ozz::math::GetX(output.rotation.z), ozz::math::GetX(output.rotation.w)};
					rawrbox::Vector3f scale = {ozz::math::GetX(output.scale.x), ozz::math::GetX(output.scale.y), ozz::math::GetX(output.scale.z)};

					mesh->matrix = rawrbox::Matrix4x4::mtxSRT(scale, rotation, position);
				}
			}
		}

		void processSkeletonAnimations(rawrbox::AnimationSkeletonSampler* sample) const {
			if (sample == nullptr) return;

			for (auto& mesh : this->_meshes) {
				const ozz::animation::Skeleton* skeleton = mesh->skeleton;
				if (skeleton == nullptr) continue;

				rawrbox::Matrix4x4 globalMtx = rawrbox::Matrix4x4(skeleton->inverseBindMatrices[0]);
				globalMtx.inverse();

				const ozz::vector<ozz::math::Float4x4>& modelOutput = sample->getOutput(skeleton);
				for (size_t i = 0; i < modelOutput.size(); i++) {
					const ozz::math::Float4x4& output = modelOutput[i];

					rawrbox::Matrix4x4 invMtx = rawrbox::Matrix4x4(skeleton->inverseBindMatrices[i]);
					rawrbox::Matrix4x4 mtx = rawrbox::Matrix4x4({ozz::math::GetX(output.cols[0]), ozz::math::GetY(output.cols[0]), ozz::math::GetZ(output.cols[0]), ozz::math::GetW(output.cols[0]), ozz::math::GetX(output.cols[1]), ozz::math::GetY(output.cols[1]), ozz::math::GetZ(output.cols[1]), ozz::math::GetW(output.cols[1]), ozz::math::GetX(output.cols[2]), ozz::math::GetY(output.cols[2]), ozz::math::GetZ(output.cols[2]), ozz::math::GetW(output.cols[2]), ozz::math::GetX(output.cols[3]), ozz::math::GetY(output.cols[3]), ozz::math::GetZ(output.cols[3]), ozz::math::GetW(output.cols[3])});

					mesh->boneTransforms[i] = globalMtx * mtx * invMtx;
				}
			}
		}

		void tickAnimations() {
			for (auto it = this->_playingAnimations.begin(); it != this->_playingAnimations.end();) {
				const auto& anim = it->second;
				if (anim == nullptr) continue;

				if (anim->tick(rawrbox::DELTA_TIME)) {
					it = this->_playingAnimations.erase(it);
					continue;
				} else {
					this->sampleAnimations(anim.get());
				}

				++it;
			}
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
			this->_meshes.clear();
			this->_vertexAnimations.clear();
			this->_animations.clear();
			this->_playingAnimations.clear();
			this->_lights.clear();
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
				std::sort(this->_meshes.begin(), this->_meshes.end(), [](auto& a, auto& b) {
					return !a->isTransparent() && b->isTransparent();
				});
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
			size_t old = this->_meshes.size();

			for (size_t i1 = 0; i1 < this->_meshes.size(); i1++) {
				auto& mesh1 = this->_meshes[i1];

				// figure out how big our buffers will get
				size_t reserveVertices = mesh1->vertices.size();
				size_t reserveIndices = mesh1->indices.size();

				for (size_t i2 = this->_meshes.size() - 1; i2 > i1; i2--) {
					auto& mesh2 = this->_meshes[i2];
					if (!mesh1->canMerge(*mesh2)) continue;

					reserveVertices += mesh2->vertices.size();
					reserveIndices += mesh2->indices.size();
				}

				if (reserveVertices == mesh1->vertices.size()) continue;

				mesh1->vertices.reserve(reserveVertices);
				mesh1->indices.reserve(reserveIndices);

				// merge what it can
				for (size_t i2 = this->_meshes.size() - 1; i2 > i1; i2--) {
					auto& mesh2 = this->_meshes[i2];
					if (!mesh1->canMerge(*mesh2)) continue;

					mesh1->merge(*mesh2);
					this->_meshes.erase(this->_meshes.begin() + i2);
				}
			}

			if (old != this->_meshes.size() && !this->isUploaded()) this->_logger->debug("Merged mesh for rendering ({} -> {})", fmt::styled(old, fmt::fg(fmt::color::cyan)), fmt::styled(this->_meshes.size(), fmt::fg(fmt::color::cyan))); // Only do it once
		}

		void updateBuffers() override {
			if (!this->isUploaded()) CRITICAL_RAWRBOX("Model is not uploaded!");
			if (!this->isDynamic()) CRITICAL_RAWRBOX("Model is not dynamic!");

			this->flattenMeshes();
			rawrbox::ModelBase<M>::updateBuffers();
		}

		// ANIMATIONS ----
		virtual bool blendAnimation(const std::string& /*otherAnim*/, float /*blend*/) {
			CRITICAL_RAWRBOX("TODO");
		}

		virtual std::vector<rawrbox::AnimationSampler*> playAnimation(std::function<void(const std::string&)> onComplete = nullptr) {
			std::vector<rawrbox::AnimationSampler*> anims = {};

			this->stopAllAnimations();
			for (size_t i = 0; i < this->_animations.size(); i++) {
				anims.push_back(this->playAnimation(i, this->_animations[i], onComplete));
			}

			return anims;
		}

		virtual rawrbox::AnimationSampler* playAnimation(const std::string& name, bool forceSingle = false, std::function<void(const std::string&)> onComplete = nullptr) {
			for (size_t i = 0; i < this->_animations.size(); i++) {
				ozz::animation::Animation* anim = this->_animations[i];
				if (anim == nullptr || anim->name() != name) continue;

				auto fnd = this->_playingAnimations.find(name);
				if (fnd != this->_playingAnimations.end()) return nullptr; // Already playing

				if (forceSingle) this->stopAllAnimations();
				return this->playAnimation(i, anim, onComplete);
			}

			this->_logger->warn("Failed to find animation {}", name);
			return nullptr;
		}

		virtual void stopAllAnimations() {
			this->_playingAnimations.clear();
		}

		virtual bool hasAnimation(const std::string& name) {
			return this->_playingAnimations.contains(name);
		}

		virtual bool stopAnimation(const std::string& name) {
			auto fnd = this->_playingAnimations.find(name);
			if (fnd == this->_playingAnimations.end()) return false;

			this->_playingAnimations.erase(fnd);
			return true;
		}

		virtual const std::vector<ozz::animation::Animation*>& getAnimations() {
			return this->_animations;
		}

		virtual bool isAnimationPlaying(const std::string& name) {
			return this->_playingAnimations.find(name) != this->_playingAnimations.end();
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

		[[nodiscard]] virtual const rawrbox::BBOX& getBBOX() const { return this->_bbox; }
		[[nodiscard]] virtual size_t totalMeshes() const {
			return this->_meshes.size();
		}
		[[nodiscard]] virtual bool empty() const {
			return this->_meshes.empty();
		}

		virtual void removeMeshByName(const std::string& id) {
			for (auto it = this->_meshes.begin(); it != this->_meshes.end();) {
				if ((*it)->getName() == id) {
					it = this->_meshes.erase(it);
					continue;
				}

				++it;
			}
		}

		virtual void removeMesh(size_t index) {
			if (index >= this->_meshes.size()) return;
			this->_meshes.erase(this->_meshes.begin() + index);
		}

		virtual rawrbox::Mesh<typename M::vertexBufferType>* addMesh(rawrbox::Mesh<typename M::vertexBufferType> mesh) {
			this->_bbox.combine(mesh.getBBOX());
			mesh.owner = this;

			auto& a = this->_meshes.emplace_back(std::make_unique<rawrbox::Mesh<typename M::vertexBufferType>>(mesh));
			return a.get();
		}

		virtual rawrbox::Mesh<typename M::vertexBufferType>* addMesh(size_t index, rawrbox::Mesh<typename M::vertexBufferType> mesh) {
			if (index >= this->_meshes.size()) CRITICAL_RAWRBOX("Index out of bounds");

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
		}

		void draw() override {
			ModelBase<M>::draw();
			this->tickAnimations();

			for (auto& mesh : this->_meshes) {
				if (mesh == nullptr || mesh->empty()) continue; // Bone, skip it.

				// Bind pipelines ----
				this->_material->bindPipeline(*mesh);
				// -------------------

				// Update uniforms -----
				// bool buffersUpdated = false;
				// buffersUpdated = this->_material->bindVertexUniforms(*mesh);
				// buffersUpdated = this->_material->bindVertexSkinnedUniforms(*mesh);
				// buffersUpdated = this->_material->bindPixelUniforms(*mesh);

				this->_material->bindVertexUniforms(*mesh);
				this->_material->bindVertexSkinnedUniforms(*mesh);
				this->_material->bindPixelUniforms(*mesh);

				rawrbox::MAIN_CAMERA->setModelTransform(this->getMatrix() * mesh->getMatrix());
				// -----------

				// DRAW -------
				Diligent::DrawIndexedAttribs DrawAttrs;
				DrawAttrs.IndexType = Diligent::VT_UINT32;
				DrawAttrs.FirstIndexLocation = mesh->baseIndex;
				DrawAttrs.BaseVertex = mesh->baseVertex;
				DrawAttrs.NumIndices = mesh->totalIndex;
				DrawAttrs.Flags = Diligent::DRAW_FLAG_VERIFY_ALL;
				// if (!buffersUpdated) DrawAttrs.Flags |= Diligent::DRAW_FLAG_DYNAMIC_RESOURCE_BUFFERS_INTACT;

				rawrbox::RENDERER->context()->DrawIndexed(DrawAttrs);
				// -----------
			}
		}
	};
} // namespace rawrbox
