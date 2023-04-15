#pragma once
#include <rawrbox/render/model/model.h>
#include <rawrbox/render/model/model_mesh.h>
#include <rawrbox/render/texture/image.h>

#include <assimp/cimport.h>
#include <assimp/mesh.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <assimp/vector2.h>
#include <assimp/vector3.h>

#include <string>
#include <vector>

namespace rawrBox {
	class ModelImported : public Model {
		std::string _fileName;

		void loadTexture(const aiScene* sc, aiMesh& assimp, std::shared_ptr<rawrBox::ModelMesh>& mesh);
		void loadSubmeshes(const aiScene* sc, const aiNode* nd);

	public:
		// Loading ----
		void load(const std::string& path, uint32_t flags = aiProcessPreset_TargetRealtime_Fast | aiProcess_GenBoundingBoxes | aiProcess_ConvertToLeftHanded);
		// ---
	};
} // namespace rawrBox
