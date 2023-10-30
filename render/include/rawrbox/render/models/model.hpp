#pragma once

#include <rawrbox/engine/static.hpp>
#include <rawrbox/render/light/manager.hpp>
#include <rawrbox/render/models/animation.hpp>
#include <rawrbox/render/models/base.hpp>
#include <rawrbox/render/models/skeleton.hpp>
#include <rawrbox/render/static.hpp>
#include <rawrbox/render/utils/anim.hpp>

#ifdef RAWRBOX_SCRIPTING
	#include <rawrbox/render/scripting/wrappers/model/model_wrapper.hpp>
	#include <sol/sol.hpp>
#endif

namespace rawrbox {

	template <typename M = rawrbox::MaterialUnlit>
	class Model : public rawrbox::ModelBase<M> {

	protected:
		std::unordered_map<std::string, rawrbox::Animation> _animations = {};

		std::vector<rawrbox::PlayingAnimationData> _playingAnimations = {};
		std::vector<rawrbox::LightBase> _lights = {};

		std::vector<std::unique_ptr<rawrbox::Mesh<typename M::vertexBufferType>>> _meshes = {};
		rawrbox::BBOX _bbox = {};

		// SKINNING ----
		std::unordered_map<std::string, rawrbox::Mesh<typename M::vertexBufferType>*> _animatedMeshes = {}; // Map for quick lookup
		// --------

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

			// store the result of our parent bone and our current node
			rawrbox::Matrix4x4 globalTransformation = parentTransform * nodeTransform;
			auto fnd = skeleton.boneMap.find(parentBone.name);
			if (fnd != skeleton.boneMap.end()) {
				calcs[fnd->second->boneId] = skeleton.invTransformationMtx * globalTransformation * fnd->second->offsetMtx;
			}

			for (auto& child : parentBone.children) {
				this->animateBones(calcs, skeleton, *child, globalTransformation);
			}
		}

		void readAnims(rawrbox::Matrix4x4& nodeTransform, const std::string& nodeName) const {
			for (auto& anim : this->_playingAnimations) {
				auto animChannel = std::find_if(anim.data->frames.begin(), anim.data->frames.end(), [&](AnimationFrame& x) {
					return x.nodeName == nodeName;
				});

				if (animChannel != anim.data->frames.end()) {
					// figure out how "fast" the animation needs to play and the current playtime of the animation
					float ticksPerSecond = anim.data->ticksPerSecond != 0 ? anim.data->ticksPerSecond : 25.0F;
					float timeInTicks = anim.time * ticksPerSecond;
					timeInTicks = std::fmod(timeInTicks, anim.data->duration);

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
					Vector3f position = nextPos.second;
					Vector4f rotation = nextRot.second;
					Vector3f scale = nextScl.second;

					float t = rawrbox::EasingUtils::ease(animChannel->stateEnd, timeInTicks);

					position = rawrbox::AnimUtils::lerpVector3(t, currPos, nextPos);
					rotation = rawrbox::AnimUtils::lerpRotation(t, currRot, nextRot);
					scale = rawrbox::AnimUtils::lerpVector3(t, currScl, nextScl);
					//   ----

					rawrbox::Matrix4x4 mt = {};
					mt.translate(position);

					rawrbox::Matrix4x4 ms = {};
					ms.scale(scale);

					rawrbox::Matrix4x4 mr = {};
					mr.rotate(rotation);

					nodeTransform = mt * mr * ms;
				}
			}
		}

		void preDraw() {
			for (auto& anim : this->_playingAnimations) {
				float timeToAdd = rawrbox::DELTA_TIME * anim.speed;
				float time = anim.time + timeToAdd;
				float totalDur = anim.data->duration;

				if (!((time > totalDur || time < -totalDur) && !anim.loop)) {
					anim.time += timeToAdd;
				}
			}
		}
		void postDraw() {
			for (auto it2 = this->_playingAnimations.begin(); it2 != this->_playingAnimations.end();) {
				if ((*it2).time >= (*it2).data->duration && !(*it2).loop) {
					it2 = this->_playingAnimations.erase(it2);
					continue;
				}

				++it2;
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

#ifdef RAWRBOX_SCRIPTING
		void initializeLua() override {
			if (this->_luaWrapper.valid()) this->_luaWrapper.abandon();
			this->_luaWrapper = sol::make_object(rawrbox::SCRIPTING::getLUA(), rawrbox::ModelWrapper(this->shared_from_this()));
		}
#endif

	public:
		Model() = default;
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
					return !a->alphaBlend && b->alphaBlend;
				});
			}
			// --------
		}

		virtual void setOptimizable(bool status) { this->_canOptimize = status; }
		virtual void optimize() {
#ifndef NDEBUG
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

#ifndef NDEBUG
			if (old != this->_meshes.size()) fmt::print("[RawrBox-Model] Optimized mesh for rendering (Before {} | After {})\n", old, this->_meshes.size());
#endif
		}

		void updateBuffers() override {
			this->flattenMeshes();
			rawrbox::ModelBase<M>::updateBuffers();
		}

		// ANIMATIONS ----
		virtual bool blendAnimation(const std::string& /*otherAnim*/, float /*blend*/) {
			throw std::runtime_error("TODO");
		}

		virtual bool playAnimation(const std::string& name, bool loop = true, float speed = 1.F) {
			auto iter = this->_animations.find(name);
			if (iter == this->_animations.end()) throw std::runtime_error(fmt::format("[RawrBox-Model] Animation {} not found!", name));

			// Add it
			this->_playingAnimations.emplace_back(name,
			    loop,
			    speed,
			    0.0F,
			    &iter->second);

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
		// --------------

		// BLEND SHAPES ---
		void createBlendShape(size_t mesh, const std::string& id, const std::vector<rawrbox::Vector3f>& newVertexPos, const std::vector<rawrbox::Vector3f>& newNormPos, float weight = 0.F) {
			if (mesh >= this->_meshes.size()) throw std::runtime_error(fmt::format("[RawrBox-ModelBase] Mesh '{}' not found!", mesh));

			auto blend = std::make_unique<rawrbox::BlendShapes<typename M::vertexBufferType>>();
			blend->pos = newVertexPos;
			blend->normals = newNormPos;
			blend->weight = weight;
			blend->mesh = this->_meshes[mesh].get();

			this->_blend_shapes[id] = std::move(blend);
		}
		// --------------

		// LIGHTS ------
		template <typename T = rawrbox::LightBase, typename... CallbackArgs>
		void addLight(const std::string& parentMesh = "", CallbackArgs&&... args) {
			if constexpr (supportsNormals<typename M::vertexBufferType>) {
				auto parent = this->_meshes.back().get();
				if (!parentMesh.empty()) {
					auto fnd = std::find_if(this->_meshes.begin(), this->_meshes.end(), [parentMesh](auto& msh) {
						return msh->getName() == parentMesh;
					});

					if (fnd != this->_meshes.end()) parent = fnd->get();
				}

				auto light = rawrbox::LIGHTS::addLight<T>(std::forward<CallbackArgs>(args)...);
				light->setOffsetPos(parent->getPos() + this->getPos());
				parent->lights.push_back(light);
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
			for (auto it2 = this->_meshes.begin(); it2 != this->_meshes.end();) {
				if ((*it2)->getName() == id) {
					it2 = this->_meshes.erase(it2);
					continue;
				}

				++it2;
			}

			if (this->isUploaded() && this->isDynamic()) this->updateBuffers(); // Already uploaded? And dynamic? Then update vertices
		}
		virtual void removeMesh(size_t index) {
			if (index >= this->_meshes.size()) return;
			this->_meshes.erase(this->_meshes.begin() + index);

			if (this->isUploaded() && this->isDynamic()) this->updateBuffers(); // Already uploaded? And dynamic? Then update vertices
		}

		virtual rawrbox::Mesh<typename M::vertexBufferType>* addMesh(rawrbox::Mesh<typename M::vertexBufferType> mesh) {
			this->_bbox.combine(mesh.getBBOX());
			mesh.owner = this;

			auto& a = this->_meshes.emplace_back(std::make_unique<rawrbox::Mesh<typename M::vertexBufferType>>(mesh));
			if (this->isUploaded() && this->isDynamic()) this->updateBuffers(); // Already uploaded? And dynamic? Then update vertices

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

		virtual bool hasMesh(size_t id) {
			return id >= 0 && id < this->_meshes.size();
		}

		virtual void setCulling(Diligent::CULL_MODE cull, int id = -1) {
			for (size_t i = 0; i < this->_meshes.size(); i++) {
				if (id != -1 && i != static_cast<size_t>(id)) continue;
				this->_meshes[i]->setCulling(cull);
			}
		}

		virtual void setWireframe(bool wireframe, int id = -1) {
			for (size_t i = 0; i < this->_meshes.size(); i++) {
				if (id != -1 && i != static_cast<size_t>(id)) continue;
				this->_meshes[i]->setWireframe(wireframe);
			}
		}

		virtual void setColor(const rawrbox::Color& color, int id = -1) {
			for (size_t i = 0; i < this->_meshes.size(); i++) {
				if (id != -1 && i != static_cast<size_t>(id)) continue;
				this->_meshes[i]->setColor(color);
			}
		}

		virtual void setRecieveDecals(bool recieve, int id = -1) {
			for (size_t i = 0; i < this->_meshes.size(); i++) {
				if (id != -1 && i != static_cast<size_t>(id)) continue;
				this->_meshes[i]->setRecieveDecals(recieve);
			}
		}

		virtual void setTexture(rawrbox::TextureBase* tex, int id = -1) {
			for (size_t i = 0; i < this->_meshes.size(); i++) {
				if (id != -1 && i != static_cast<size_t>(id)) continue;
				this->_meshes[i]->setTexture(tex);
			}
		}

		virtual std::vector<std::unique_ptr<rawrbox::Mesh<typename M::vertexBufferType>>>& meshes() {
			return this->_meshes;
		}

		void upload(bool dynamic = false) override {
			this->flattenMeshes(); // Merge and optimize meshes for drawing
			ModelBase<M>::upload(dynamic);
		}

		void draw() override {
			ModelBase<M>::draw();
			this->preDraw();

			auto context = rawrbox::RENDERER->context();
			for (auto& mesh : this->_meshes) {
				// Process animations ---
				this->animate(*mesh);
				// ---

				// Bind materials uniforms & textures ----
				rawrbox::TRANSFORM = this->getMatrix() * mesh->getMatrix();
				this->_material->bindTexture(*mesh);
				this->_material->bindPipeline(*mesh);
				this->_material->bindUniforms(*mesh);
				this->_material->bindShaderResources();
				//  -----------

				Diligent::DrawIndexedAttribs DrawAttrs;    // This is an indexed draw call
				DrawAttrs.IndexType = Diligent::VT_UINT16; // Index type
				DrawAttrs.FirstIndexLocation = mesh->baseIndex;
				DrawAttrs.BaseVertex = mesh->baseVertex;
				DrawAttrs.NumIndices = mesh->totalIndex;
				DrawAttrs.Flags = Diligent::DRAW_FLAG_VERIFY_ALL; // Verify the state of vertex and index buffers
				context->DrawIndexed(DrawAttrs);
			}

			this->postDraw();
		}
	};
} // namespace rawrbox
