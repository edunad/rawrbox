#include <rawrbox/render/model/light/manager.h>
#include <rawrbox/render/model/model.h>
#include <rawrbox/render/static.h>
#include <rawrbox/render/util/assimp/utils.hpp>
#include <rawrbox/utils/time.h>

#include <assimp/matrix3x3.h>
#include <assimp/quaternion.h>
#include <bx/math.h>
#include <fmt/format.h>

#include <algorithm>
#include <cstdint>
#include <memory>
#include <stdexcept>

#include "bgfx/bgfx.h"
#include "rawrbox/render/model/base.hpp"

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
		std::array<float, 16> root = {};
		bx::mtxIdentity(root.data());

		this->_boneCalcs.clear();
		this->readAnim(mesh->skeleton, mesh->skeleton->rootBone, root);

		// copy the bone matrixes inside of the shader
		std::vector<std::array<float, 16>> bones;
		for (auto pair : this->_boneCalcs)
			bones.push_back(pair.second);

		bgfx::setUniform(this->_material->getUniform("u_bones"), &bones.front(), static_cast<uint32_t>(bones.size()));
	}

	void Model::readAnim(Skeleton* skeleton, Bone& bone, const std::array<float, 16>& parentTransform) {
		if (skeleton == nullptr) return;

		// store the result of our parent bone and our current node
		std::array<float, 16> nodeTransformation = {};
		bx::mtxTranspose(nodeTransformation.data(), bone.transformationMtx.data());

		std::array<float, 16> globalTransformation = {};
		bx::mtxMul(globalTransformation.data(), parentTransform.data(), nodeTransformation.data());

		// std::array<float, 16> globalInverseTransformation = {};
		// bx::mtxMul(globalInverseTransformation.data(), parentTransform.data(), nodeTransformation.data());

		// update the final result inside the bones
		std::string boneKey = fmt::format("{}-{}", skeleton->name, bone.name);

		auto fnd = this->_boneMap.find(boneKey);
		if (fnd != this->_boneMap.end()) {
			bx::mtxMul(this->_boneCalcs[fnd->second.first].data(), globalTransformation.data(), fnd->second.second.data());
		}

		// recursively go the children of our current bone
		for (auto& child : bone.children) {
			this->readAnim(skeleton, child, globalTransformation);
		}

		// start with the main tran

		/*
		// start with the main transform of our node
		auto nodeTransform = node.transformMtx;

		// if we have animations playing
		for (auto& animation : this->_currentAnimations) {
			// find the animation bone inside of the animation
			auto animChannel = std::find_if(animation.data->frames.begin(), animation.data->frames.end(), [&](AnimationFrame& x) {
				return x.nodeName == node.name;
			});

			if (animChannel == animation.data->frames.end()) continue;

			// figure out how "fast" the animation needs to play and the current playtime of the animation
			float ticksPerSecond = animation.data->ticksPerSecond != 0 ? animation.data->ticksPerSecond : 25.0f;
			float timeInTicks = animation.time * ticksPerSecond;
			timeInTicks = std::fmod(timeInTicks, animation.data->duration);

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
			auto scaleFrameIndex = findFrameIndex(animChannel->scale);
			auto positionFrameIndex = findFrameIndex(animChannel->position);
			auto rotationFrameIndex = findFrameIndex(animChannel->rotation);

			// lerp the 3 components
			aiVector3D position = AssimpUtils::lerpPosition(timeInTicks, animChannel->position[positionFrameIndex], positionFrameIndex + 1 >= animChannel->position.size() ? animChannel->position.front() : animChannel->position[positionFrameIndex + 1]);
			aiQuaternion rotation = AssimpUtils::lerpRotation(timeInTicks, animChannel->rotation[rotationFrameIndex], rotationFrameIndex + 1 >= animChannel->rotation.size() ? animChannel->rotation.front() : animChannel->rotation[rotationFrameIndex + 1]);
			aiVector3D scale = AssimpUtils::lerpScale(timeInTicks, animChannel->scale[scaleFrameIndex], scaleFrameIndex + 1 >= animChannel->scale.size() ? animChannel->scale.front() : animChannel->scale[scaleFrameIndex + 1]);

			// position.SymMul(scale);
			aiMatrix3x3 rotMat = rotation.GetMatrix();
			// aiMatrix3x3 posMat(position); // create a 4x4 matrix

			// Convert position data from parent's bone space to the current bone's space...
			const aiVectorKey& positionKey = channel.mPositionKeys[keyIndex];
			const aiVector3D position(parentToBone * positionKey.mValue);

			// Convert rotation data into bone space...
			const aiQuatKey& rotationKey = channel.mRotationKeys[keyIndex];
			const aiQuaternion rotation(aiQuaternion(aiMatrix3x3(parentToBone)) * rotationKey.mValue);
		}

		// if we have animations playing
		for (auto& animation : animations) {
			// find the animation bone inside of the animation
			auto animChannel = std::find_if(animation.data->frames.begin(), animation.data->frames.end(), [&](AnimationFrame& x) {
				return x.nodeName == node.name;
			});

			if (animChannel == animation.data->frames.end()) continue;

			// figure out how "fast" the animation needs to play and the current playtime of the animation
			float ticksPerSecond = animation.data->ticksPerSecond != 0 ? animation.data->ticksPerSecond : 25.0f;
			float timeInTicks = animation.time * ticksPerSecond;
			timeInTicks = std::fmod(timeInTicks, animation.data->duration);

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
			auto scaleFrameIndex = findFrameIndex(animChannel->scale);
			auto positionFrameIndex = findFrameIndex(animChannel->position);
			auto rotationFrameIndex = findFrameIndex(animChannel->rotation);

			// lerp the 3 components
			auto position = AssimpUtils::lerpPosition(timeInTicks, animChannel->position[positionFrameIndex], positionFrameIndex + 1 >= animChannel->position.size() ? animChannel->position.front() : animChannel->position[positionFrameIndex + 1]);
			auto rotation = AssimpUtils::lerpRotation(timeInTicks, animChannel->rotation[rotationFrameIndex], rotationFrameIndex + 1 >= animChannel->rotation.size() ? animChannel->rotation.front() : animChannel->rotation[rotationFrameIndex + 1]);
			auto scale = AssimpUtils::lerpScale(timeInTicks, animChannel->scale[scaleFrameIndex], scaleFrameIndex + 1 >= animChannel->scale.size() ? animChannel->scale.front() : animChannel->scale[scaleFrameIndex + 1]);

			// use GLM due Mainframe does not have quaterions :(
			auto glmRotation = bx::Quaternion(rotation.w, rotation.x, rotation.y, rotation.z);

			bx::mtxIdentity(nodeTransform.data());

			float a[16];
			bx::mtxIdentity(a);
			bx::mtxTranslate(a, position.x, position.y, position.z);

			float b[16];
			bx::mtxIdentity(b);
			bx::mtxScale(b, scale.x, scale.y, scale.z);

			bx::mtxMul(nodeTransform.data(), a, b);

			float c[16];
			bx::mtxIdentity(c);
			bx::mtxFromQuaternion(c, rotation, {position.x, position.y, position.z});

			bx::mtxMul(nodeTransform.data(), nodeTransform.data(), c);
		}

		// store the result of our parent bone and our current node
		std::array<float, 16> glbTf = {};
		bx::mtxMul(glbTf.data(), parentTransform.data(), nodeTransform.data());

		// update the final result inside the bones
		if (boneMapping.find(node.name) != boneMapping.end()) {
			auto& boneInfo = boneMapping[node.name];

			bx::mtxMul(boneInfo.computedOffsetMtx.data(), inverse.data(), glbTf.data());
			bx::mtxMul(boneInfo.computedOffsetMtx.data(), boneInfo.computedOffsetMtx.data(), boneInfo.offsetMtx.data());
		}

		// recursively go the children of our current bone
		for (auto& c : node.childs) {
			this->readAnim(inverse, boneMapping, animations, c, glbTf);
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

		/*this->_material->process(this->_meshes[0]);
		bgfx::setVertexBuffer(0, this->_vbh);
		bgfx::setIndexBuffer(this->_ibh);

		bgfx::setState(BGFX_STATE_DEFAULT_3D, 0);
		this->_material->postProcess();*/

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
