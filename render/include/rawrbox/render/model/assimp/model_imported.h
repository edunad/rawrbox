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

#define DEFAULT_ASSIMP_FLAGS (0 | aiProcessPreset_TargetRealtime_Fast | aiProcess_GenBoundingBoxes | aiProcess_ConvertToLeftHanded)

namespace rawrBox {
	namespace ModelLoadFlags {
		const uint32_t NONE = 0;
		const uint32_t IMPORT_LIGHT = 1 << 1;
	}; // namespace ModelLoadFlags

	class ModelImported : public Model {
		std::string _fileName;
		std::unordered_map<std::string, std::shared_ptr<rawrBox::TextureBase>> _textures;

		void loadTexture(const aiScene* sc, aiMesh& assimp, std::shared_ptr<rawrBox::ModelMesh>& mesh);
		void loadSubmeshes(const aiScene* sc, const aiNode* nd);
		void loadLights(const aiScene* sc);

	public:
		virtual ~ModelImported();

		// Loading ----
		void load(const std::string& path, uint32_t loadFlags = ModelLoadFlags::NONE, uint32_t assimpFlags = DEFAULT_ASSIMP_FLAGS);
		// ---
	};
} // namespace rawrBox