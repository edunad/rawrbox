
#include <rawrbox/render/model/assimp/model_imported.h>
#include <rawrbox/render/model/light/directional.hpp>
#include <rawrbox/render/model/light/manager.h>
#include <rawrbox/render/model/light/point.hpp>
#include <rawrbox/render/model/light/spot.hpp>
#include <rawrbox/render/texture/image.h>
#include <rawrbox/utils/pack.hpp>

#include <bx/math.h>
#include <fmt/format.h>

#include <filesystem>
#include <stdexcept>

namespace rawrBox {
	ModelImported::~ModelImported() {
		Model::~Model();
		this->_textures.clear();
	}

	// LOADING -----
	void ModelImported::load(const std::string& path, uint32_t loadFlags, uint32_t assimpFlags) {
		const aiScene* scene = aiImportFile(path.c_str(), assimpFlags);

		if (scene == nullptr) {
			auto aaaaaa = aiGetErrorString(); // Because vscode doesn't print the error bellow
			throw std::runtime_error(fmt::format("[Resources] Content 'model' error: {}: '{}'\n", path, aaaaaa));
		}

		this->_meshes.clear(); // Clear old meshes

		this->_fileName = path;
		this->_cull = BGFX_STATE_CULL_CCW;
		this->_loadFlags = loadFlags;

		// load models
		this->loadSubmeshes(scene, scene->mRootNode);
		if ((this->_loadFlags & rawrBox::ModelLoadFlags::IMPORT_LIGHT) > 0) this->loadLights(scene);
		// ----

		aiReleaseImport(scene);
	}

	std::shared_ptr<rawrBox::TextureBase> ModelImported::importTexture(const std::string& path) {
		auto textPath = std::filesystem::path(path);
		auto base = std::filesystem::path(this->_fileName);

		std::string finalPath = fmt::format("{}/{}", base.parent_path().generic_string(), textPath.generic_string());

		auto fnd = this->_textures.find(finalPath);
		if (fnd == this->_textures.end()) {
			auto texture = std::make_shared<rawrBox::TextureImage>(finalPath);
			texture->upload(bgfx::TextureFormat::Count);

			this->_textures[finalPath] = std::move(texture);
			return this->_textures[finalPath];
		} else {
			return fnd->second;
		}
	}

	void ModelImported::loadTextures(const aiScene* sc, aiMesh& assimp, std::shared_ptr<rawrBox::Mesh<rawrBox::ModelVertexData>>& mesh) {
		if (sc->mNumMaterials <= 0 || assimp.mMaterialIndex > sc->mNumMaterials) return;

		const aiMaterial* pMaterial = sc->mMaterials[assimp.mMaterialIndex];
		aiString matpath;

		// TEXTURE DIFFUSE
		if (pMaterial->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
			if (pMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &matpath, nullptr, nullptr, nullptr, nullptr, nullptr) == AI_SUCCESS) {
				auto ptr = this->importTexture(matpath.data);
				if (ptr == nullptr) throw std::runtime_error(fmt::format("[RawrBox-Assimp] Failed to load diffuse texture '{}'", matpath.data));

				mesh->setTexture(ptr);
			}
		}

		// TEXTURE SPECULAR
		if (pMaterial->GetTextureCount(aiTextureType_SPECULAR) > 0) {
			if (pMaterial->GetTexture(aiTextureType_SPECULAR, 0, &matpath, nullptr, nullptr, nullptr, nullptr, nullptr) == AI_SUCCESS) {
				auto ptr = this->importTexture(matpath.data);
				if (ptr == nullptr) throw std::runtime_error(fmt::format("[RawrBox-Assimp] Failed to load specular texture '{}'", matpath.data));

				mesh->setSpecularTexture(ptr);
			}
		}
	}

	void ModelImported::loadSubmeshes(const aiScene* sc, const aiNode* nd) {
		for (size_t n = 0; n < nd->mNumMeshes; ++n) {
			auto& aiMesh = *sc->mMeshes[nd->mMeshes[n]];
			auto mesh = std::make_shared<rawrBox::Mesh<rawrBox::ModelVertexData>>();

			auto& data = mesh->getData();
			auto& vertices = mesh->getVertices();
			auto& indices = mesh->getIndices();

			bx::mtxTranspose(data->offsetMatrix.data(), &nd->mTransformation.a1);
			mesh->setName(aiMesh.mName.data);

			data->baseVertex = static_cast<uint16_t>(data->vertices.size());
			data->baseIndex = static_cast<uint16_t>(data->indices.size());

			// Textures
			if ((this->_loadFlags & rawrBox::ModelLoadFlags::IMPORT_TEXTURES) > 0) {
				this->loadTextures(sc, aiMesh, mesh);
			}

			// Vertices
			for (size_t i = 0; i < aiMesh.mNumVertices; i++) {
				auto& vert = aiMesh.mVertices[i];

				rawrBox::ModelVertexData v;
				v.x = vert.x;
				v.y = vert.y;
				v.z = vert.z;

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

				data->vertices.push_back(v);
			}

			// Indices
			for (size_t t = 0; t < aiMesh.mNumFaces; ++t) {
				auto& face = aiMesh.mFaces[t];
				for (size_t i = 0; i < face.mNumIndices; i++) {
					data->indices.push_back(static_cast<uint16_t>(data->vertices.size()) - static_cast<uint16_t>(face.mIndices[i]));
				}
			}

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

			rawrBox::Vector3f pos;
			aiVector3D p;
			aiQuaternion q;

			lightNode->mTransformation.DecomposeNoScaling(q, p);
			pos = {aiLight.mPosition.x + p.x, aiLight.mPosition.y + p.y, aiLight.mPosition.z + p.z};

			auto parent = lightNode->mParent;
			if (parent != nullptr) {
				parent->mTransformation.DecomposeNoScaling(q, p);
				pos += {p.x, p.y, p.z};
			}

			auto diffuse = rawrBox::Colori(static_cast<int>(aiLight.mColorDiffuse.r), static_cast<int>(aiLight.mColorDiffuse.g), static_cast<int>(aiLight.mColorDiffuse.b)).cast<float>();
			auto ambient = rawrBox::Colori(static_cast<int>(aiLight.mColorAmbient.r), static_cast<int>(aiLight.mColorAmbient.g), static_cast<int>(aiLight.mColorAmbient.b)).cast<float>();
			auto specular = rawrBox::Colori(static_cast<int>(aiLight.mColorSpecular.r), static_cast<int>(aiLight.mColorSpecular.g), static_cast<int>(aiLight.mColorSpecular.b)).cast<float>();

			auto direction = rawrBox::Vector3f(aiLight.mDirection.x, aiLight.mDirection.y, aiLight.mDirection.z);

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
