#pragma once

#include <rawrbox/render/model/light/directional.hpp>
#include <rawrbox/render/model/light/manager.hpp>
#include <rawrbox/render/model/light/point.hpp>
#include <rawrbox/render/model/light/spot.hpp>
#include <rawrbox/render/model/mesh.hpp>
#include <rawrbox/render/model/model.hpp>
#include <rawrbox/render/texture/image.hpp>
#include <rawrbox/utils/string.hpp>

#include <assimp/GltfMaterial.h>
#include <assimp/anim.h>
#include <assimp/cimport.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <filesystem>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>

#define DEFAULT_ASSIMP_FLAGS (aiProcessPreset_TargetRealtime_Fast | aiProcess_GenBoundingBoxes | aiProcess_ConvertToLeftHanded | aiProcess_RemoveRedundantMaterials)

namespace rawrbox {
	// NOLINTBEGIN{unused-const-variable}
	namespace ModelLoadFlags {
		const uint32_t NONE = 0;
		const uint32_t IMPORT_LIGHT = 1 << 1;
		const uint32_t IMPORT_TEXTURES = 1 << 2;
		const uint32_t IMPORT_ANIMATIONS = 1 << 3;

		namespace Debug {
			const uint32_t PRINT_BONE_STRUCTURE = 1 << 4;
		}
	}; // namespace ModelLoadFlags
	// NOLINTEND{unused-const-variable}

	template <typename M = rawrbox::MaterialBase>
	class ModelImported : public rawrbox::Model<M> {
		using Model<M>::Model;

		std::string _fileName;
		std::unordered_map<std::string, std::shared_ptr<rawrbox::TextureBase>> _textures;

		uint32_t _loadFlags;
		uint32_t _assimpFlags;

		// TEXTURE LOADING -----
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

		std::shared_ptr<rawrbox::TextureBase> importTexture(const std::string& path, const std::string& name, const std::array<aiTextureMapMode, 3>& mode) {
			auto textPath = std::filesystem::path(path);
			auto base = std::filesystem::path(this->_fileName);

			std::string finalPath = fmt::format("{}/{}", base.parent_path().generic_string(), textPath.generic_string());

			auto fnd = this->_textures.find(finalPath);
			if (fnd == this->_textures.end()) {
				auto texture = std::make_shared<rawrbox::TextureImage>(finalPath);

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

		void loadTextures(const aiScene* sc, aiMesh& assimp, std::shared_ptr<rawrbox::Mesh<typename M::vertexBufferType>> mesh) {
			if (sc->mNumMaterials <= 0 || assimp.mMaterialIndex > sc->mNumMaterials) return;
			const aiMaterial* pMaterial = sc->mMaterials[assimp.mMaterialIndex];

			aiString matName;
			pMaterial->Get(AI_MATKEY_NAME, matName);

			float opacity = 1.F;
			pMaterial->Get(AI_MATKEY_OPACITY, opacity);

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

			mesh->setSpecularTexture(rawrbox::MISSING_SPECULAR_TEXTURE, 25.F); // Default
			mesh->setEmissionTexture(rawrbox::MISSING_SPECULAR_TEXTURE, 1.F);  // Default

			// TEXTURE DIFFUSE / BASE_COLOR
			if (pMaterial->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
				if (pMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &matPath, nullptr, nullptr, nullptr, nullptr, matMode.data()) == AI_SUCCESS) {
					auto ptr = this->importTexture(matPath.data, matName.data, matMode);
					if (ptr == nullptr) throw std::runtime_error(fmt::format("[RawrBox-Assimp] Failed to load diffuse texture '{}'", matPath.data));

					aiColor3D color;
					pMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, color);

					mesh->setColor(rawrbox::Color(color.r, color.g, color.b, opacity));
					mesh->setTexture(ptr);
				}
			} else {
				// Default painted texture ----
				aiColor3D flatColor;
				pMaterial->Get(AI_MATKEY_BASE_COLOR, flatColor);

				mesh->setColor(rawrbox::Color(flatColor.r, flatColor.g, flatColor.b, opacity));
				mesh->setTexture(rawrbox::WHITE_TEXTURE);
				// -----------
			}
			// --------------------

			if constexpr (supportsNormals<typename M::vertexBufferType>) {
				// EMISSION ------
				if (pMaterial->GetTextureCount(aiTextureType_EMISSION_COLOR) > 0) {
					if (pMaterial->GetTexture(aiTextureType_EMISSION_COLOR, 0, &matPath, nullptr, nullptr, nullptr, nullptr, matMode.data()) == AI_SUCCESS) {
						auto ptr = this->importTexture(matPath.data, matName.data, matMode);
						if (ptr == nullptr) throw std::runtime_error(fmt::format("[RawrBox-Assimp] Failed to load emissive texture '{}'", matPath.data));

						aiColor3D color;
						pMaterial->Get(AI_MATKEY_COLOR_EMISSIVE, color);

						float intensity = 1.F;
						pMaterial->Get(AI_MATKEY_EMISSIVE_INTENSITY, intensity);

						mesh->setEmissionColor(rawrbox::Color(color.r, color.g, color.b, opacity));
						mesh->setEmissionTexture(ptr, intensity);
					}
				}
				// --------------------
				// SPECULAR -----------
				if (pMaterial->GetTextureCount(aiTextureType_SPECULAR) > 0) {
					if (pMaterial->GetTexture(aiTextureType_SPECULAR, 0, &matPath, nullptr, nullptr, nullptr, nullptr, matMode.data()) == AI_SUCCESS) {
						auto ptr = this->importTexture(matPath.data, matName.data, matMode);
						if (ptr == nullptr) throw std::runtime_error(fmt::format("[RawrBox-Assimp] Failed to load specular texture '{}'", matPath.data));

						float shininess = 0;
						pMaterial->Get(AI_MATKEY_SHININESS, shininess);

						aiColor3D color;
						pMaterial->Get(AI_MATKEY_COLOR_SPECULAR, color);

						mesh->setSpecularColor(rawrbox::Color(color.r, color.g, color.b, opacity));
						mesh->setSpecularTexture(ptr, shininess);
					}
				}
				// ---------------------
			}
		}
		/// -------

		// SKELETON LOADING -----
		void loadSkeleton(const aiScene* sc, std::shared_ptr<rawrbox::Mesh<typename M::vertexBufferType>> mesh, const aiMesh& aiMesh) {
			if (!aiMesh.HasBones()) return;

			for (size_t i = 0; i < aiMesh.mNumBones; i++) {
				aiBone* bone = aiMesh.mBones[i];
				if (bone->mArmature == nullptr) continue;

				// Armature parsing
				std::string name = std::string(bone->mArmature->mName.data);
				std::string boneName = std::string(bone->mName.data);
				std::string boneKey = fmt::format("{}-{}", name, boneName);

				// Armature does not exist, this should be the root bone
				if (this->_skeletons.find(name) == this->_skeletons.end()) {
					auto armature = std::make_shared<Skeleton>(name);

					// Create root bone ----
					armature->rootBone = std::make_shared<Bone>("ROOT-BONE");
					armature->rootBone->owner = armature.get();

					armature->invTransformationMtx.transpose(&sc->mRootNode->mTransformation.a1);
					armature->invTransformationMtx.inverse();

					armature->rootBone->transformationMtx.transpose(&bone->mArmature->mTransformation.a1);
					//  ---------------------

					this->generateSkeleton(armature, bone->mArmature, armature->rootBone);

					// DEBUG ----
					if ((this->_loadFlags & rawrbox::ModelLoadFlags::Debug::PRINT_BONE_STRUCTURE) > 0) {
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
					}
					// -------------

					this->_skeletons[name] = std::move(armature);
				}
				// ---------------

				// Set armature ---
				mesh->skeleton = this->_skeletons[name];
				// ----

				// Apply the weights -----
				auto fnd = this->_globalBoneMap.find(boneKey);
				if (fnd == this->_globalBoneMap.end()) throw std::runtime_error(fmt::format("[RawrBox-Assimp] Failed to map bone {}", boneKey));

				fnd->second->offsetMtx.transpose(&bone->mOffsetMatrix.a1);
				fnd->second->offsetMtx.mul(mesh->offsetMatrix);

				// Calculate object weights
				if constexpr (supportsBones<typename M::vertexBufferType>) {
					for (size_t j = 0; j < bone->mNumWeights; j++) {
						auto& weightobj = bone->mWeights[j];
						mesh->vertices[mesh->baseVertex + weightobj.mVertexId].addBoneData(fnd->second->boneId, weightobj.mWeight); // Global vertices
					}
				}
				// ------
			}
		}

		void generateSkeleton(std::shared_ptr<Skeleton> skeleton, const aiNode* pNode, std::shared_ptr<rawrbox::Bone> parent) {
			for (size_t i = 0; i < pNode->mNumChildren; i++) {
				auto child = pNode->mChildren[i];

				std::string boneName = child->mName.data;
				std::string boneKey = fmt::format("{}-{}", skeleton->name, boneName);

				std::shared_ptr<rawrbox::Bone> bone = std::make_shared<rawrbox::Bone>(boneKey);
				bone->parent = parent;
				bone->owner = skeleton.get();
				bone->boneId = static_cast<uint8_t>(this->_globalBoneMap.size());
				bone->transformationMtx.transpose(&child->mTransformation.a1);

				this->_globalBoneMap[boneKey] = bone;

				this->generateSkeleton(skeleton, child, bone);
				parent->children.push_back(std::move(bone));
			}
		}

		AnimBehaviour assimpBehavior(aiAnimBehaviour b) {
			switch (b) {
				case aiAnimBehaviour_CONSTANT:
					return AnimBehaviour::CONSTANT;
				default:
					return AnimBehaviour::LERP;
			}
		}

		aiNode* findRootSkeleton(const aiScene* sc, const std::string& meshName) {
			// Attempt to find armature
			auto aiNode = sc->mRootNode->FindNode(meshName.c_str());
			if (aiNode == nullptr) throw std::runtime_error(fmt::format("[RawrBox-Model] Failed to find node '{}' on scene root", meshName));

			auto fnd = this->_skeletons.find(aiNode->mName.data);
			if (fnd != this->_skeletons.end()) return aiNode;

			while (true) {
				if (aiNode->mParent == nullptr) return nullptr;

				auto fnd = this->_skeletons.find(aiNode->mParent->mName.data);
				if (fnd != this->_skeletons.end()) return aiNode->mParent;

				aiNode = aiNode->mParent;
			}
			// --------

			return nullptr;
		}

		void markMeshAnimated(const std::string& meshName, const std::string& search) {
			auto m = std::find_if(this->_meshes.begin(), this->_meshes.end(), [&](std::shared_ptr<rawrbox::Mesh<typename M::vertexBufferType>> x) { return x->name == search; });
			if (m == this->_meshes.end()) return;

			(*m)->setMergeable(false); // Has animation, don't merge
			this->_animatedMeshes[meshName] = *m;
		}

		void loadAnimations(const aiScene* sc) {
			if (!sc->HasAnimations()) return;

			// Load animations ----
			for (size_t i = 0; i < sc->mNumAnimations; i++) {
				auto& anim = *sc->mAnimations[i];

				std::string animName = anim.mName.data;
				if (animName.empty()) animName = fmt::format("anim_{}", i);

				auto spl = rawrbox::StrUtils::split(animName, '|');

				// create an entry in the mapping and start filling it with data
				auto& ourAnim = this->_animations[spl.back()];

				ourAnim.ticksPerSecond = static_cast<float>(anim.mTicksPerSecond);
				ourAnim.duration = static_cast<float>(anim.mDuration);

				// for each channel (frame / keyframe)
				// extract position, rotation, scale and timings
				for (size_t channelIndex = 0; channelIndex < anim.mNumChannels; channelIndex++) {
					auto aChannel = anim.mChannels[channelIndex];
					std::string meshName = aChannel->mNodeName.data;

					rawrbox::AnimationFrame ourChannel = {};

					// ANIMATION MAPPING -----
					auto pNode = this->findRootSkeleton(sc, meshName.c_str());
					if (pNode != nullptr) {
						std::string armature = pNode->mName.data;
						if (armature == meshName) continue;

						ourChannel.nodeName = fmt::format("{}-{}", armature, meshName);
					}

					// Could not find a skeleton / bone, probably a vertice anim then
					if (ourChannel.nodeName.empty()) {
						// Mark meshes as animated for quick lookup ----
						pNode = sc->mRootNode->FindNode(meshName.c_str());
						if (pNode == nullptr) throw std::runtime_error(fmt::format("[RawrBox-Model] Failed to find animated mesh '{}'", meshName));

						for (size_t i = 0; i < pNode->mNumChildren; i++) {
							this->markMeshAnimated(meshName, pNode->mChildren[i]->mName.data);
						}

						for (size_t n = 0; n < pNode->mNumMeshes; ++n) {
							this->markMeshAnimated(meshName, sc->mMeshes[pNode->mMeshes[n]]->mName.data);
						}

						ourChannel.nodeName = meshName;
						// ----------------------
					}
					// -------------------------

					ourChannel.stateStart = assimpBehavior(aChannel->mPreState);
					ourChannel.stateEnd = assimpBehavior(aChannel->mPostState);

					for (size_t positionIndex = 0; positionIndex < aChannel->mNumPositionKeys; positionIndex++) {
						auto aPos = aChannel->mPositionKeys[positionIndex];
						ourChannel.position.push_back({static_cast<float>(aPos.mTime), {aPos.mValue.x, aPos.mValue.y, aPos.mValue.z}});
					}

					for (size_t scaleIndex = 0; scaleIndex < aChannel->mNumScalingKeys; scaleIndex++) {
						auto aScale = aChannel->mScalingKeys[scaleIndex];
						ourChannel.scale.push_back({static_cast<float>(aScale.mTime), {aScale.mValue.x, aScale.mValue.y, aScale.mValue.z}});
					}

					for (size_t rotationIndex = 0; rotationIndex < aChannel->mNumRotationKeys; rotationIndex++) {
						auto aRot = aChannel->mRotationKeys[rotationIndex];
						ourChannel.rotation.push_back({static_cast<float>(aRot.mTime), {aRot.mValue.x, aRot.mValue.y, aRot.mValue.z, aRot.mValue.w}});
					}

					ourAnim.frames.push_back(ourChannel);
				}
			}
		}
		/// -------

		// LIGHT LOADING -----
		void loadLights(const aiScene* sc) {
			if (!sc->HasLights()) return;

			for (size_t n = 0; n < sc->mNumLights; ++n) {
				auto& aiLight = *sc->mLights[n];

				auto lightNode = sc->mRootNode->FindNode(aiLight.mName.data);
				if (lightNode == nullptr) continue;

				rawrbox::Vector3f pos = rawrbox::Vector3f(aiLight.mPosition.x, aiLight.mPosition.y, aiLight.mPosition.z) + this->getPos();
				rawrbox::Vector3f direction = rawrbox::Vector3f(aiLight.mDirection.x, aiLight.mDirection.y, aiLight.mDirection.z).normalized();

				aiVector3D p;
				aiQuaternion q;

				lightNode->mTransformation.DecomposeNoScaling(q, p);
				pos += {p.x, p.y, p.z};

				auto parent = lightNode->mParent;
				if (parent != nullptr) {
					parent->mTransformation.DecomposeNoScaling(q, p);
					pos += {p.x, p.y, p.z};
				}

				auto diffuse = rawrbox::Color(aiLight.mColorDiffuse.r, aiLight.mColorDiffuse.g, aiLight.mColorDiffuse.b, 1.F) / 255.F;
				auto specular = rawrbox::Color(aiLight.mColorSpecular.r, aiLight.mColorSpecular.g, aiLight.mColorSpecular.b, 1.F) / 255.F;

				switch (aiLight.mType) {
					case aiLightSource_DIRECTIONAL:
						rawrbox::LightManager::get().addLight(std::make_shared<rawrbox::LightDirectional>(pos, direction, diffuse, specular));
						continue;
					case aiLightSource_SPOT:
						rawrbox::LightManager::get().addLight(std::make_shared<rawrbox::LightSpot>(pos, direction, diffuse, specular, aiLight.mAngleInnerCone, aiLight.mAngleOuterCone, aiLight.mAttenuationConstant, aiLight.mAttenuationLinear, aiLight.mAttenuationQuadratic));
						continue;
					case aiLightSource_POINT:
						rawrbox::LightManager::get().addLight(std::make_shared<rawrbox::LightPoint>(pos, diffuse, specular, aiLight.mAttenuationConstant, aiLight.mAttenuationLinear, aiLight.mAttenuationQuadratic));
						continue;
					default:
						continue;
				}
			}
		}
		/// -------

		// MESH LOADING -----
		void loadSubmeshes(const aiScene* sc, const aiNode* root, std::shared_ptr<rawrbox::Mesh<typename M::vertexBufferType>> parent) {
			std::shared_ptr<rawrbox::Mesh<typename M::vertexBufferType>> mesh = nullptr;

			for (size_t n = 0; n < root->mNumMeshes; ++n) {
				aiMesh& aiMesh = *sc->mMeshes[root->mMeshes[n]];

				mesh = std::make_shared<rawrbox::Mesh<typename M::vertexBufferType>>();
				mesh->setName(aiMesh.mName.data);
				mesh->parent = parent;

				// Offset for rendering
				mesh->baseVertex = static_cast<uint16_t>(mesh->vertices.size());
				mesh->baseIndex = static_cast<uint16_t>(mesh->indices.size());
				// ----

				// Calculate bbox ---
				auto min = aiMesh.mAABB.mMin;
				auto max = aiMesh.mAABB.mMax;

				mesh->bbox.m_min = {min.x, min.y, min.z};
				mesh->bbox.m_max = {max.x, max.y, max.z};
				mesh->bbox.m_size = mesh->bbox.m_min.abs() + mesh->bbox.m_max.abs();
				// -----

				if ((this->_assimpFlags & aiProcess_PreTransformVertices) == 0) {
					mesh->offsetMatrix.transpose(&root->mTransformation.a1); // Append matrix to our vertices, since pre-transform is disabled
				}

				// Textures
				if ((this->_loadFlags & rawrbox::ModelLoadFlags::IMPORT_TEXTURES) > 0) {
					this->loadTextures(sc, aiMesh, mesh);
				} else {
					mesh->setCulling(BGFX_STATE_CULL_CCW); // Default cullingf or assimp
				}

				// Vertices
				for (size_t i = 0; i < aiMesh.mNumVertices; i++) {
					typename M::vertexBufferType v;

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
						v.abgr = Colorf{col.r, col.g, col.b, col.a}.pack();
					}

					if constexpr (supportsNormals<typename M::vertexBufferType>) {
						if (aiMesh.HasNormals()) {
							auto& normal = aiMesh.mNormals[i];
							v.normal[0] = rawrbox::PackUtils::packNormal(normal.x, normal.y, normal.z);
						}

						if (aiMesh.HasTangentsAndBitangents()) {
							auto& tangents = aiMesh.mTangents[i];
							v.normal[1] = rawrbox::PackUtils::packNormal(tangents.x, tangents.y, tangents.z);

							// auto& bitangents = aiMesh.mBitangents[i];
							// v.normal[2] = rawrbox::PackUtils::packNormal(bitangents.x, bitangents.y, bitangents.z);
						}
					}

					mesh->vertices.push_back(v);
				}

				// Indices
				for (size_t t = 0; t < aiMesh.mNumFaces; ++t) {
					auto& face = aiMesh.mFaces[t];
					if (face.mNumIndices != 3) continue; // we only do triangles

					for (size_t i = 0; i < face.mNumIndices; i++) {
						mesh->indices.push_back(static_cast<uint16_t>(static_cast<uint16_t>(mesh->vertices.size()) - face.mIndices[i]));
					}
				}

				// Bones
				if ((this->_loadFlags & rawrbox::ModelLoadFlags::IMPORT_ANIMATIONS) > 0 && aiMesh.HasBones()) {
					this->loadSkeleton(sc, mesh, aiMesh);
				}
				// -------------------

				mesh->totalVertex = static_cast<uint16_t>(mesh->vertices.size());
				mesh->totalIndex = static_cast<uint16_t>(mesh->indices.size());

				this->addMesh(mesh);
			}

			// recursive
			for (size_t n = 0; n < root->mNumChildren; ++n) {
				this->loadSubmeshes(sc, root->mChildren[n], mesh);
			}
		}
		/// -------

	public:
		~ModelImported() override {
			this->_textures.clear();
		}

		ModelImported(ModelImported&&) = delete;
		ModelImported& operator=(ModelImported&&) = delete;
		ModelImported(const ModelImported&) = delete;
		ModelImported& operator=(const ModelImported&) = delete;

		// Loading ----
		void load(const std::string& path, uint32_t loadFlags = ModelLoadFlags::NONE, uint32_t assimpFlags = DEFAULT_ASSIMP_FLAGS) {
			this->_meshes.clear(); // Clear old meshes

			this->_fileName = path;
			this->_loadFlags = loadFlags;
			this->_assimpFlags = assimpFlags;

			if ((this->_loadFlags & rawrbox::ModelLoadFlags::IMPORT_ANIMATIONS) > 0) {
				this->_assimpFlags |= aiProcess_PopulateArmatureData | aiProcess_OptimizeGraph | aiProcess_LimitBoneWeights; // Enable armature & limit bones
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
			if ((this->_loadFlags & rawrbox::ModelLoadFlags::IMPORT_LIGHT) > 0) this->loadLights(scene);
			if ((this->_loadFlags & rawrbox::ModelLoadFlags::IMPORT_ANIMATIONS) > 0) this->loadAnimations(scene);
			// ----

			aiReleaseImport(scene);
		}
		// ---
	};
} // namespace rawrbox
