
#include <rawrbox/assimp/importer.hpp>
#include <rawrbox/math/utils/math.hpp>
#include <rawrbox/render/textures/image.hpp>
#include <rawrbox/utils/pack.hpp>
#include <rawrbox/utils/string.hpp>

#include <magic_enum.hpp>

#include <assimp/cimport.h>
#include <fmt/format.h>

namespace rawrbox {
	// TEXTURE LOADING -----
	void AssimpImporter::assimpSamplerToDiligent(Diligent::SamplerDesc& desc, const std::array<aiTextureMapMode, 3>& mode, int axis) {
		switch (mode[axis]) {
			case aiTextureMapMode_Wrap:
				if (axis == 0)
					desc.AddressU = Diligent::TEXTURE_ADDRESS_WRAP;
				else if (axis == 1)
					desc.AddressV = Diligent::TEXTURE_ADDRESS_WRAP;
				else if (axis == 2)
					desc.AddressW = Diligent::TEXTURE_ADDRESS_WRAP;
				break;
			case aiTextureMapMode_Clamp:
				if (axis == 0)
					desc.AddressU = Diligent::TEXTURE_ADDRESS_CLAMP;
				else if (axis == 1)
					desc.AddressV = Diligent::TEXTURE_ADDRESS_CLAMP;
				else if (axis == 2)
					desc.AddressW = Diligent::TEXTURE_ADDRESS_CLAMP;
				break;
			case aiTextureMapMode_Decal:
				if (axis == 0)
					desc.AddressU = Diligent::TEXTURE_ADDRESS_BORDER;
				else if (axis == 1)
					desc.AddressV = Diligent::TEXTURE_ADDRESS_BORDER;
				else if (axis == 2)
					desc.AddressW = Diligent::TEXTURE_ADDRESS_BORDER;
				break;
			case aiTextureMapMode_Mirror:
				if (axis == 0)
					desc.AddressU = Diligent::TEXTURE_ADDRESS_MIRROR;
				else if (axis == 1)
					desc.AddressV = Diligent::TEXTURE_ADDRESS_MIRROR;
				else if (axis == 2)
					desc.AddressW = Diligent::TEXTURE_ADDRESS_MIRROR;
				break;
			default: break; // WRAP
		}
	}

	std::vector<rawrbox::OptionalTexture> AssimpImporter::importTexture(const aiScene* scene, const aiMaterial* mat, aiTextureType type, Diligent::TEXTURE_FORMAT format) {
		std::vector<rawrbox::OptionalTexture> _textures = {};

		int count = mat->GetTextureCount(type);
		if (count <= 0) return _textures;

		for (int i = 0; i < count; i++) {
			aiString matPath;
			std::array<aiTextureMapMode, 3> matMode = {};

			if (mat->GetTexture(type, count - 1, &matPath, nullptr, nullptr, nullptr, nullptr, matMode.data()) == AI_SUCCESS) {
				auto textPath = std::filesystem::path(matPath.data);

				std::string basename = textPath.filename().generic_string();
				std::unique_ptr<rawrbox::TextureImage> texture = nullptr;

				if (basename.starts_with("*")) { // Embedded
					auto aiTex = scene->GetEmbeddedTexture(textPath.generic_string().c_str());
					texture = std::make_unique<rawrbox::TextureImage>(std::bit_cast<uint8_t*>(aiTex->pcData), aiTex->mWidth * std::max(aiTex->mHeight, 1U));
				} else {
					auto parentFolder = std::filesystem::path(this->fileName).parent_path();

					std::string loadPath = fmt::format("{}/{}", parentFolder.generic_string(), textPath.generic_string());
					if (!std::filesystem::exists(loadPath)) {
						// Check relative
						loadPath = fmt::format("{}/{}", parentFolder.generic_string(), basename);
						if (!std::filesystem::exists(loadPath)) {
							this->_logger->warn("Failed to load texture '{}'", matPath.data);
							_textures.emplace_back(nullptr);
							continue;
						}
					}

					texture = std::make_unique<rawrbox::TextureImage>(loadPath);
				}

				// Setup flags ----
				Diligent::SamplerDesc flags{
				    Diligent::FILTER_TYPE_POINT, Diligent::FILTER_TYPE_POINT, Diligent::FILTER_TYPE_POINT};

				this->assimpSamplerToDiligent(flags, matMode, 0); // u
				this->assimpSamplerToDiligent(flags, matMode, 1); // v
				this->assimpSamplerToDiligent(flags, matMode, 2); // w

				texture->setSampler(flags);
				// ----

				// ----
				texture->setName(basename);
				texture->upload(format);

				_textures.emplace_back(std::move(texture));
			}
		}

		return _textures;
	}

	void AssimpImporter::loadTextures(const aiScene* sc, const aiMesh& assimp, rawrbox::AssimpMesh& mesh) {
		if (sc->mNumMaterials <= 0 || assimp.mMaterialIndex > sc->mNumMaterials) return;
		const aiMaterial* pMaterial = sc->mMaterials[assimp.mMaterialIndex];

		aiString matName;

		std::string name = "default";
		if (pMaterial->Get(AI_MATKEY_NAME, matName) == AI_SUCCESS) {
			name = matName.data;
		}

		auto fnd = this->materials.find(name);
		if (fnd == this->materials.end()) {
			auto mat = std::make_unique<rawrbox::AssimpMaterial>(name);

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
				const auto dump = [this](const aiMaterial* mat, aiTextureType type) {
					const unsigned count = mat->GetTextureCount(type);

					if (count > 0) {
						aiString matPath;
						ai_real matBlend = 0;

						for (size_t c = 0; c < count; c++) {
							if (mat->GetTexture(type, c, &matPath, nullptr, nullptr, &matBlend, nullptr, nullptr) == AI_SUCCESS) {
								fmt::print("\t [{}] '{}' -> '{}'\n", c, matPath.C_Str(), magic_enum::enum_name(type));
							}
						}
					}
				};

				constexpr auto assimp_mat = magic_enum::enum_entries<aiTextureType>();

				this->_logger->info("Material: {}", fmt::format(fmt::fg(fmt::color::cyan), pMaterial->GetName().C_Str()));
				for (auto& m : assimp_mat)
					dump(pMaterial, m.first);
			}

			// TEXTURE DIFFUSE
			auto diffuse = this->importTexture(sc, pMaterial, aiTextureType_DIFFUSE);
			if (!diffuse.empty()) {
				mat->diffuse = std::move(diffuse[0]); // Only support one for the moment
			}

			aiColor3D flatColor;
			if (pMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, flatColor) == AI_SUCCESS) {
				mat->baseColor = rawrbox::Colorf(flatColor.r, flatColor.g, flatColor.b, alpha);
			} else if (pMaterial->Get(AI_MATKEY_BASE_COLOR, flatColor) == AI_SUCCESS) {
				mat->baseColor = rawrbox::Colorf(flatColor.r, flatColor.g, flatColor.b, alpha);
			}
			// ----------------------

			// TEXTURE NORMAL
			auto normal = this->importTexture(sc, pMaterial, aiTextureType_NORMALS);
			if (!normal.empty()) {
				mat->normal = std::move(normal[0].value()); // Only support one for the moment
			}
			// ----------------------

			// TEXTURE ROUGHT / METAL
			auto metalR = this->importTexture(sc, pMaterial, aiTextureType_METALNESS); // Merge?
			if (!metalR.empty()) {
				mat->metalRough = std::move(metalR[0].value()); // Only support one for the moment
			}

			pMaterial->Get(AI_MATKEY_ROUGHNESS_FACTOR, mat->roughnessFactor);
			pMaterial->Get(AI_MATKEY_METALLIC_FACTOR, mat->metalnessFactor);
			// ----------------------

			// TEXTURE EMISSION
			auto emission = this->importTexture(sc, pMaterial, aiTextureType_EMISSION_COLOR);
			if (!emission.empty()) {
				mat->emissive = std::move(emission[0].value()); // Only support one for the moment
			} else {
				emission = this->importTexture(sc, pMaterial, aiTextureType_EMISSIVE);
				if (!emission.empty()) mat->emissive = std::move(emission[0].value()); // Only support one for the moment
			}

			pMaterial->Get(AI_MATKEY_EMISSIVE_INTENSITY, mat->emissionFactor);

			aiColor3D emissionColor;
			if (pMaterial->Get(AI_MATKEY_COLOR_EMISSIVE, emissionColor) == AI_SUCCESS) {
				mat->emissionColor = rawrbox::Color{emissionColor.r, emissionColor.g, emissionColor.b, alpha};
			}
			// ----------------------

			// TEXTURE SPECULAR
			auto specular = this->importTexture(sc, pMaterial, aiTextureType_SPECULAR);
			if (!specular.empty()) {
				mat->specular = std::move(specular[0].value()); // Only support one for the moment
			}

			aiColor3D specularColor;
			if (pMaterial->Get(AI_MATKEY_COLOR_SPECULAR, specularColor) == AI_SUCCESS) {
				mat->specularColor = rawrbox::Color{specularColor.r, specularColor.g, specularColor.b, alpha};
			}
			// ----------------------

			mat->alpha = alpha != 1.F;
			this->materials[matName.data] = std::move(mat);
		}

		mesh.material = this->materials[matName.data].get();
	}
	/// -------

	// SKELETON LOADING -----
	void AssimpImporter::loadSkeleton(const aiScene* sc, rawrbox::AssimpMesh& mesh, const aiMesh& aiMesh) {
		if (!aiMesh.HasBones()) return;

		std::unordered_map<size_t, int> bone_index = {};
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
					printBone = [&printBone, this](std::unique_ptr<Bone>& bn, int deep) -> void {
						for (auto& c : bn->children) {
							std::string d = "";
							for (int i = 0; i < deep; i++)
								d += "\t";

							fmt::print("\t{}[{}] {}\n", d, c->boneId, c->name);
							printBone(c, ++deep);
						}
					};

					this->_logger->info("'{}' BONES", fmt::format(fmt::fg(fmt::color::cyan), this->fileName.generic_string()));
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
			if (fnd == mesh.skeleton->boneMap.end()) throw this->_logger->error("Failed to map bone {}", boneKey);

			fnd->second->offsetMtx.transpose(&bone->mOffsetMatrix.a1);
			fnd->second->offsetMtx *= mesh.matrix;

			// Calculate object weights
			for (size_t j = 0; j < bone->mNumWeights; j++) {
				auto& weightobj = bone->mWeights[j];
				auto& v = mesh.vertices[weightobj.mVertexId];
				auto& indx = bone_index[weightobj.mVertexId];

				uint32_t boneId = fnd->second->boneId;
				float boneWeight = weightobj.mWeight;

				if (indx < rawrbox::MAX_BONES_PER_VERTEX) {
					v.bone_indices[indx] = boneId;
					v.bone_weights[indx] = boneWeight;

					indx++;
				} else {
					// find the bone with the smallest weight
					int minIndex = 0;
					float minWeight = v.bone_weights[0];

					for (int o = 1; o < rawrbox::MAX_BONES_PER_VERTEX; o++) {
						if (v.bone_weights[o] < minWeight) {
							minIndex = o;
							minWeight = v.bone_weights[o];
						}
					}

					// replace with new bone if the new bone has greater weight
					if (boneWeight > minWeight) {
						this->_logger->warn("Model bone past max limit of '{}', replacing bone '{}' with bone '{}'", rawrbox::MAX_BONES_PER_VERTEX, v.bone_indices[minIndex], boneId);

						v.bone_indices[minIndex] = boneId;
						v.bone_weights[minIndex] = boneWeight;
					}
				}
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

	rawrbox::Easing AssimpImporter::assimpBehavior(aiAnimBehaviour b) {
		switch (b) {
			case aiAnimBehaviour_CONSTANT:
				return rawrbox::Easing::STEP;
			default:
				return rawrbox::Easing::LINEAR;
		}
	}

	aiNode* AssimpImporter::findRootSkeleton(const aiScene* sc, const std::string& meshName) {
		// Attempt to find armature
		auto aiNode = sc->mRootNode->FindNode(meshName.c_str());
		if (aiNode == nullptr) throw this->_logger->error("Failed to find node '{}' on scene root", meshName);

		auto fnd = this->skeletons.find(aiNode->mName.data);
		if (fnd != this->skeletons.end()) return aiNode;

		while (true) {
			if (aiNode->mParent == nullptr) return nullptr;

			fnd = this->skeletons.find(aiNode->mParent->mName.data);
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
				this->_logger->info("Found animation '{}'", fmt::format(fmt::fg(fmt::color::green_yellow), animName));
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
					if (pNode == nullptr) throw this->_logger->error("Failed to find animated mesh '{}'", meshName);

					for (size_t p = 0; p < pNode->mNumChildren; p++) {
						this->markMeshAnimated(meshName, pNode->mChildren[p]->mName.data);
					}

					for (size_t n = 0; n < pNode->mNumMeshes; n++) {
						this->markMeshAnimated(meshName, sc->mMeshes[pNode->mMeshes[n]]->mName.data);
					}

					ourChannel.nodeName = meshName;
					// ----------------------
				}
				// -------------------------

				ourChannel.stateStart = this->assimpBehavior(aChannel->mPreState);
				ourChannel.stateEnd = this->assimpBehavior(aChannel->mPostState);

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

	void AssimpImporter::loadBlendShapes(const aiMesh& assimp, rawrbox::AssimpMesh& mesh) {
		if (assimp.mNumAnimMeshes == 0) return;

		for (size_t n = 0; n < assimp.mNumAnimMeshes; ++n) {
			auto& aiMesh = *assimp.mAnimMeshes[n];
			rawrbox::AssimpBlendShapes shape;

			shape.name = aiMesh.mName.data;
			if (shape.name.empty()) {
				shape.name = std::to_string(++this->_aiAnimMeshIndex); // Fix name with a global model index
			}

			shape.weight = aiMesh.mWeight;
			shape.mesh_index = this->meshes.size();

			if (aiMesh.mNumVertices != mesh.vertices.size()) {
				this->_logger->warn("Failed to load blend shape '{}'! Vertex sizes do not match!", shape.name);
				return;
			}

			for (size_t i = 0; i < aiMesh.mNumVertices; i++) {
				if (aiMesh.HasPositions()) {
					auto& vert = aiMesh.mVertices[i];
					shape.pos.emplace_back(vert.x, vert.y, vert.z);
				}

				if (aiMesh.HasNormals()) {
					auto& norm = aiMesh.mNormals[i];
					shape.norms.emplace_back(norm.x, norm.y, norm.z);
				}
			}

			this->blendShapes[fmt::format("{}-{}", mesh.name, shape.name)] = shape;
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

			if (aiLight.mColorDiffuse != aiLight.mColorSpecular) throw this->_logger->error("Light diffuse and specular do not match");
			float intensity = std::max(std::max(aiLight.mColorDiffuse.r, aiLight.mColorDiffuse.g), aiLight.mColorDiffuse.b);

			light.diffuse = rawrbox::Colorf(aiLight.mColorDiffuse.r, aiLight.mColorDiffuse.g, aiLight.mColorDiffuse.b, 1.F) / intensity;
			light.specular = rawrbox::Colorf(aiLight.mColorSpecular.r, aiLight.mColorSpecular.g, aiLight.mColorSpecular.b, 1.F) / intensity;
			light.ambient = rawrbox::Colorf(aiLight.mColorAmbient.r, aiLight.mColorAmbient.g, aiLight.mColorAmbient.b, 1.F) / intensity;

			light.intensity = intensity * 2;
			light.attenuationConstant = aiLight.mAttenuationConstant;
			light.attenuationLinear = aiLight.mAttenuationLinear;
			light.attenuationQuadratic = aiLight.mAttenuationQuadratic;

			light.angleInnerCone = rawrbox::MathUtils::toDeg(aiLight.mAngleInnerCone);
			light.angleOuterCone = rawrbox::MathUtils::toDeg(aiLight.mAngleOuterCone);

			light.up = rawrbox::Vector3f(aiLight.mUp.x, aiLight.mUp.y, aiLight.mUp.z);

			switch (aiLight.mType) {
				case aiLightSource_DIRECTIONAL:
					light.type = rawrbox::LightType::DIR;
					break;
				case aiLightSource_SPOT:
					light.type = rawrbox::LightType::SPOT;
					break;
				case aiLightSource_POINT:
					light.type = rawrbox::LightType::POINT;
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

			rawrbox::AssimpMesh mesh(aiMesh.mName.data);

			// Calculate bbox ---
			auto min = aiMesh.mAABB.mMin;
			auto max = aiMesh.mAABB.mMax;

			mesh.bbox.m_min = {min.x, min.y, min.z};
			mesh.bbox.m_max = {max.x, max.y, max.z};
			mesh.bbox.m_size = mesh.bbox.m_min.abs() + mesh.bbox.m_max.abs();
			// -----

			if ((this->assimpFlags & aiProcess_PreTransformVertices) == 0) {
				mesh.matrix.transpose(&root->mTransformation.a1); // Append matrix to our vertices, since pre-transform is disabled
			}

			// Textures
			if ((this->loadFlags & rawrbox::ModelLoadFlags::IMPORT_TEXTURES) > 0) {
				this->loadTextures(sc, aiMesh, mesh);
			}
			// -------------------

			// Vertices
			for (size_t i = 0; i < aiMesh.mNumVertices; i++) {
				rawrbox::VertexNormBoneData v;

				if (aiMesh.HasPositions()) {
					auto& vert = aiMesh.mVertices[i];
					v.position = {vert.x, vert.y, vert.z};
				}

				if (aiMesh.HasTextureCoords(0)) {
					auto& uv = aiMesh.mTextureCoords[0][i];
					v.uv = {uv.x, uv.y, 0, 0};
				}

				if (aiMesh.HasVertexColors(0)) {
					auto& col = aiMesh.mColors[0][i];
					v.color = rawrbox::Colorf::pack(col.r, col.g, col.b, col.a);
				} else {
					v.color = 0xFFFFFFFF;
				}

				if (aiMesh.HasNormals()) {
					auto& normal = aiMesh.mNormals[i];
					v.normal = {normal.x, normal.y, normal.z};
				}

				if (aiMesh.HasTangentsAndBitangents()) {
					auto& tangents = aiMesh.mTangents[i];
					v.tangent = {tangents.x, tangents.y, tangents.z};
				}

				mesh.vertices.push_back(v);
			}
			// -------------------

			// Indices
			for (size_t t = 0; t < aiMesh.mNumFaces; ++t) {
				auto& face = aiMesh.mFaces[t];
				if (face.mNumIndices != 3) continue; // we only do triangles

				for (size_t i = 0; i < face.mNumIndices; i++) {
					mesh.indices.push_back(static_cast<uint16_t>(face.mIndices[i]));
				}
			}
			// -------------------

			// Bones
			if ((this->loadFlags & rawrbox::ModelLoadFlags::IMPORT_ANIMATIONS) > 0 && aiMesh.HasBones()) {
				this->loadSkeleton(sc, mesh, aiMesh);
			}
			// -------------------

			// Blendshapes
			if ((this->loadFlags & rawrbox::ModelLoadFlags::IMPORT_BLEND_SHAPES) > 0) {
				this->loadBlendShapes(aiMesh, mesh);
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
			this->_logger->warn("Failed to load '{}' ──> {}\n  └── Loading fallback model!", this->fileName.generic_string(), aiGetErrorString());

			if (attemptedFallback) {
				throw this->_logger->error("Failed to load fallback model!");
			} else {
				scene = aiImportFile("./assets/models/error.gltf", this->assimpFlags); // fallback

				if (scene == nullptr) {
					throw this->_logger->error("Failed to load fallback '{}' ──> '{}'", this->fileName.generic_string(), aiGetErrorString());
				}
			}
		}

		// Parse metadata
		if (onMetadata != nullptr) onMetadata(scene->mMetaData); // Allow metadata to be parsed outside, used on vrm for example
		if ((this->loadFlags & rawrbox::ModelLoadFlags::Debug::PRINT_METADATA) > 0) {
			this->_logger->info("'{}' METADATA", fmt::format(fmt::fg(fmt::color::cyan), this->fileName.generic_string()));

			std::function<void(aiMetadata*)> printMetaData;
			printMetaData = [&printMetaData, this](aiMetadata* meta) -> void {
				for (uint8_t i = 0; i < meta->mNumProperties; i++) {
					auto data = meta->mValues[i];
					std::string str = "";

					switch (data.mType) {
						case AI_AISTRING:
							str = fmt::format("{}", static_cast<aiString*>(data.mData)->data);
							break;
						case AI_BOOL:
							str = fmt::format("{}", std::to_string(*static_cast<bool*>(data.mData)));
							break;
						case AI_INT32:
							str = fmt::format("{}", std::to_string(*static_cast<int32_t*>(data.mData)));
							break;
						case AI_UINT64:
							str = fmt::format("{}", std::to_string(*static_cast<uint64_t*>(data.mData)));
							break;
						case AI_FLOAT:
							str = fmt::format("{}", std::to_string(*static_cast<float*>(data.mData)));
							break;
						case AI_DOUBLE:
							str = fmt::format("{}", std::to_string(*static_cast<double*>(data.mData)));
							break;
						case AI_AIVECTOR3D:
							{
								auto vec = static_cast<aiVector3D*>(data.mData);
								str = fmt::format("{},{},{}", vec->x, vec->y, vec->z);
							}
							break;
						case AI_INT64:
							str = fmt::format("{}", std::to_string(*static_cast<int64_t*>(data.mData)));
							break;

						case AI_UINT32:
							str = fmt::format("{}", std::to_string(*static_cast<uint32_t*>(data.mData)));
							break;
						case AI_AIMETADATA:
							printMetaData(static_cast<aiMetadata*>(data.mData));
							break;
						default:
						case AI_META_MAX:
						case FORCE_32BIT: break;
					}

					fmt::print("\t{}: {}\n", meta->mKeys[i].C_Str(), str);
				}
			};

			printMetaData(scene->mMetaData);
		}
		// ------------

		// Reset index for aiAnimMeshes with no names
		this->_aiAnimMeshIndex = 0;
		// ------------

		// load models
		this->loadSubmeshes(scene, scene->mRootNode);
		if ((this->loadFlags & rawrbox::ModelLoadFlags::IMPORT_LIGHT) > 0) this->loadLights(scene);
		if ((this->loadFlags & rawrbox::ModelLoadFlags::IMPORT_ANIMATIONS) > 0) this->loadAnimations(scene);
		// ----

		// Parse metadata
		if ((this->loadFlags & rawrbox::ModelLoadFlags::Debug::PRINT_BLENDSHAPES) > 0) {
			this->_logger->info("'{}' BLEND SHAPES", fmt::format(fmt::fg(fmt::color::cyan), this->fileName.generic_string()));

			std::string old = "";
			for (auto& s : this->blendShapes) {
				auto split = rawrbox::StrUtils::split(s.first, '-');
				auto shapeId = split[split.size() - 1];
				auto shapeName = rawrbox::StrUtils::replace(s.first, fmt::format("-{}", shapeId), "");

				if (old.empty() || old != shapeName) {
					old = shapeName;
					fmt::print("\t{} --->\n", shapeName);
				}

				fmt::print("\t\t{}\n", shapeId);
			}
		}
		// ----

		aiReleaseImport(scene);
	}

	AssimpImporter::AssimpImporter(uint32_t loadFlags_, uint32_t assimpFlags_) : loadFlags(loadFlags_), assimpFlags(assimpFlags_) {
		this->meshes.clear(); // Clear old meshes

		bool optimize = (this->loadFlags & rawrbox::ModelLoadFlags::Optimization::DISABLE) == 0;
		if (optimize) {
			this->assimpFlags |= aiProcessPreset_TargetRealtime_Fast | aiProcess_GlobalScale | aiProcess_TransformUVCoords;
		}

		if ((this->loadFlags & rawrbox::ModelLoadFlags::IMPORT_ANIMATIONS) > 0) {
			this->assimpFlags |= aiProcess_PopulateArmatureData | aiProcess_LimitBoneWeights; // Enable armature & limit bones
		} else if (optimize && (this->loadFlags & rawrbox::ModelLoadFlags::IMPORT_BLEND_SHAPES) == 0) {
			this->assimpFlags |= aiProcess_PreTransformVertices; // Enable PreTransformVertices for optimization
		}
	}

	// Loading ----
	void AssimpImporter::load(const std::filesystem::path& path, const std::vector<uint8_t>& buffer, const std::string& hint) {
		this->fileName = path;

		auto b = buffer;
		if (!b.empty()) {
			const char* bah = std::bit_cast<const char*>(b.data());

			if (path.extension() == ".gltf") {
				this->load(path); // GLTF has external dependencies, not sure how to load them using file from memory
			} else {
				this->internalLoad(aiImportFileFromMemory(bah, static_cast<uint32_t>(b.size() * sizeof(char)), this->assimpFlags, hint.c_str()));
			}
		} else {
			this->load(path);
		}
	}

	void AssimpImporter::load(const std::filesystem::path& path) {
		this->fileName = path;
		this->internalLoad(aiImportFile(path.generic_string().c_str(), this->assimpFlags));
	}
	// ---
} // namespace rawrbox
