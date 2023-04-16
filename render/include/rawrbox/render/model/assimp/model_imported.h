#pragma once
#include <rawrbox/render/model/model.h>
#include <rawrbox/render/model/model_mesh.h>
#include <rawrbox/render/texture/base.h>

#include <assimp/cimport.h>
#include <assimp/mesh.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <string>
#include <unordered_map>

namespace rawrBox {
	class ModelImported : public Model {
		std::string _fileName;
		std::unordered_map<std::string, std::shared_ptr<rawrBox::TextureBase>> _textures;

		void loadTexture(const aiScene* sc, aiMesh& assimp, std::shared_ptr<rawrBox::ModelMesh>& mesh);
		void loadSubmeshes(const aiScene* sc, const aiNode* nd);

	public:
		virtual ~ModelImported();

		// Loading ----
		void load(const std::string& path, uint32_t flags = aiProcessPreset_TargetRealtime_Fast | aiProcess_GenBoundingBoxes | aiProcess_ConvertToLeftHanded);
		// ---
	};
} // namespace rawrBox
