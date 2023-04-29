#include <rawrbox/render/model/light/manager.h>
#include <rawrbox/render/model/model.h>
#include <rawrbox/render/static.h>
#include <rawrbox/render/util/assimp/utils.hpp>
#include <rawrbox/utils/time.h>

#include <assimp/matrix3x3.h>
#include <assimp/matrix4x4.h>
#include <assimp/quaternion.h>
#include <bx/math.h>
#include <fmt/format.h>

#include <algorithm>
#include <array>
#include <cstdint>
#include <glm/fwd.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>
#include <memory>
#include <stdexcept>

#include "rawrbox/math/quaternion.hpp"
#include "rawrbox/utils/math.hpp"

#define BGFX_STATE_DEFAULT_3D (0 | BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_WRITE_Z | BGFX_STATE_DEPTH_TEST_LESS | BGFX_STATE_BLEND_ALPHA_TO_COVERAGE)

namespace rawrBox {

	void Model::preDraw() {
		if (this->_currentAnimation == nullptr) return;

		float timeToAdd = rawrBox::TimeUtils::deltaTime * this->_currentAnimation->speed;
		float time = this->_currentAnimation->time + timeToAdd;
		float totalDur = this->_currentAnimation->data->duration;

		if (!((time > totalDur || time < -totalDur) && !this->_currentAnimation->loop)) {
			this->_currentAnimation->time += timeToAdd;
		}
	}

	void Model::postDraw() {
		if (this->_currentAnimation == nullptr) return;

		// remove anim playing if we're done
		if (this->_currentAnimation->time >= this->_currentAnimation->data->duration && !this->_currentAnimation->loop) {
			this->_currentAnimation = nullptr;
		}
	}

	void Model::updateBones(std::shared_ptr<rawrBox::Mesh> mesh) {
		if (mesh->skeleton == nullptr || this->_currentAnimation == nullptr) return;

		// recursively calculate bone offsets
		this->_boneCalcs.clear();
		this->readAnim(mesh->skeleton, mesh->skeleton->rootBone, glm::mat4(1.f));

		std::vector<std::array<float, 16>> transforms;
		for (auto pair : this->_boneCalcs) {
			float* pSource = glm::value_ptr(pair.second);

			std::array<float, 16> a = {};
			for (int i = 0; i < 16; ++i)
				a[i] = pSource[i];

			transforms.push_back(a);
		}

		bgfx::setUniform(this->_material->getUniform("u_bones"), &transforms.front(), static_cast<uint32_t>(transforms.size()));
	}

	void Model::readAnim(std::shared_ptr<Skeleton> skeleton, std::shared_ptr<Bone> parentBone, const glm::mat4x4& parentTransform) {
		if (skeleton == nullptr) return;

		// update the final result inside the bones
		auto nodeTransform = parentBone->transformationMtx;
		std::string boneKey = fmt::format("{}-{}", skeleton->name, parentBone->name);

		// ANIMATE
		auto animChannel = std::find_if(this->_currentAnimation->data->frames.begin(), this->_currentAnimation->data->frames.end(), [&](AnimationFrame& x) {
			return x.nodeName == parentBone->name;
		});

		if (animChannel != this->_currentAnimation->data->frames.end()) {

			// figure out how "fast" the animation needs to play and the current playtime of the animation
			float ticksPerSecond = this->_currentAnimation->data->ticksPerSecond != 0 ? this->_currentAnimation->data->ticksPerSecond : 25.0f;
			float timeInTicks = this->_currentAnimation->time * ticksPerSecond;
			timeInTicks = std::fmod(timeInTicks, this->_currentAnimation->data->duration);

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
			aiVector3D position = animChannel->position[positionFrameIndex].value;   // AssimpUtils::lerpPosition(timeInTicks, animChannel->position[positionFrameIndex], positionFrameIndex + 1 >= animChannel->position.size() ? animChannel->position.front() : animChannel->position[positionFrameIndex + 1]);
			aiQuaternion rotation = animChannel->rotation[rotationFrameIndex].value; // AssimpUtils::lerpRotation(timeInTicks, animChannel->rotation[rotationFrameIndex], rotationFrameIndex + 1 >= animChannel->rotation.size() ? animChannel->rotation.front() : animChannel->rotation[rotationFrameIndex + 1]);
			aiVector3D scale = animChannel->scale[scaleFrameIndex].value;            // AssimpUtils::lerpScale(timeInTicks, animChannel->scale[scaleFrameIndex], scaleFrameIndex + 1 >= animChannel->scale.size() ? animChannel->scale.front() : animChannel->scale[scaleFrameIndex + 1]);

			auto glmRotation = glm::toMat4(glm::quat(rotation.w, rotation.x, rotation.y, rotation.z));
			nodeTransform = glm::mat4x4(1.f);
			nodeTransform = glm::translate(nodeTransform, {position.x, position.y, position.z});
			nodeTransform = glm::scale(nodeTransform, {scale.x, scale.y, scale.z});
			nodeTransform *= glmRotation;
		}

		// store the result of our parent bone and our current node
		auto globalTransformation = parentTransform * nodeTransform;

		auto fnd = this->_boneMap.find(boneKey);
		if (fnd != this->_boneMap.end()) {
			auto& p = this->_boneCalcs[fnd->second.first];
			p = skeleton->invTransformationMtx * globalTransformation * fnd->second.second;

			// bx::mtxMul(p.data(), skeleton->invTransformationMtx.data(), globalTransformation.data());
			// bx::mtxMul(p.data(), p.data(), fnd->second.second.data());
		}

		for (auto child : parentBone->children) {
			this->readAnim(skeleton, child, globalTransformation);
		}

		/*std::array<float, 16> globalTransformation = MathUtils::mtxMul(parentTransform, nodeTransform);

		auto fnd = this->_boneMap.find(boneKey);
		if (fnd != this->_boneMap.end()) {
			auto& p = this->_boneCalcs[fnd->second.first];

			p = MathUtils::mtxMul(skeleton->invTransformationMtx, globalTransformation);
			p = MathUtils::mtxMul(p, fnd->second.second);
		}

		// recursively go the children of our current bone
		for (auto child : parentBone->children) {
			this->readAnim(skeleton, child, globalTransformation);
		}*/
	}

	// Animations ----
	bool Model::playAnimation(const std::string& name, bool loop, float speed) {
		auto iter = this->_animations.find(name);
		if (iter == this->_animations.end()) throw std::runtime_error(fmt::format("[RawrBox-Model] Animation {} not found!", name));
		if (this->_currentAnimation != nullptr) return false; // Already playing one

		// Add it
		this->_currentAnimation = std::make_unique<PlayingAnimationData>(name,
		    loop,
		    speed,
		    0.0f,
		    &iter->second);

		return true;
	}

	bool Model::stopAnimation() {
		if (this->_currentAnimation == nullptr) return false;
		this->_currentAnimation = nullptr;

		return true;
	}
	// -----

	void Model::draw(const rawrBox::Vector3f& camPos) {
		ModelBase::draw(camPos);

		this->preDraw();
		for (auto& mesh : this->_meshes) {
			this->_material->process(mesh);

			bgfx::setVertexBuffer(0, this->_vbh, mesh->baseVertex, mesh->totalVertex);
			bgfx::setIndexBuffer(this->_ibh, mesh->baseIndex, mesh->totalIndex);

			float matrix[16];
			bx::mtxMul(matrix, mesh->offsetMatrix.data(), this->_matrix.data());
			bgfx::setTransform(matrix);

			if (this->_material->hasUniform("u_displayBone")) {
				float a[]{1};
				bgfx::setUniform(this->_material->getUniform("u_displayBone"), a);
			}

			if (this->_material->hasUniform("u_bones")) {
				this->updateBones(mesh);
			}

			uint64_t flags = BGFX_STATE_DEFAULT_3D | mesh->culling | mesh->blending;
			if (mesh->wireframe) flags |= BGFX_STATE_PT_LINES;

			bgfx::setState(flags, 0);
			this->_material->postProcess();
		}
		this->postDraw();
	}
} // namespace rawrBox
