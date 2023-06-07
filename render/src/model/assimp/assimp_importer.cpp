
#include <rawrbox/render/model/assimp/assimp_importer.hpp>
#include <rawrbox/render/texture/image.hpp>
#include <rawrbox/utils/string.hpp>

#include <magic_enum.hpp>

#include <assimp/cimport.h>
#include <fmt/format.h>

namespace rawrbox {

	// TEXTURE LOADING -----
	uint64_t AssimpImporter::assimpSamplerToBGFX(const std::array<aiTextureMapMode, 3>& mode, int axis) {
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

	std::vector<std::shared_ptr<rawrbox::TextureBase>> AssimpImporter::importTexture(const aiMaterial* mat, aiTextureType type, bgfx::TextureFormat::Enum format) {
		std::vector<std::shared_ptr<rawrbox::TextureBase>> _textures = {};

		int count = mat->GetTextureCount(type);
		if (count <= 0) return _textures;

		for (size_t i = 0; i < count; i++) {
			aiString matPath;
			std::array<aiTextureMapMode, 3> matMode = {};

			if (mat->GetTexture(type, count - 1, &matPath, nullptr, nullptr, nullptr, nullptr, matMode.data()) == AI_SUCCESS) {
				auto textPath = std::filesystem::path(matPath.data);

				auto parentFolder = std::filesystem::path(this->fileName).parent_path();
				std::string basename = textPath.filename().generic_string();

				std::string loadPath = fmt::format("{}/{}", parentFolder.generic_string(), textPath.generic_string());
				if (!std::filesystem::exists(loadPath)) {
					// Check relative
					loadPath = fmt::format("{}/{}", parentFolder.generic_string(), basename);
					if (!std::filesystem::exists(loadPath)) {
						fmt::print("[RawrBox-Assimp] Failed to load texture '{}'\n", matPath.data);
						_textures.push_back(rawrbox::MISSING_TEXTURE);
						return _textures;
					}
				}

				auto texture = std::make_shared<rawrbox::TextureImage>(loadPath);

				// Setup flags ----
				auto flags = BGFX_SAMPLER_MIN_POINT | BGFX_SAMPLER_MAG_POINT;
				flags |= assimpSamplerToBGFX(matMode, 0); // u
				flags |= assimpSamplerToBGFX(matMode, 1); // v
				flags |= assimpSamplerToBGFX(matMode, 2); // w

				texture->setFlags(flags);
				// ----

				// ----
				texture->setName(basename);
				texture->upload(format);

				_textures.push_back(texture);
			}
		}

		return _textures;
	}

	void AssimpImporter::loadTextures(const aiScene* sc, aiMesh& assimp, rawrbox::AssimpMesh& mesh) {
		if (sc->mNumMaterials <= 0 || assimp.mMaterialIndex > sc->mNumMaterials) return;
		const aiMaterial* pMaterial = sc->mMaterials[assimp.mMaterialIndex];

		aiString matName;

		std::string name = "default";
		if (pMaterial->Get(AI_MATKEY_NAME, matName) == AI_SUCCESS) {
			name = matName.data;
		}

		auto fnd = this->materials.find(name);
		if (fnd == this->materials.end()) {
			auto mat = std::make_shared<rawrbox::AssimpMaterial>(name);

			// WIREFRAME ----
			pMaterial->Get(AI_MATKEY_ENABLE_WIREFRAME, mat->wireframe);
			// ---------

			// CULLING ----
			pMaterial->Get(AI_MATKEY_TWOSIDED, mat->doubleSided);
			/// ----

			// TRANSPARENCY ----
			float alpha = 1.F;
			if (pMaterial->Get(AI_MATKEY_OPACITY, alpha) != AI_SUCCESS) {
				pMaterial->Get(AI_MATKEY_TRANSPARENCYFACTOR, alpha);
			}
			// ---------

			// Texture loading ----
			if ((this->loadFlags & rawrbox::ModelLoadFlags::Debug::PRINT_MATERIALS) > 0) {
				const auto dump = [](const aiMaterial* mat, aiTextureType type) {
					const unsigned count = mat->GetTextureCount(type);

					if (count > 0) {
						aiString matPath;
						if (mat->GetTexture(type, 0, &matPath, nullptr, nullptr, nullptr, nullptr, nullptr) == AI_SUCCESS) {
							fmt::print("[RawrBox-Assimp] Found {} texture(s) '{}' of type '{}'\n", count, matPath.C_Str(), magic_enum::enum_name(type));
						}
					}
				};

				constexpr auto assimp_mat = magic_enum::enum_entries<aiTextureType>();

				fmt::print("==== DUMP FOR MATERIAL {}\n", pMaterial->GetName().C_Str());
				for (auto& m : assimp_mat)
					dump(pMaterial, m.first);
				fmt::print("==== ====================\n");
			}

			// TEXTURE DIFFUSE
			auto diffuse = this->importTexture(pMaterial, aiTextureType_DIFFUSE);
			if (!diffuse.empty()) {
				mat->diffuse = diffuse[0]; // Only support one for the moment
			}

			aiColor3D flatColor;
			if (pMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, flatColor) == AI_SUCCESS) {
				mat->diffuseColor = rawrbox::Color(flatColor.r, flatColor.g, flatColor.b, alpha);
			} else if (pMaterial->Get(AI_MATKEY_BASE_COLOR, flatColor) == AI_SUCCESS) {
				mat->diffuseColor = rawrbox::Color(flatColor.r, flatColor.g, flatColor.b, alpha);
			}
			// ----------------------

			// TEXTURE EMISSION
			auto emission = this->importTexture(pMaterial, aiTextureType_EMISSION_COLOR);
			if (!emission.empty()) {
				mat->emissive = emission[0]; // Only support one for the moment
			} else {
				auto emission = this->importTexture(pMaterial, aiTextureType_EMISSIVE);
				if (!emission.empty()) mat->emissive = emission[0]; // Only support one for the moment
			}

			pMaterial->Get(AI_MATKEY_EMISSIVE_INTENSITY, mat->intensity);

			aiColor3D emissionColor;
			if (pMaterial->Get(AI_MATKEY_COLOR_EMISSIVE, emissionColor) == AI_SUCCESS) {
				mat->emissionColor = rawrbox::Color{emissionColor.r, emissionColor.g, emissionColor.b, alpha};
			}
			// ----------------------

			// TEXTURE SPECULAR
			auto specular = this->importTexture(pMaterial, aiTextureType_SPECULAR);
			if (!specular.empty()) {
				mat->specular = specular[0]; // Only support one for the moment
			}

			pMaterial->Get(AI_MATKEY_SHININESS, mat->shininess);

			aiColor3D specularColor;
			if (pMaterial->Get(AI_MATKEY_COLOR_SPECULAR, specularColor) == AI_SUCCESS) {
				mat->specularColor = rawrbox::Color{specularColor.r, specularColor.g, specularColor.b, alpha};
			}
			// ----------------------

			// TEXTURE OPACITY
			auto opacity = this->importTexture(pMaterial, aiTextureType_OPACITY); // bgfx::TextureFormat::D24S8
			if (!opacity.empty()) {
				mat->opacity = opacity[0]; // Only support one for the moment
			}
			// ----------------------

			if (alpha != 1.F || mat->opacity != nullptr) {
				mat->blending = BGFX_STATE_BLEND_ALPHA;
			}

			this->materials[matName.data] = std::move(mat);
		}

		mesh.material = this->materials[matName.data];
	}
	/// -------

	// SKELETON LOADING -----
	void AssimpImporter::loadSkeleton(const aiScene* sc, rawrbox::AssimpMesh& mesh, const aiMesh& aiMesh) {
		if (!aiMesh.HasBones()) return;

		for (size_t i = 0; i < aiMesh.mNumBones; i++) {
			aiBone* bone = aiMesh.mBones[i];
			if (bone->mArmature == nullptr) continue;

			// Armature parsing
			std::string name = std::string(bone->mArmature->mName.data);
			std::string boneName = std::string(bone->mName.data);
			std::string boneKey = fmt::format("{}-{}", name, boneName);

			// Armature does not exist, this should be the root bone
			if (this->skeletons.find(name) == this->skeletons.end()) {
				auto armature = std::make_unique<rawrbox::Skeleton>(name);

				// Create root bone ----
				armature->rootBone = std::make_unique<rawrbox::Bone>("ROOT-BONE");
				armature->rootBone->owner = armature.get();

				armature->invTransformationMtx.transpose(&sc->mRootNode->mTransformation.a1);
				armature->invTransformationMtx.inverse();

				armature->rootBone->transformationMtx.transpose(&bone->mArmature->mTransformation.a1);
				//  ---------------------

				this->generateSkeleton(*armature, bone->mArmature, *armature->rootBone);

				// DEBUG ----
				if ((this->loadFlags & rawrbox::ModelLoadFlags::Debug::PRINT_BONE_STRUCTURE) > 0) {
					std::function<void(std::unique_ptr<Bone>&, int)> printBone;
					printBone = [&printBone](std::unique_ptr<Bone>& bn, int deep) -> void {
						for (auto& c : bn->children) {
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

				this->skeletons[name] = std::move(armature);
			}
			// ---------------

			// Set armature ---
			mesh.skeleton = this->skeletons[name].get();
			// ----

			// Apply the weights -----
			auto fnd = mesh.skeleton->boneMap.find(boneKey);
			if (fnd == mesh.skeleton->boneMap.end()) throw std::runtime_error(fmt::format("[RawrBox-Assimp] Failed to map bone {}", boneKey));

			fnd->second->offsetMtx.transpose(&bone->mOffsetMatrix.a1);
			fnd->second->offsetMtx.mul(mesh.offsetMatrix);

			// Calculate object weights
			for (size_t j = 0; j < bone->mNumWeights; j++) {
				auto& weightobj = bone->mWeights[j];
				mesh.vertices[weightobj.mVertexId].addBoneData(fnd->second->boneId, weightobj.mWeight); // Global vertices
			}
			// ------
		}
	}

	void AssimpImporter::generateSkeleton(rawrbox::Skeleton& skeleton, const aiNode* pNode, rawrbox::Bone& parent) {
		for (size_t i = 0; i < pNode->mNumChildren; i++) {
			auto child = pNode->mChildren[i];

			std::string boneName = child->mName.data;
			std::string boneKey = fmt::format("{}-{}", skeleton.name, boneName);

			auto bone = std::make_unique<rawrbox::Bone>(boneKey);
			bone->parent = &parent;
			bone->owner = &skeleton;
			bone->boneId = static_cast<uint8_t>(skeleton.boneMap.size());
			bone->transformationMtx.transpose(&child->mTransformation.a1);

			skeleton.boneMap[boneKey] = bone.get();

			this->generateSkeleton(skeleton, child, *bone);
			parent.children.push_back(std::move(bone));
		}
	}

	bx::Easing::Enum AssimpImporter::assimpBehavior(aiAnimBehaviour b) {
		switch (b) {
			case aiAnimBehaviour_CONSTANT:
				return bx::Easing::Step;
			default:
				return bx::Easing::Linear;
		}
	}

	aiNode* AssimpImporter::findRootSkeleton(const aiScene* sc, const std::string& meshName) {
		// Attempt to find armature
		auto aiNode = sc->mRootNode->FindNode(meshName.c_str());
		if (aiNode == nullptr) throw std::runtime_error(fmt::format("[RawrBox-Model] Failed to find node '{}' on scene root", meshName));

		auto fnd = this->skeletons.find(aiNode->mName.data);
		if (fnd != this->skeletons.end()) return aiNode;

		while (true) {
			if (aiNode->mParent == nullptr) return nullptr;

			auto fnd = this->skeletons.find(aiNode->mParent->mName.data);
			if (fnd != this->skeletons.end()) return aiNode->mParent;

			aiNode = aiNode->mParent;
		}
		// --------

		return nullptr;
	}

	void AssimpImporter::markMeshAnimated(const std::string& meshName, const std::string& search) {
		auto m = std::find_if(this->meshes.begin(), this->meshes.end(), [&](rawrbox::AssimpMesh x) { return x.name == search; });
		if (m == this->meshes.end()) return;
		(*m).animated = true;

		this->animatedMeshes[meshName] = &(*m);
	}

	void AssimpImporter::loadAnimations(const aiScene* sc) {
		if (!sc->HasAnimations()) return;

		// Load animations ----
		for (size_t i = 0; i < sc->mNumAnimations; i++) {
			auto& anim = *sc->mAnimations[i];

			std::string animName = anim.mName.data;
			if (animName.empty()) animName = fmt::format("anim_{}", i);

			if ((this->loadFlags & rawrbox::ModelLoadFlags::Debug::PRINT_ANIMATIONS) > 0) {
				fmt::print("[RawrBox-Assimp] Found animation '{}'\n", animName);
			}

			auto spl = rawrbox::StrUtils::split(animName, '|');

			// create an entry in the mapping and start filling it with data
			auto& ourAnim = this->animations[spl.back()];

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

					// Found a bone!
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
	void AssimpImporter::loadLights(const aiScene* sc) {
		if (!sc->HasLights()) return;

		for (size_t n = 0; n < sc->mNumLights; ++n) {
			auto& aiLight = *sc->mLights[n];

			auto lightNode = sc->mRootNode->FindNode(aiLight.mName.data);
			if (lightNode == nullptr) continue;

			rawrbox::AssimpLight light;
			light.name = aiLight.mName.data;

			light.pos = rawrbox::Vector3f(aiLight.mPosition.x, aiLight.mPosition.y, aiLight.mPosition.z);
			light.direction = rawrbox::Vector3f(aiLight.mDirection.x, aiLight.mDirection.y, aiLight.mDirection.z).normalized();

			light.parentID = "";
			if (lightNode->mParent != nullptr) {
				light.parentID = lightNode->mParent->mName.data; // TODO: Assimp doesn't seem to give the correct parent, will need to manually find it
			}

			light.diffuse = rawrbox::Color(aiLight.mColorDiffuse.r, aiLight.mColorDiffuse.g, aiLight.mColorDiffuse.b, 1.F) / 255.F;
			light.specular = rawrbox::Color(aiLight.mColorSpecular.r, aiLight.mColorSpecular.g, aiLight.mColorSpecular.b, 1.F) / 255.F;
			light.ambient = rawrbox::Color(aiLight.mColorAmbient.r, aiLight.mColorAmbient.g, aiLight.mColorAmbient.b, 1.F) / 255.F;

			light.attenuationConstant = aiLight.mAttenuationConstant;
			light.attenuationLinear = aiLight.mAttenuationLinear;
			light.attenuationQuadratic = aiLight.mAttenuationQuadratic;

			light.angleInnerCone = aiLight.mAngleInnerCone;
			light.angleOuterCone = aiLight.mAngleOuterCone;

			light.up = rawrbox::Vector3f(aiLight.mUp.x, aiLight.mUp.y, aiLight.mUp.z);

			switch (aiLight.mType) {
				case aiLightSource_DIRECTIONAL:
					light.type = rawrbox::LightType::LIGHT_DIR;
					break;
				case aiLightSource_SPOT:
					light.type = rawrbox::LightType::LIGHT_SPOT;
					break;
				case aiLightSource_POINT:
					light.type = rawrbox::LightType::LIGHT_POINT;
					break;
				default:
					break;
			}

			this->lights.push_back(light);
		}
	}
	/// -------

	// MESH LOADING -----
	void AssimpImporter::loadSubmeshes(const aiScene* sc, const aiNode* root) {
		for (size_t n = 0; n < root->mNumMeshes; ++n) {
			aiMesh& aiMesh = *sc->mMeshes[root->mMeshes[n]];

			rawrbox::AssimpMesh mesh;
			mesh.name = aiMesh.mName.data;

			// Calculate bbox ---
			auto min = aiMesh.mAABB.mMin;
			auto max = aiMesh.mAABB.mMax;

			mesh.bbox.m_min = {min.x, min.y, min.z};
			mesh.bbox.m_max = {max.x, max.y, max.z};
			mesh.bbox.m_size = mesh.bbox.m_min.abs() + mesh.bbox.m_max.abs();
			// -----

			if ((this->assimpFlags & aiProcess_PreTransformVertices) == 0) {
				mesh.offsetMatrix.transpose(&root->mTransformation.a1); // Append matrix to our vertices, since pre-transform is disabled
			}

			// Textures
			if ((this->loadFlags & rawrbox::ModelLoadFlags::IMPORT_TEXTURES) > 0) {
				this->loadTextures(sc, aiMesh, mesh);
			}

			// Vertices
			for (size_t i = 0; i < aiMesh.mNumVertices; i++) {
				rawrbox::VertexSkinnedLitData v;

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

				if (aiMesh.HasNormals()) {
					auto& normal = aiMesh.mNormals[i];
					v.normal[0] = rawrbox::PackUtils::packNormal(normal.x, normal.y, normal.z);
				}

				if (aiMesh.HasTangentsAndBitangents()) {
					auto& tangents = aiMesh.mTangents[i];
					v.normal[1] = rawrbox::PackUtils::packNormal(tangents.x, tangents.y, tangents.z);

					auto& bitangents = aiMesh.mBitangents[i];
					v.normal[2] = rawrbox::PackUtils::packNormal(bitangents.x, bitangents.y, bitangents.z);
				}

				mesh.vertices.push_back(v);
			}

			// Indices
			for (size_t t = 0; t < aiMesh.mNumFaces; ++t) {
				auto& face = aiMesh.mFaces[t];
				if (face.mNumIndices != 3) continue; // we only do triangles

				for (size_t i = 0; i < face.mNumIndices; i++) {
					mesh.indices.push_back(face.mIndices[i]);
				}
			}

			// Bones
			if ((this->loadFlags & rawrbox::ModelLoadFlags::IMPORT_ANIMATIONS) > 0 && aiMesh.HasBones()) {
				this->loadSkeleton(sc, mesh, aiMesh);
			}
			// -------------------

			this->meshes.push_back(mesh);
		}

		// recursive
		for (size_t n = 0; n < root->mNumChildren; ++n) {
			this->loadSubmeshes(sc, root->mChildren[n]);
		}
	}
	/// -------

	void AssimpImporter::internalLoad(const aiScene* scene, bool attemptedFallback) {
		if (scene == nullptr) {
			auto error = aiGetErrorString(); // Because vscode doesn't print the error bellow
			fmt::print("[RawrBox-Assimp] Failed to load '{}' ──> {}\n  └── Loading fallback model!\n", this->fileName.generic_string(), error);

			if (attemptedFallback) {
				throw std::runtime_error(fmt::format("[RawrBox-Assimp] Failed to load fallback model! \n"));
			} else {

				fmt::print("[RawrBox-Assimp] Failed to load '{}'\n", this->fileName.generic_string());
				scene = aiImportFile("./content/models/error.gltf", this->assimpFlags); // fallback

				if (scene == nullptr) {
					error = aiGetErrorString(); // Because vscode doesn't print the error bellow
					throw std::runtime_error(fmt::format("[RawrBox-Assimp] Content '{}' error : '{}'\n", this->fileName.generic_string(), error));
				}
			}
		}

		// load models
		this->loadSubmeshes(scene, scene->mRootNode);
		if ((this->loadFlags & rawrbox::ModelLoadFlags::IMPORT_LIGHT) > 0) this->loadLights(scene);
		if ((this->loadFlags & rawrbox::ModelLoadFlags::IMPORT_ANIMATIONS) > 0) this->loadAnimations(scene);
		// ----

		aiReleaseImport(scene);
	}

	AssimpImporter::AssimpImporter(uint32_t loadFlags, uint32_t assimpFlags) : loadFlags(loadFlags), assimpFlags(assimpFlags) {
		this->meshes.clear(); // Clear old meshes

		if ((this->loadFlags & rawrbox::ModelLoadFlags::IMPORT_ANIMATIONS) > 0) {
			this->assimpFlags |= aiProcess_PopulateArmatureData | aiProcess_LimitBoneWeights; // Enable armature & limit bones
		} else {
			this->assimpFlags |= aiProcess_PreTransformVertices; // Enable PreTransformVertices for optimization
		}

		if ((this->loadFlags & rawrbox::ModelLoadFlags::IMPORT_TEXTURES) > 0) {
			this->assimpFlags |= aiProcess_RemoveRedundantMaterials; // Enable armature & limit bones
		}
	}

	AssimpImporter::~AssimpImporter() {
		this->animations.clear();
		this->animatedMeshes.clear();
		this->skeletons.clear();
		this->lights.clear();
		this->materials.clear();
		this->meshes.clear();
	}

	// Loading ----
	void AssimpImporter::load(const std::filesystem::path& path, const std::vector<uint8_t>& buffer, uint32_t loadFlags, uint32_t assimpFlags) {
		this->fileName = path;

		auto b = buffer;
		if (!b.empty()) {
			const char* bah = std::bit_cast<const char*>(b.data());

			if (path.extension() == ".gltf") {
				this->load(path, loadFlags, assimpFlags); // GLTF has external dependencies, not sure how to load them using file from memory
			} else {
				this->internalLoad(aiImportFileFromMemory(bah, static_cast<uint32_t>(b.size() * sizeof(char)), this->assimpFlags, nullptr));
			}
		} else {
			// Fallback
			fmt::print("[RawrBox-Assimp] Failed to load '{}'\n", this->fileName.generic_string());
			this->internalLoad(aiImportFile("./content/models/error.gltf", this->assimpFlags), true);
		}
	}

	void AssimpImporter::load(const std::filesystem::path& path, uint32_t loadFlags, uint32_t assimpFlags) {
		this->fileName = path;
		this->internalLoad(aiImportFile(path.generic_string().c_str(), this->assimpFlags));
	}
	// ---
} // namespace rawrbox
