
#include <rawrbox/math/vector3.hpp>
#include <rawrbox/render/model/assimp/model_imported.h>
#include <rawrbox/render/model/light/directional.hpp>
#include <rawrbox/render/model/light/manager.h>
#include <rawrbox/render/model/light/point.hpp>
#include <rawrbox/render/model/light/spot.hpp>
#include <rawrbox/render/texture/image.h>
#include <rawrbox/utils/pack.hpp>

#include <assimp/material.h>
#include <assimp/pbrmaterial.h>
#include <bx/math.h>
#include <fmt/format.h>
#include <stdint.h>

#include <filesystem>
#include <stdexcept>

// NOLINTBEGIN(cppcoreguidelines-pro-bounds-pointer-arithmetic)

namespace rawrBox {
	ModelImported::~ModelImported() {
		Model::~Model();
		this->_textures.clear();
	}

	// LOADING -----
	void ModelImported::load(const std::string& path, uint32_t loadFlags, uint32_t assimpFlags) {
		const aiScene* scene = aiImportFile(path.c_str(), assimpFlags);

		if (scene == nullptr) {
			auto error = aiGetErrorString(); // Because vscode doesn't print the error bellow
			throw std::runtime_error(fmt::format("[Resources] Content 'model' error: {}: '{}'\n", path, error));
		}

		this->_meshes.clear(); // Clear old meshes

		this->_fileName = path;
		this->_loadFlags = loadFlags;

		// load models
		this->loadSubmeshes(scene, scene->mRootNode);
		if ((this->_loadFlags & rawrBox::ModelLoadFlags::IMPORT_LIGHT) > 0) this->loadLights(scene);
		// ----

		aiReleaseImport(scene);
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
			flags |= assimpSamplerToBGFX(mode, 0);
			flags |= assimpSamplerToBGFX(mode, 1);
			flags |= assimpSamplerToBGFX(mode, 2);

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
			if (pMaterial->GetTexture(aiTextureType_EMISSIVE, 0, &matpath, nullptr, nullptr, nullptr, nullptr, nullptr) == AI_SUCCESS) {
				auto ptr = this->importTexture(matpath.data);
				if (ptr == nullptr) throw std::runtime_error(fmt::format("[RawrBox-Assimp] Failed to load specular texture '{}'", matpath.data));

				mesh->setSpecularTexture(ptr);
			}
		}*/
	}

	void ModelImported::loadSubmeshes(const aiScene* sc, const aiNode* nd) {
		for (size_t n = 0; n < nd->mNumMeshes; ++n) {
			auto& aiMesh = *sc->mMeshes[nd->mMeshes[n]];
			auto mesh = std::make_shared<rawrBox::Mesh>();

			mesh->setName(aiMesh.mName.data);

			mesh->baseVertex = static_cast<uint16_t>(mesh->vertices.size());
			mesh->baseIndex = static_cast<uint16_t>(mesh->indices.size());

			// Textures
			if ((this->_loadFlags & rawrBox::ModelLoadFlags::IMPORT_TEXTURES) > 0) {
				this->loadTextures(sc, aiMesh, mesh);
			}

			// Vertices
			for (size_t i = 0; i < aiMesh.mNumVertices; i++) {
				rawrBox::MeshVertexData v;

				if (aiMesh.HasPositions()) {
					auto& vert = aiMesh.mVertices[i];

					v.x = vert.x;
					v.y = vert.y;
					v.z = vert.z;
				}

				if (aiMesh.HasTextureCoords(0)) {
					auto& uv = aiMesh.mTextureCoords[0][i];

					v.u = uv.x;
					v.v = uv.y;
				}

				if (aiMesh.HasVertexColors(0)) {
					auto& col = aiMesh.mColors[0][i];
					v.abgr = rawrBox::Color::pack({col.r, col.g, col.b, col.a});
				}

				if (aiMesh.HasNormals()) {
					auto& normal = aiMesh.mNormals[i];
					v.normal = rawrBox::PackUtils::packNormal(normal.x, normal.y, normal.z);
				}

				if (aiMesh.HasTangentsAndBitangents()) {
					auto& tangents = aiMesh.mTangents[i];
					v.tangent = rawrBox::PackUtils::packNormal(tangents.x, tangents.y, tangents.z);
				}

				mesh->vertices.push_back(v);
			}

			// Indices
			for (size_t t = 0; t < aiMesh.mNumFaces; ++t) {
				auto& face = aiMesh.mFaces[t];
				for (size_t i = 0; i < face.mNumIndices; i++) {
					mesh->indices.push_back(static_cast<uint16_t>(mesh->vertices.size()) - static_cast<uint16_t>(face.mIndices[i]));
				}
			}

			mesh->totalVertex = static_cast<uint16_t>(mesh->vertices.size());
			mesh->totalIndex = static_cast<uint16_t>(mesh->indices.size());

			this->addMesh(mesh);
		}

		// recursive
		for (size_t n = 0; n < nd->mNumChildren; ++n) {
			this->loadSubmeshes(sc, nd->mChildren[n]);
		}
	}

	void ModelImported::loadLights(const aiScene* sc) {
		if (!sc->HasLights()) return;

		for (size_t n = 0; n < sc->mNumLights; ++n) {
			auto& aiLight = *sc->mLights[n];

			auto lightNode = sc->mRootNode->FindNode(aiLight.mName.data);
			if (lightNode == nullptr) continue;

			rawrBox::Vector3f pos = {aiLight.mPosition.x, aiLight.mPosition.y, aiLight.mPosition.z};
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
