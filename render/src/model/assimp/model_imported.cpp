
#include <rawrbox/math/vector3.hpp>
#include <rawrbox/render/model/assimp/model_imported.h>
#include <rawrbox/render/model/light/directional.hpp>
#include <rawrbox/render/model/light/manager.h>
#include <rawrbox/render/model/light/point.hpp>
#include <rawrbox/render/model/light/spot.hpp>
#include <rawrbox/render/texture/image.h>
#include <rawrbox/utils/pack.hpp>
#include <rawrbox/utils/string.hpp>

#include <assimp/GltfMaterial.h>
#include <assimp/material.h>
#include <assimp/mesh.h>
#include <bx/math.h>
#include <fmt/format.h>

#include <cstdint>
#include <filesystem>
#include <functional>
#include <memory>
#include <stdexcept>

#include "rawrbox/render/model/base.hpp"

// NOLINTBEGIN(cppcoreguidelines-pro-bounds-pointer-arithmetic)

namespace rawrBox {
	ModelImported::~ModelImported() {
		this->_textures.clear();
	}

	// LOADING -----
	void ModelImported::load(const std::string& path, uint32_t loadFlags, uint32_t assimpFlags) {
		this->_meshes.clear(); // Clear old meshes

		this->_fileName = path;
		this->_loadFlags = loadFlags;
		this->_assimpFlags = assimpFlags;

		if ((this->_loadFlags & rawrBox::ModelLoadFlags::IMPORT_ANIMATIONS) > 0) {
			this->_assimpFlags |= aiProcess_PopulateArmatureData | aiProcess_OptimizeGraph; // Enable armature & limit bones
		} else {
			this->_assimpFlags |= aiProcess_PreTransformVertices; // Enable PreTransformVertices for optimization
		}

		const aiScene* scene = aiImportFile(path.c_str(), this->_assimpFlags);
		if (scene == nullptr) {
			auto error = aiGetErrorString(); // Because vscode doesn't print the error bellow
			throw std::runtime_error(fmt::format("[Resources] Content 'model' error: {}: '{}'\n", path, error));
		}

		// load models
		this->loadSubmeshes(scene, scene->mRootNode, nullptr);
		if ((this->_loadFlags & rawrBox::ModelLoadFlags::IMPORT_LIGHT) > 0) this->loadLights(scene);
		if ((this->_loadFlags & rawrBox::ModelLoadFlags::IMPORT_ANIMATIONS) > 0) this->loadAnimations(scene);
		// ----

		aiReleaseImport(scene);
	}

	void ModelImported::loadSkeleton(std::shared_ptr<Mesh> mesh, const aiMesh& aiMesh) {
		if (!aiMesh.HasBones()) return;

		for (size_t i_bone = 0; i_bone < aiMesh.mNumBones; i_bone++) {
			aiBone* bone = aiMesh.mBones[i_bone];
			if (bone->mArmature == nullptr) continue;

			// Armature parsing
			std::string name = std::string(bone->mArmature->mName.data);
			std::string boneName = std::string(bone->mName.data);

			// Armature does not exist, this should be the root bone
			if (this->_skeletons.find(name) == this->_skeletons.end()) {
				auto armature = std::make_shared<Skeleton>(name);
				armature->rootBone = std::make_shared<Bone>("ARMATURE-ROOT");

				bx::mtxIdentity(armature->rootBone->transformationMtx.data());
				bx::mtxTranspose(armature->rootBone->transformationMtx.data(), &bone->mArmature->mTransformation.a1);

				bx::mtxIdentity(armature->invTransformationMtx.data());
				bx::mtxInverse(armature->invTransformationMtx.data(), &bone->mArmature->mTransformation.a1);

				this->generateSkeleton(armature, bone->mArmature, armature->rootBone);

				// DEBUG ----
				std::function<void(std::shared_ptr<Bone>, int)> printBone;
				printBone = [&printBone](std::shared_ptr<Bone> bn, int deep) -> void {
					for (auto c : bn->children) {
						std::string d = "";
						for (size_t i = 0; i < deep; i++)
							d += "\t";

						fmt::print("{}[{}] {}\n", d, c->boneId, c->name);
						printBone(c, ++deep);
					}
				};

				printBone(armature->rootBone, 0);
				this->_skeletons[name] = std::move(armature);
			}
			// ---------------

			// Apply the weights -----
			std::string boneKey = fmt::format("{}-{}", name, boneName);

			auto fnd = this->_boneMap.find(boneKey);
			if (fnd == this->_boneMap.end()) throw std::runtime_error(fmt::format("[RawrBox-Assimp] Failed to map bone {}", boneKey));
			bx::mtxTranspose(fnd->second.second.data(), &bone->mOffsetMatrix.a1);

			mesh->skeleton = this->_skeletons[name];

			// Calculate object weights
			for (size_t j = 0; j < bone->mNumWeights; j++) {
				auto& weightobj = bone->mWeights[j];
				mesh->vertices[mesh->baseVertex + weightobj.mVertexId].addBoneData(fnd->second.first, weightobj.mWeight); // Global vertices
			}
			// ------
		}
	}

	void ModelImported::generateSkeleton(std::shared_ptr<Skeleton> skeleton, const aiNode* pNode, std::shared_ptr<rawrBox::Bone> parent) {
		for (size_t i = 0; i < pNode->mNumChildren; i++) {
			auto child = pNode->mChildren[i];

			std::string boneName = child->mName.data;
			std::string boneKey = fmt::format("{}-{}", skeleton->name, boneName);

			if (this->_boneMap.find(boneKey) == this->_boneMap.end()) { // Bone does not exist in our global map?
				this->_boneMap[boneKey] = {static_cast<uint8_t>(this->_boneMap.size()), {}};
			}

			std::shared_ptr<rawrBox::Bone> bone = std::make_shared<rawrBox::Bone>(boneName);
			bone->parent = parent;
			bone->boneId = this->_boneMap[boneKey].first;

			bx::mtxTranspose(bone->transformationMtx.data(), &pNode->mTransformation.a1);

			this->generateSkeleton(skeleton, child, bone);
			parent->children.push_back(std::move(bone));
		}
	}

	uint64_t assimpSamplerToBGFX(const std::array<aiTextureMapMode, 3>& mode, int axis) {
		uint64_t flags = 0;

		switch (mode[axis]) {
			case aiTextureMapMode_Clamp:
				if (axis == 0)
					flags |= BGFX_SAMPLER_U_CLAMP;
				else if (axis == 1)
					flags |= BGFX_SAMPLER_V_CLAMP;
				else if (axis == 2)
					flags |= BGFX_SAMPLER_W_CLAMP;
				break;
			case aiTextureMapMode_Decal:
				if (axis == 0)
					flags |= BGFX_SAMPLER_U_BORDER;
				else if (axis == 1)
					flags |= BGFX_SAMPLER_V_BORDER;
				else if (axis == 2)
					flags |= BGFX_SAMPLER_W_BORDER;
				break;
			case aiTextureMapMode_Mirror:
				if (axis == 0)
					flags |= BGFX_SAMPLER_U_MIRROR;
				else if (axis == 1)
					flags |= BGFX_SAMPLER_V_MIRROR;
				else if (axis == 2)
					flags |= BGFX_SAMPLER_W_MIRROR;
				break;
			default: break; // WRAP
		}

		return flags;
	}

	std::shared_ptr<rawrBox::TextureBase> ModelImported::importTexture(const std::string& path, const std::string& name, const std::array<aiTextureMapMode, 3>& mode) {
		auto textPath = std::filesystem::path(path);
		auto base = std::filesystem::path(this->_fileName);

		std::string finalPath = fmt::format("{}/{}", base.parent_path().generic_string(), textPath.generic_string());

		auto fnd = this->_textures.find(finalPath);
		if (fnd == this->_textures.end()) {
			auto texture = std::make_shared<rawrBox::TextureImage>(finalPath);

			// Setup flags ----
			auto flags = BGFX_SAMPLER_MIN_POINT | BGFX_SAMPLER_MAG_POINT;
			flags |= assimpSamplerToBGFX(mode, 0); // u
			flags |= assimpSamplerToBGFX(mode, 1); // v
			flags |= assimpSamplerToBGFX(mode, 2); // w

			texture->setFlags(flags);
			// ----
			texture->setName(name);
			texture->upload(bgfx::TextureFormat::Count);
			this->_textures[finalPath] = std::move(texture);
			return this->_textures[finalPath];
		} else {
			return fnd->second;
		}
	}

	void ModelImported::loadTextures(const aiScene* sc, aiMesh& assimp, std::shared_ptr<rawrBox::Mesh>& mesh) {
		if (sc->mNumMaterials <= 0 || assimp.mMaterialIndex > sc->mNumMaterials) return;

		const aiMaterial* pMaterial = sc->mMaterials[assimp.mMaterialIndex];

		aiString matName;
		pMaterial->Get(AI_MATKEY_NAME, matName);

		bool matWireframe = false;
		pMaterial->Get(AI_MATKEY_ENABLE_WIREFRAME, matWireframe);
		mesh->setWireframe(matWireframe);

		bool matDisableCulling = false;
		pMaterial->Get(AI_MATKEY_TWOSIDED, matDisableCulling);
		mesh->setCulling(matDisableCulling ? 0 : BGFX_STATE_CULL_CCW);

		aiBlendMode blending = aiBlendMode_Default;
		pMaterial->Get(AI_MATKEY_BLEND_FUNC, blending);
		switch (blending) {
				/*
				case aiBlendMode_Mix:
					mesh->setBlend(BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA));
					break;

				case aiBlendMode_Sub:
					mesh->setBlend(BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_ONE) | BGFX_STATE_BLEND_EQUATION(BGFX_STATE_BLEND_EQUATION_REVSUB));
					break;

				case aiBlendMode_Mul:
					mesh->setBlend(BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA));
					break;

				case aiBlendMode_Alpha:
					mesh->setBlend(BGFX_STATE_BLEND_ALPHA);
					break;
					*/
			case aiBlendMode_Additive:
				mesh->setBlend(BGFX_STATE_BLEND_ADD);
				break;

			default:
			case aiBlendMode_Default:
				mesh->setBlend(BGFX_STATE_BLEND_NORMAL);
				break;
		}

		aiString matPath;
		std::array<aiTextureMapMode, 3> matMode = {};

		// TEXTURE DIFFUSE
		if (pMaterial->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
			if (pMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &matPath, nullptr, nullptr, nullptr, nullptr, matMode.data()) == AI_SUCCESS) {
				auto ptr = this->importTexture(matPath.data, matName.data, matMode);
				if (ptr == nullptr) throw std::runtime_error(fmt::format("[RawrBox-Assimp] Failed to load diffuse texture '{}'", matPath.data));

				mesh->setTexture(ptr);
			}
		}

		// TEXTURE SPECULAR
		if (pMaterial->GetTextureCount(aiTextureType_SPECULAR) > 0) {
			if (pMaterial->GetTexture(aiTextureType_SPECULAR, 0, &matPath, nullptr, nullptr, nullptr, nullptr, matMode.data()) == AI_SUCCESS) {
				auto ptr = this->importTexture(matPath.data, matName.data, matMode);
				if (ptr == nullptr) throw std::runtime_error(fmt::format("[RawrBox-Assimp] Failed to load specular texture '{}'", matPath.data));

				float shininess = 0;
				pMaterial->Get(AI_MATKEY_SHININESS, shininess);

				mesh->setSpecularTexture(ptr, shininess);
			}
		}

		// TEXTURE EMISSIVE
		/*if (pMaterial->GetTextureCount(aiTextureType_EMISSIVE) > 0) {
			if (pMaterial->GetTexture(aiTextureType_EMISSIVE, 0, nullptr, nullptr, nullptr, nullptr, nullptr, matMode.data()) == AI_SUCCESS) {
				auto ptr = this->importTexture(matpath.data);
				if (ptr == nullptr) throw std::runtime_error(fmt::format("[RawrBox-Assimp] Failed to load specular texture '{}'", matpath.data));

				mesh->setSpecularTexture(ptr);
			}
		}*/
	}

	void ModelImported::loadSubmeshes(const aiScene* sc, const aiNode* root, std::shared_ptr<Mesh> parentMesh) {
		std::shared_ptr<rawrBox::Mesh> mesh = nullptr;

		for (size_t n = 0; n < root->mNumMeshes; n++) {
			aiMesh& aiMesh = *sc->mMeshes[root->mMeshes[n]];
			mesh = std::make_shared<rawrBox::Mesh>();

			mesh->setName(aiMesh.mName.data);
			mesh->parent = parentMesh;

			// Offset for rendering
			mesh->baseVertex = static_cast<uint16_t>(mesh->vertices.size());
			mesh->baseIndex = static_cast<uint16_t>(mesh->indices.size());
			// ----

			if ((this->_assimpFlags & aiProcess_PreTransformVertices) == 0) {
				bx::mtxTranspose(mesh->offsetMatrix.data(), &root->mTransformation.a1); // Append matrix to our vertices
			}

			// Textures
			if ((this->_loadFlags & rawrBox::ModelLoadFlags::IMPORT_TEXTURES) > 0) {
				this->loadTextures(sc, aiMesh, mesh);
			}

			// Vertices
			for (size_t i = 0; i < aiMesh.mNumVertices; i++) {
				rawrBox::MeshVertexData v;

				if (aiMesh.HasPositions()) {
					auto& vert = aiMesh.mVertices[i];
					v.position = {vert.x, vert.y, vert.z};
				}

				if (aiMesh.HasTextureCoords(0)) {
					auto& uv = aiMesh.mTextureCoords[0][i];
					v.uv = {uv.x, uv.y};
				}

				if (aiMesh.HasVertexColors(0)) {
					auto& col = aiMesh.mColors[0][i];
					v.abgr = rawrBox::Color::pack({col.r, col.g, col.b, col.a});
				}

				if (aiMesh.HasNormals()) {
					auto& normal = aiMesh.mNormals[i];
					v.normal[0] = rawrBox::PackUtils::packNormal(normal.x, normal.y, normal.z);
				}

				if (aiMesh.HasTangentsAndBitangents()) {
					auto& tangents = aiMesh.mTangents[i];
					v.normal[1] = rawrBox::PackUtils::packNormal(tangents.x, tangents.y, tangents.z);
				}

				mesh->vertices.push_back(v);
			}

			// Indices
			for (size_t t = 0; t < aiMesh.mNumFaces; t++) {
				auto& face = aiMesh.mFaces[t];
				if (face.mNumIndices != 3) continue; // we only do triangles

				for (size_t i = 0; i < face.mNumIndices; i++) {
					mesh->indices.push_back(static_cast<uint16_t>(static_cast<uint16_t>(mesh->vertices.size()) - face.mIndices[i]));
				}
			}

			// Bones
			if ((this->_loadFlags & rawrBox::ModelLoadFlags::IMPORT_ANIMATIONS) > 0 && aiMesh.HasBones()) {
				this->loadSkeleton(mesh, aiMesh);
			}
			// -------------------

			mesh->totalVertex = static_cast<uint16_t>(mesh->vertices.size());
			mesh->totalIndex = static_cast<uint16_t>(mesh->indices.size());

			this->addMesh(mesh);
		}

		// recursive
		for (size_t n = 0; n < root->mNumChildren; n++) {
			this->loadSubmeshes(sc, root->mChildren[n], mesh);
		}
	}

	void ModelImported::loadAnimations(const aiScene* sc) {
		if (!sc->HasAnimations()) return;

		// Load animations ----
		for (size_t i = 0; i < sc->mNumAnimations; i++) {
			auto& anim = *sc->mAnimations[i];

			std::string animName = anim.mName.data;
			if (animName.empty()) animName = fmt::format("anim_{}", i);

			auto spl = rawrBox::StrUtils::split(animName, '|');

			// create an entry in the mapping and start filling it with data
			auto& ourAnim = this->_animations[spl[spl.size() - 1]];
			ourAnim.ticksPerSecond = static_cast<float>(anim.mTicksPerSecond);
			ourAnim.duration = static_cast<float>(anim.mDuration);

			// for each channel (frame / keyframe)
			// extract position, rotation, scale and timings
			for (size_t channelIndex = 0; channelIndex < anim.mNumChannels; channelIndex++) {
				auto aChannel = anim.mChannels[channelIndex];

				rawrBox::AnimationFrame ourChannel;
				ourChannel.nodeName = aChannel->mNodeName.data;
				// ourChannel.stateStart = static_cast<AnimationChannelBehaviour>(aChannel->mPreState);
				// ourChannel.stateEnd = static_cast<AnimationChannelBehaviour>(aChannel->mPostState);

				for (size_t positionIndex = 0; positionIndex < aChannel->mNumPositionKeys; positionIndex++) {
					auto aPos = aChannel->mPositionKeys[positionIndex];
					ourChannel.position.push_back({static_cast<float>(aPos.mTime), aPos.mValue});
				}

				for (size_t scaleIndex = 0; scaleIndex < aChannel->mNumScalingKeys; scaleIndex++) {
					auto aScale = aChannel->mScalingKeys[scaleIndex];
					ourChannel.scale.push_back({static_cast<float>(aScale.mTime), aScale.mValue});
				}

				for (size_t rotationIndex = 0; rotationIndex < aChannel->mNumRotationKeys; rotationIndex++) {
					auto aRot = aChannel->mRotationKeys[rotationIndex];
					ourChannel.rotation.push_back({static_cast<float>(aRot.mTime), aRot.mValue});
				}

				ourAnim.frames.push_back(ourChannel);
			}
		}
		// --------------
	}

	void ModelImported::loadLights(const aiScene* sc) {
		if (!sc->HasLights()) return;

		for (size_t n = 0; n < sc->mNumLights; ++n) {
			auto& aiLight = *sc->mLights[n];

			auto lightNode = sc->mRootNode->FindNode(aiLight.mName.data);
			if (lightNode == nullptr) continue;

			rawrBox::Vector3f pos = rawrBox::Vector3f(aiLight.mPosition.x, aiLight.mPosition.y, aiLight.mPosition.z) + this->getPos();
			rawrBox::Vector3f direction = rawrBox::Vector3f(aiLight.mDirection.x, aiLight.mDirection.y, aiLight.mDirection.z).normalized();

			aiVector3D p;
			aiQuaternion q;

			lightNode->mTransformation.DecomposeNoScaling(q, p);
			pos += {p.x, p.y, p.z};

			auto parent = lightNode->mParent;
			if (parent != nullptr) {
				parent->mTransformation.DecomposeNoScaling(q, p);
				pos += {p.x, p.y, p.z};
			}

			auto diffuse = rawrBox::Colori(static_cast<int>(aiLight.mColorDiffuse.r), static_cast<int>(aiLight.mColorDiffuse.g), static_cast<int>(aiLight.mColorDiffuse.b)).cast<float>();
			// auto ambient = rawrBox::Colori(static_cast<int>(aiLight.mColorAmbient.r), static_cast<int>(aiLight.mColorAmbient.g), static_cast<int>(aiLight.mColorAmbient.b)).cast<float>();
			auto specular = rawrBox::Colori(static_cast<int>(aiLight.mColorSpecular.r), static_cast<int>(aiLight.mColorSpecular.g), static_cast<int>(aiLight.mColorSpecular.b)).cast<float>();

			switch (aiLight.mType) {
				case aiLightSource_DIRECTIONAL:
					rawrBox::LightManager::getInstance().addLight(std::make_shared<rawrBox::LightDirectional>(pos, direction, diffuse, specular));
					continue;
				case aiLightSource_SPOT:
					rawrBox::LightManager::getInstance().addLight(std::make_shared<rawrBox::LightSpot>(pos, direction, diffuse, specular, aiLight.mAngleInnerCone, aiLight.mAngleOuterCone, aiLight.mAttenuationConstant, aiLight.mAttenuationLinear, aiLight.mAttenuationQuadratic));
					continue;
				case aiLightSource_POINT:
					rawrBox::LightManager::getInstance().addLight(std::make_shared<rawrBox::LightPoint>(pos, diffuse, specular, aiLight.mAttenuationConstant, aiLight.mAttenuationLinear, aiLight.mAttenuationQuadratic));
					continue;
				default:
					continue;
			}
		}
	}
	// ----------
} // namespace rawrBox

// NOLINTEND(cppcoreguidelines-pro-bounds-pointer-arithmetic)
