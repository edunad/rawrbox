#pragma once
#include <rawrbox/engine/static.hpp>
#include <rawrbox/math/matrix4x4.hpp>
#include <rawrbox/math/utils/math.hpp>
#include <rawrbox/math/vector4.hpp>
#include <rawrbox/render/model/animation.hpp>
#include <rawrbox/render/model/base.hpp>
#include <rawrbox/render/model/light/base.hpp>
#include <rawrbox/render/model/light/manager.hpp>
#include <rawrbox/render/model/material/base.hpp>
#include <rawrbox/render/utils/anim.hpp>

#include <assimp/anim.h>
#include <assimp/vector3.h>
#include <bx/easing.h>

#include <unordered_map>
#include <utility>
#include <vector>

#define BGFX_STATE_DEFAULT_3D (0 | BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A)

namespace rawrbox {

	template <typename M = rawrbox::MaterialBase>
	class Model : public rawrbox::ModelBase<M> {
	protected:
		std::unordered_map<std::string, Animation> _animations = {};
		std::vector<rawrbox::PlayingAnimationData> _playingAnimations = {};
		std::vector<rawrbox::LightBase> lights = {};

		// ANIMATIONS ----
		void animate(const rawrbox::Mesh<typename M::vertexBufferType>& mesh) const {
			// VERTEX ANIMATION ----
			for (auto& anim : this->_animatedMeshes) {
				if (anim.second == nullptr) continue;
				this->readAnims(anim.second->offsetMatrix, anim.first);
			}
			// ------------

			// BONE ANIMATION ----
			if constexpr (supportsBones<typename M::vertexBufferType>) {
				std::vector<rawrbox::Matrix4x4> boneTransforms = {};
				boneTransforms.resize(rawrbox::MAX_BONES_PER_MODEL);

				if (mesh.skeleton != nullptr) {
					auto calcs = std::unordered_map<uint8_t, rawrbox::Matrix4x4>();
					this->animateBones(calcs, *mesh.skeleton, *mesh.skeleton->rootBone, {});

					for (size_t i = 0; i < calcs.size(); i++) {
						boneTransforms[i] = calcs[static_cast<uint8_t>(i)];
					}
				}

				bgfx::setUniform(this->_material->u_bones, &boneTransforms.front(), static_cast<uint32_t>(boneTransforms.size()));
			}
			// -----
		}

		void animateBones(std::unordered_map<uint8_t, rawrbox::Matrix4x4>& calcs, const rawrbox::Skeleton& skeleton, const rawrbox::Bone& parentBone, const rawrbox::Matrix4x4& parentTransform) const {
			auto nodeTransform = parentBone.transformationMtx;
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

					bx::EaseFn ease = bx::getEaseFunc(animChannel->stateEnd);
					float t = ease(timeInTicks);

					position = AnimUtils::lerpVector3(t, currPos, nextPos);
					rotation = AnimUtils::lerpRotation(t, currRot, nextRot);
					scale = AnimUtils::lerpVector3(t, currScl, nextScl);
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
			for (auto& mesh : this->meshes()) {
				rawrbox::Vector3f meshPos = {mesh->offsetMatrix[12], mesh->offsetMatrix[13], mesh->offsetMatrix[14]};
				auto p = rawrbox::MathUtils::applyRotation(meshPos + this->getPos(), this->getAngle());

				for (auto light : mesh->lights) {
					if (light.expired()) continue;
					light.lock()->setOffsetPos(p);
				}
			}
		}

	public:
		using ModelBase<M>::ModelBase;

		// Animations ----
		bool blendAnimation(const std::string& otherAnim, float blend) {
			throw std::runtime_error("TODO");
		}

		bool playAnimation(const std::string& name, bool loop = true, float speed = 1.F) {
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
		// LIGHTS ------
		virtual void addLight(std::shared_ptr<rawrbox::LightBase> light, const std::string& parentMesh = "") {
			auto parent = this->_meshes.back().get();

			if (!parentMesh.empty()) {
				auto fnd = std::find_if(this->_meshes.begin(), this->_meshes.end(), [parentMesh](auto& msh) {
					return msh->getName() == parentMesh;
				});

				if (fnd != this->_meshes.end()) parent = fnd->get();
			}

			light->setOffsetPos(parent->getPos() + this->getPos());
			parent->lights.push_back(light);

			rawrbox::LIGHTS::addLight(light);
		}
		// -----

		void setPos(const rawrbox::Vector3f& pos) override {
			rawrbox::ModelBase<M>::setPos(pos);
			this->updateLights();
		}

		void setAngle(const rawrbox::Vector4f& angle) override {
			rawrbox::ModelBase<M>::setAngle(angle);
			// this->updateLights(); // TODO
		}

		void setEulerAngle(const rawrbox::Vector3f& angle) override {
			rawrbox::ModelBase<M>::setEulerAngle(angle);
			// this->updateLights(); // TODO
		}

		void setScale(const rawrbox::Vector3f& size) override {
			rawrbox::ModelBase<M>::setScale(size);
			// this->updateLights(); // TODO
		}

		void draw(const rawrbox::Vector3f& camPos) override {
			ModelBase<M>::draw(camPos);

			this->preDraw();
			for (auto& mesh : this->_meshes) {
				this->_material->process(*mesh);

				// Process animations ---
				this->animate(*mesh);
				// ---

				if (this->isDynamicBuffer()) {
					bgfx::setVertexBuffer(0, this->_vbdh, mesh->baseVertex, mesh->totalVertex);
					bgfx::setIndexBuffer(this->_ibdh, mesh->baseIndex, mesh->totalIndex);
				} else {
					bgfx::setVertexBuffer(0, this->_vbh, mesh->baseVertex, mesh->totalVertex);
					bgfx::setIndexBuffer(this->_ibh, mesh->baseIndex, mesh->totalIndex);
				}

				bgfx::setTransform((this->_matrix * mesh->offsetMatrix).data());

				uint64_t flags = BGFX_STATE_DEFAULT_3D | mesh->culling | mesh->blending | mesh->depthTest;
				flags |= mesh->lineMode ? BGFX_STATE_PT_LINES : mesh->wireframe ? BGFX_STATE_PT_LINESTRIP
												: 0;

				bgfx::setState(flags, 0);
				this->_material->postProcess();
			}
			this->postDraw();
		}
	};
} // namespace rawrbox
