
#include <rawrbox/render/model/assimp/model_imported.h>
#include <rawrbox/render/model/light/manager.h>
#include <rawrbox/render/model/light/point.hpp>
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
		if (scene == nullptr) throw std::runtime_error(fmt::format("[Resources] Content 'model' error: {}: '{}'\n", path, aiGetErrorString()));

		this->_meshes.clear(); // Clear old meshes
		this->_fileName = path;
		this->_cull = BGFX_STATE_CULL_CCW;

		// load models
		this->loadSubmeshes(scene, scene->mRootNode);
		if ((loadFlags & rawrBox::ModelLoadFlags::IMPORT_LIGHT) > 0) this->loadLights(scene);
		// ----

		aiReleaseImport(scene);
	}

	void ModelImported::loadTexture(const aiScene* sc, aiMesh& assimp, std::shared_ptr<rawrBox::ModelMesh>& mesh) {
		if (sc->mNumMaterials <= 0 || assimp.mMaterialIndex > sc->mNumMaterials) return;

		const aiMaterial* pMaterial = sc->mMaterials[assimp.mMaterialIndex];
		if (pMaterial->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
			aiString matpath;

			// TEXTURE
			if (pMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &matpath, nullptr, nullptr, nullptr, nullptr, nullptr) == AI_SUCCESS) {
				auto textPath = std::filesystem::path(matpath.data);
				auto base = std::filesystem::path(this->_fileName);

				std::string finalPath = fmt::format("{}/{}", base.parent_path().generic_string(), textPath.generic_string());

				auto fnd = this->_textures.find(finalPath);
				if (fnd == this->_textures.end()) {
					auto texture = std::make_shared<rawrBox::TextureImage>(finalPath);
					texture->upload(bgfx::TextureFormat::Count);

					mesh->setTexture(texture);
					this->_textures[finalPath] = std::move(texture);
				} else {
					mesh->setTexture(fnd->second);
				}
			}
		}
	}

	void ModelImported::loadSubmeshes(const aiScene* sc, const aiNode* nd) {
		for (size_t n = 0; n < nd->mNumMeshes; ++n) {
			auto mesh = std::make_shared<rawrBox::ModelMesh>();

			auto& data = mesh->getData();
			auto& vertices = mesh->getVertices();
			auto& indices = mesh->getIndices();

			bx::mtxTranspose(data->offsetMatrix.data(), &nd->mTransformation.a1);

			auto& aiMesh = *sc->mMeshes[nd->mMeshes[n]];
			mesh->setName(aiMesh.mName.data);

			data->baseVertex = static_cast<uint16_t>(data->vertices.size());
			data->baseIndex = static_cast<uint16_t>(data->indices.size());

			// Event
			this->loadTexture(sc, aiMesh, mesh);

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
		for (size_t n = 0; n < sc->mNumLights; ++n) {
			auto& aiLight = *sc->mLights[n];

			auto pos = rawrBox::Vector3f(aiLight.mPosition.x, aiLight.mPosition.y, aiLight.mPosition.z);
			auto color = rawrBox::Colori(aiLight.mColorDiffuse.r, aiLight.mColorDiffuse.g, aiLight.mColorDiffuse.b).cast<float>();

			switch (aiLight.mType) {
			case aiLightSource_SPOT:
			case aiLightSource_POINT:
				rawrBox::LightManager::getInstance().addLight(std::make_shared<rawrBox::LightPoint>(pos, color, aiLight.mAngleInnerCone, aiLight.mAngleOuterCone, 0.7f)); // aiLight.mAttenuationQuadratic
				continue;
			default:
				continue;
			}
		}
	}
	// ----------
} // namespace rawrBox
