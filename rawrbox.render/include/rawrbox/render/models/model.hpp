#pragma once

#include <rawrbox/engine/static.hpp>
#include <rawrbox/render/lights/manager.hpp>
#include <rawrbox/render/models/animation.hpp>
#include <rawrbox/render/models/base.hpp>
#include <rawrbox/render/models/skeleton.hpp>
#include <rawrbox/render/static.hpp>
#include <rawrbox/render/utils/anim.hpp>

namespace rawrbox {

	template <typename M = rawrbox::MaterialUnlit>
		requires(std::derived_from<M, rawrbox::MaterialBase>)
	class Model : public rawrbox::ModelBase<M> {

	protected:
		// ANIMATION ---
		std::unordered_map<std::string, rawrbox::Animation> _animations = {};
		std::vector<rawrbox::PlayingAnimationData> _playingAnimations = {};
		// ------------

		std::vector<std::unique_ptr<rawrbox::Mesh<typename M::vertexBufferType>>> _meshes = {};
		rawrbox::BBOX _bbox = {};

		// SKINNING ----
		std::unordered_map<std::string, rawrbox::Mesh<typename M::vertexBufferType>*> _animatedMeshes = {}; // Map for quick lookup
		// --------

		// LIGHTS ---
		std::vector<rawrbox::LightBase> _lights = {};
		// -----

		bool _canOptimize = true;

		// ANIMATIONS ----
		void animate(rawrbox::Mesh<typename M::vertexBufferType>& mesh) const {
			// VERTEX ANIMATION ----
			for (auto& anim : this->_animatedMeshes) {
				if (anim.second == nullptr) continue;
				this->readAnims(anim.second->matrix, anim.first);
			}
			// ------------

			// BONE ANIMATION ----
			if constexpr (supportsBones<typename M::vertexBufferType>) {
				std::array<rawrbox::Matrix4x4, rawrbox::MAX_BONES_PER_MODEL> boneTransforms = {};

				if (mesh.skeleton != nullptr) {
					auto calcs = std::unordered_map<uint8_t, rawrbox::Matrix4x4>();
					this->animateBones(calcs, *mesh.skeleton, *mesh.skeleton->rootBone, {});

					for (size_t i = 0; i < calcs.size(); i++) {
						boneTransforms[i] = calcs[static_cast<uint8_t>(i)];
					}
				}

				mesh.boneTransforms = boneTransforms;
			}
			// -----
		}

		void animateBones(std::unordered_map<uint8_t, rawrbox::Matrix4x4>& calcs, const rawrbox::Skeleton& skeleton, const rawrbox::Bone& parentBone, const rawrbox::Matrix4x4& parentTransform) const {
			auto nodeTransform = parentBone.transformationMtx * parentBone.overrideMtx;
			this->readAnims(nodeTransform, parentBone.name);

			rawrbox::Matrix4x4 globalTransformation = parentTransform * nodeTransform;
			auto fnd = skeleton.boneMap.find(parentBone.name);
			if (fnd != skeleton.boneMap.end()) {
				calcs[fnd->second->boneId] = skeleton.invTransformationMtx * globalTransformation * fnd->second->offsetMtx;
			}

			for (const auto& child : parentBone.children) {
				this->animateBones(calcs, skeleton, *child, globalTransformation);
			}
		}

		void readAnims(rawrbox::Matrix4x4& nodeTransform, const std::string& nodeName) const {
			for (auto& anim : this->_playingAnimations) {
				auto animChannel = std::find_if(anim.data->frames.begin(), anim.data->frames.end(), [&](AnimationFrame& x) {
					return x.nodeName == nodeName;
				});

				if (animChannel != anim.data->frames.end()) {
					float timeInTicks = std::fmod(anim.time, anim.data->duration);

					// helper, select the next "frame" we should play depending on the time
					auto findFrameIndex = [&](auto& keys) {
						for (size_t i = 0; i + 1 < keys.size(); i++) {
							if (timeInTicks < keys[i + 1].first) {
								return i;
							}
						}

						return static_cast<size_t>(0);
					};

					// find all frames
					auto positionFrameIndex = findFrameIndex(animChannel->position);
					auto rotationFrameIndex = findFrameIndex(animChannel->rotation);
					auto scaleFrameIndex = findFrameIndex(animChannel->scale);

					auto currPos = animChannel->position[positionFrameIndex];
					auto nextPos = positionFrameIndex + 1 >= animChannel->position.size() ? animChannel->position.front() : animChannel->position[positionFrameIndex + 1];

					auto currRot = animChannel->rotation[rotationFrameIndex];
					auto nextRot = rotationFrameIndex + 1 >= animChannel->rotation.size() ? animChannel->rotation.front() : animChannel->rotation[rotationFrameIndex + 1];

					auto currScl = animChannel->scale[scaleFrameIndex];
					auto nextScl = scaleFrameIndex + 1 >= animChannel->scale.size() ? animChannel->scale.front() : animChannel->scale[scaleFrameIndex + 1];

					// Easing ----
					rawrbox::Vector3f position = nextPos.second;
					rawrbox::Vector4f rotation = nextRot.second;
					rawrbox::Vector3f scale = nextScl.second;

					float t = rawrbox::EasingUtils::ease(animChannel->stateEnd, timeInTicks);

					position = rawrbox::AnimUtils::lerpVector3(t, currPos, nextPos);
					rotation = rawrbox::AnimUtils::lerpRotation(t, currRot, nextRot);
					scale = rawrbox::AnimUtils::lerpVector3(t, currScl, nextScl);
					//   ----

					nodeTransform = rawrbox::Matrix4x4::mtxSRT(scale, rotation, position);
				}
			}
		}

		void updateAnimations() {
			for (auto it = this->_playingAnimations.begin(); it != this->_playingAnimations.end();) {
				float timeToAdd = rawrbox::DELTA_TIME * it->speed * it->data->ticksPerSecond;
				float newTime = it->time + timeToAdd;
				float totalDur = it->data->duration;

				// Check if the animation has reached the end or the beginning (for negative speed)
				bool animationEnded = (it->speed >= 0 && newTime >= totalDur) || (it->speed < 0 && newTime <= 0);

				if (animationEnded) {
					if (it->loop) {
						// If looping, wrap the time around
						newTime = std::fmod(newTime, totalDur);
						if (newTime < 0) newTime += totalDur;
					} else {
						// If not looping, clamp the time to the duration or 0
						newTime = it->speed >= 0 ? totalDur : 0;
					}

					this->onAnimationComplete(it->name);

					if (!it->loop) {
						it = this->_playingAnimations.erase(it);
						continue;
					}
				}

				it->time = newTime;
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
		// ANIMATION ---
		rawrbox::Event<std::string> onAnimationComplete = {};
		// ------------

		Model(size_t vertices = 0, size_t indices = 0) : rawrbox::ModelBase<M>(vertices, indices){};
		Model(const Model&) = delete;
		Model(Model&&) = delete;
		Model& operator=(const Model&) = delete;
		Model& operator=(Model&&) = delete;
		~Model() override {
			this->_meshes.clear();
			this->_animatedMeshes.clear();
			this->_animations.clear();
			this->_lights.clear();
		}

		virtual void flattenMeshes(bool optimize = true, bool sort = true) {
			this->_mesh->clear();

			// Merge same meshes to reduce calls
			if (this->_canOptimize && optimize) {
				this->optimize();
			}
			// ----------------------

			// Flatten meshes for buffers
			for (auto& mesh : this->_meshes) {
				// Fix start index ----
				mesh->baseIndex = static_cast<uint16_t>(this->_mesh->indices.size());
				mesh->baseVertex = static_cast<uint16_t>(this->_mesh->vertices.size());
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

		virtual void setOptimizable(bool status) { this->_canOptimize = status; }
		virtual void optimize() {
#ifdef _DEBUG
			size_t old = this->_meshes.size();
#endif

			for (size_t i1 = 0; i1 < this->_meshes.size(); i1++) {
				auto& mesh1 = this->_meshes[i1];

				// figure out how big our buffers will get
				size_t reserveVertices = mesh1->vertices.size();
				size_t reserveIndices = mesh1->indices.size();

				for (size_t i2 = this->_meshes.size() - 1; i2 > i1; i2--) {
					auto& mesh2 = this->_meshes[i2];
					if (!mesh1->canOptimize(*mesh2)) continue;

					reserveVertices += mesh2->vertices.size();
					reserveIndices += mesh2->indices.size();
				}

				if (reserveVertices == mesh1->vertices.size()) continue;
				mesh1->vertices.reserve(reserveVertices);
				mesh1->indices.reserve(reserveIndices);

				// merge what it can
				for (size_t i2 = this->_meshes.size() - 1; i2 > i1; i2--) {
					auto& mesh2 = this->_meshes[i2];
					if (!mesh1->canOptimize(*mesh2)) continue;

					mesh1->merge(*mesh2);
					this->_meshes.erase(this->_meshes.begin() + i2);
				}
			}

#ifdef _DEBUG
			if (old != this->_meshes.size() && !this->isUploaded()) this->_logger->info("Optimized mesh for rendering (Before {} | After {}), this will only display once to prevent spam.", old, this->_meshes.size()); // Only do it once
#endif
		}

		void updateBuffers() override {
			if (!this->isUploaded()) throw this->_logger->error("Model is not uploaded!");
			if (!this->isDynamic()) throw this->_logger->error("Model is not dynamic!");

			this->flattenMeshes();
			rawrbox::ModelBase<M>::updateBuffers();
		}

		// ANIMATIONS ----
		virtual bool blendAnimation(const std::string& /*otherAnim*/, float /*blend*/) {
			throw this->_logger->error("TODO");
		}

		virtual bool playAnimation(const std::string& name, bool loop = true, float speed = 1.F) {
			auto iter = this->_animations.find(name);
			if (iter == this->_animations.end()) {
				throw this->_logger->error("Animation '{}' not found", fmt::format(fmt::fg(fmt::color::coral), name));
			}

			// Add it
			this->_playingAnimations.emplace_back(name,
			    loop,
			    speed,
			    0.0F,
			    iter->second);

			return true;
		}

		virtual bool stopAnimation(const std::string& name) {
			for (size_t i = 0; i < this->_playingAnimations.size(); i++) {
				if (this->_playingAnimations[i].name != name) continue;
				this->_playingAnimations.erase(this->_playingAnimations.begin() + i);
				return true;
			}

			return false;
		}

		virtual bool hasAnimation(const std::string& name) {
			return this->_animations.find(name) != this->_animations.end();
		}

		virtual bool isAnimationPlaying(const std::string& name) {
			return std::find_if(this->_playingAnimations.begin(), this->_playingAnimations.end(), [&name](const rawrbox::PlayingAnimationData& anim) { return anim.name == name; }) != this->_playingAnimations.end();
		}
		// --------------

		// LIGHTS ------
		template <typename T = rawrbox::LightBase, typename... CallbackArgs>
			requires(std::derived_from<T, rawrbox::LightBase>)
		T* addLight(const std::string& parentMesh = "", CallbackArgs&&... args) {
			if constexpr (supportsNormals<typename M::vertexBufferType>) {
				auto parent = this->_meshes.back().get();
				if (!parentMesh.empty()) {
					auto fnd = std::find_if(this->_meshes.begin(), this->_meshes.end(), [parentMesh](auto& msh) {
						return msh->getName() == parentMesh;
					});

					if (fnd != this->_meshes.end()) parent = fnd->get();
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
			return index >= 0 && index < this->_meshes.size();
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
			this->updateAnimations();

			for (auto& mesh : this->_meshes) {
				// Process animations ---
				this->animate(*mesh);
				// ---

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
				DrawAttrs.IndexType = Diligent::VT_UINT16;
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
