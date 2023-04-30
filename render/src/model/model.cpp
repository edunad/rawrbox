#include <rawrbox/render/model/light/manager.hpp>
#include <rawrbox/render/model/model.hpp>
#include <rawrbox/render/static.hpp>
#include <rawrbox/render/util/assimp/utils.hpp>
#include <rawrbox/utils/math.hpp>
#include <rawrbox/utils/time.hpp>

#include <assimp/matrix3x3.h>
#include <assimp/matrix4x4.h>
#include <assimp/quaternion.h>
#include <bx/math.h>
#include <fmt/format.h>

#include <algorithm>
#include <array>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <xstring>

#define BGFX_STATE_DEFAULT_3D (0 | BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_WRITE_Z | BGFX_STATE_DEPTH_TEST_LESS | BGFX_STATE_BLEND_ALPHA_TO_COVERAGE)

namespace rawrBox {

	void Model::preDraw() {
		for (auto& anim : this->_playingAnimations) {
			float timeToAdd = rawrBox::TimeUtils::deltaTime * anim.speed;
			float time = anim.time + timeToAdd;
			float totalDur = anim.data->duration;

			if (!((time > totalDur || time < -totalDur) && !anim.loop)) {
				anim.time += timeToAdd;
			}
		}
	}

	void Model::postDraw() {
		for (auto it2 = this->_playingAnimations.begin(); it2 != this->_playingAnimations.end();) {
			if ((*it2).time >= (*it2).data->duration && !(*it2).loop) {
				it2 = this->_playingAnimations.erase(it2);
			}

			++it2;
		}
	}

	void Model::updateBones(std::shared_ptr<rawrBox::Mesh> mesh) {
		this->_boneCalcs.clear();

		if (mesh->skeleton != nullptr) {
			// recursively calculate bone offsets
			this->readAnim(mesh->skeleton, mesh->skeleton->rootBone, {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1});
		}

		std::vector<std::array<float, 16>> transforms = {};
		transforms.resize(rawrBox::MAX_BONES_PER_MODEL);
		for (size_t i = 0; i < this->_boneCalcs.size(); i++) {
			transforms[i] = this->_boneCalcs[static_cast<uint8_t>(i)];
		}

		bgfx::setUniform(this->_material->getUniform("u_bones"), &transforms.front(), static_cast<uint32_t>(transforms.size()));
	}

	void Model::readAnim(std::shared_ptr<Skeleton> skeleton, std::shared_ptr<Bone> parentBone, const std::array<float, 16>& parentTransform) {
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

				// lerp the 3 components
				aiVector3D position = AssimpUtils::lerpPosition(timeInTicks, animChannel->position[positionFrameIndex], positionFrameIndex + 1 >= animChannel->position.size() ? animChannel->position.front() : animChannel->position[positionFrameIndex + 1]);
				aiQuaternion rotation = AssimpUtils::lerpRotation(timeInTicks, animChannel->rotation[rotationFrameIndex], rotationFrameIndex + 1 >= animChannel->rotation.size() ? animChannel->rotation.front() : animChannel->rotation[rotationFrameIndex + 1]);
				aiVector3D scale = AssimpUtils::lerpScale(timeInTicks, animChannel->scale[scaleFrameIndex], scaleFrameIndex + 1 >= animChannel->scale.size() ? animChannel->scale.front() : animChannel->scale[scaleFrameIndex + 1]);

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
			this->_boneCalcs[fnd->second->boneId] = MathUtils::mtxMul(MathUtils::mtxMul(skeleton->invTransformationMtx, globalTransformation), fnd->second->offsetMtx);
		}

		for (auto child : parentBone->children) {
			this->readAnim(skeleton, child, globalTransformation);
		}
	}

	// Animations ----
	bool Model::playAnimation(const std::string& name, bool loop, float speed) {
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

	bool Model::stopAnimation(const std::string& name) {
		for (size_t i = 0; i < this->_playingAnimations.size(); i++) {
			if (this->_playingAnimations[i].name != name) continue;
			this->_playingAnimations.erase(this->_playingAnimations.begin() + i);
			return true;
		}

		return false;
	}
	// -----

	void Model::draw(const rawrBox::Vector3f& camPos) {
		ModelBase::draw(camPos);

		this->preDraw();
		for (auto& mesh : this->_meshes) {
			this->_material->process(mesh);
			if (this->_material->hasUniform("u_bones")) this->updateBones(mesh);

			bgfx::setVertexBuffer(0, this->_vbh, mesh->baseVertex, mesh->totalVertex);
			bgfx::setIndexBuffer(this->_ibh, mesh->baseIndex, mesh->totalIndex);

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
} // namespace rawrBox
