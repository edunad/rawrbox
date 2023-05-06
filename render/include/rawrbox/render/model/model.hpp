#pragma once
#include <rawrbox/math/quaternion.hpp>
#include <rawrbox/math/utils/math.hpp>
#include <rawrbox/render/model/base.hpp>
#include <rawrbox/render/model/material/base.hpp>
#include <rawrbox/render/util/anim_utils.hpp>
#include <rawrbox/utils/time.hpp>

#include <assimp/anim.h>
#include <assimp/vector3.h>

#include <unordered_map>
#include <utility>
#include <vector>

#define BGFX_STATE_DEFAULT_3D (0 | BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_WRITE_Z | BGFX_STATE_DEPTH_TEST_LESS | BGFX_STATE_BLEND_ALPHA_TO_COVERAGE)

namespace rawrbox {

	template <typename T>
	struct AnimKey {
		float time;
		T value;

		std::pair<float, T> toPair() { return std::make_pair(time, value); }
	};

	enum AnimBehaviour {
		CONSTANT = 0,
		LERP = 1
	};

	struct AnimationFrame {
		std::string nodeName;

		std::vector<AnimKey<rawrbox::Vector3f>> position;
		std::vector<AnimKey<rawrbox::Vector3f>> scale;
		std::vector<AnimKey<rawrbox::Quaternion>> rotation;

		AnimBehaviour stateStart;
		AnimBehaviour stateEnd;
	};

	struct Animation {
		float ticksPerSecond = 0;
		float duration = 0;

		std::vector<AnimationFrame> frames;
	};

	struct PlayingAnimationData {
		std::string name;

		bool loop = false;
		float speed = 1;
		float time = 0;

		Animation* data = nullptr;
		PlayingAnimationData() = default;
		PlayingAnimationData(std::string _name, bool _loop, float _speed, float _time, Animation* _data) : name(std::move(_name)), loop(_loop), speed(_speed), time(_time), data(_data){};
	};

	template <typename M = rawrbox::MaterialBase>
	class Model : public rawrbox::ModelBase<M> {
	protected:
		std::unordered_map<std::string, Animation> _animations = {};
		std::vector<rawrbox::PlayingAnimationData> _playingAnimations = {};

		// ANIMATIONS ----
		virtual void updateBones(std::shared_ptr<rawrbox::Mesh<typename M::vertexBufferType>> mesh) {
			std::vector<std::array<float, 16>> transforms = {};
			transforms.resize(rawrbox::MAX_BONES_PER_MODEL);

			if (mesh->skeleton != nullptr) {
				auto calcs = std::unordered_map<uint8_t, std::array<float, 16>>();
				this->readAnim(calcs, mesh->skeleton, mesh->skeleton->rootBone, {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1});

				for (size_t i = 0; i < calcs.size(); i++) {
					transforms[i] = calcs[static_cast<uint8_t>(i)];
				}
			}

			if constexpr (supportsBones<typename M::vertexBufferType>) {
				bgfx::setUniform(this->_material->u_bones, &transforms.front(), static_cast<uint32_t>(transforms.size()));
			}
		}

		virtual void readAnim(std::unordered_map<uint8_t, std::array<float, 16>>& calcs, std::shared_ptr<Skeleton> skeleton, std::shared_ptr<Bone> parentBone, const std::array<float, 16>& parentTransform) {
			if (skeleton == nullptr) return;

			// update the final result inside the bones
			auto nodeTransform = parentBone->transformationMtx; // Default T-POSE

			// ANIMATE
			for (auto& anim : this->_playingAnimations) {
				auto animChannel = std::find_if(anim.data->frames.begin(), anim.data->frames.end(), [&](AnimationFrame& x) {
					return x.nodeName == parentBone->name;
				});

				if (animChannel != anim.data->frames.end()) {

					// figure out how "fast" the animation needs to play and the current playtime of the animation
					float ticksPerSecond = anim.data->ticksPerSecond != 0 ? anim.data->ticksPerSecond : 25.0F;
					float timeInTicks = anim.time * ticksPerSecond;
					timeInTicks = std::fmod(timeInTicks, anim.data->duration);

					// helper, select the next "frame" we should play depending on the time
					auto findFrameIndex = [&](auto& keys) {
						for (size_t i = 0; i + 1 < keys.size(); i++) {
							if (timeInTicks < keys[i + 1].time) {
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

					// lerp the 3 components.
					Vector3f position = nextPos.value;
					Quaternion rotation = nextRot.value;
					Vector3f scale = nextScl.value;

					if (animChannel->stateEnd == AnimBehaviour::LERP) {
						position = AnimUtils::lerpPosition(timeInTicks, currPos.toPair(), nextPos.toPair());
						rotation = AnimUtils::lerpRotation(timeInTicks, currRot.toPair(), nextRot.toPair());
						scale = AnimUtils::lerpScale(timeInTicks, currScl.toPair(), nextScl.toPair());
					}
					// ----

					bx::mtxIdentity(nodeTransform.data());

					auto rot = MathUtils::mtxQuaternion(rotation.w, rotation.x, rotation.y, rotation.z);
					MathUtils::mtxTranslate(nodeTransform, {position.x, position.y, position.z});
					MathUtils::mtxScale(nodeTransform, {scale.x, scale.y, scale.z});
					nodeTransform = MathUtils::mtxMul(nodeTransform, rot);
				}
			}

			// store the result of our parent bone and our current node
			auto globalTransformation = MathUtils::mtxMul(parentTransform, nodeTransform);

			auto fnd = this->_globalBoneMap.find(parentBone->name);
			if (fnd != this->_globalBoneMap.end()) {
				calcs[fnd->second->boneId] = MathUtils::mtxMul(MathUtils::mtxMul(skeleton->invTransformationMtx, globalTransformation), fnd->second->offsetMtx);
			}

			for (auto child : parentBone->children) {
				this->readAnim(calcs, skeleton, child, globalTransformation);
			}

			return;
		}

		virtual void preDraw() {
			for (auto& anim : this->_playingAnimations) {
				float timeToAdd = rawrbox::TimeUtils::deltaTime * anim.speed;
				float time = anim.time + timeToAdd;
				float totalDur = anim.data->duration;

				if (!((time > totalDur || time < -totalDur) && !anim.loop)) {
					anim.time += timeToAdd;
				}
			}
		}

		virtual void postDraw() {
			for (auto it2 = this->_playingAnimations.begin(); it2 != this->_playingAnimations.end();) {
				if ((*it2).time >= (*it2).data->duration && !(*it2).loop) {
					it2 = this->_playingAnimations.erase(it2);
				}

				++it2;
			}
		}
		// --------------

	public:
		using ModelBase<M>::ModelBase;

		// Animations ----
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
		// -----

		void draw(const rawrbox::Vector3f& camPos) override {
			ModelBase<M>::draw(camPos);

			this->preDraw();
			for (auto mesh : this->_meshes) {
				this->_material->process(mesh);

				// Process bones ---
				if constexpr (supportsBones<typename M::vertexBufferType>) {
					this->updateBones(mesh);
				}
				// ---

				if (this->isDynamicBuffer()) {
					bgfx::setVertexBuffer(0, this->_vbdh, mesh->baseVertex, mesh->totalVertex);
					bgfx::setIndexBuffer(this->_ibdh, mesh->baseIndex, mesh->totalIndex);
				} else {
					bgfx::setVertexBuffer(0, this->_vbh, mesh->baseVertex, mesh->totalVertex);
					bgfx::setIndexBuffer(this->_ibh, mesh->baseIndex, mesh->totalIndex);
				}

				std::array<float, 16> matrix = {};
				bx::mtxMul(matrix.data(), mesh->offsetMatrix.data(), this->_matrix.data());
				bgfx::setTransform(matrix.data());

				uint64_t flags = BGFX_STATE_DEFAULT_3D | mesh->culling | mesh->blending;
				if (mesh->wireframe) flags |= BGFX_STATE_PT_LINES;

				bgfx::setState(flags, 0);
				this->_material->postProcess();
			}
			this->postDraw();
		}
	};
} // namespace rawrbox
