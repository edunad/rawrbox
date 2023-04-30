#pragma once

#include <rawrbox/render/model/mesh.hpp>
#include <rawrbox/render/model/model.hpp>
#include <rawrbox/render/texture/base.hpp>

#include <assimp/cimport.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <string>
#include <unordered_map>

#define DEFAULT_ASSIMP_FLAGS (aiProcessPreset_TargetRealtime_Fast | aiProcess_GenBoundingBoxes | aiProcess_ConvertToLeftHanded | aiProcess_RemoveRedundantMaterials)

namespace rawrBox {
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

	class ModelImported : public rawrBox::Model {
		using Model::Model;

		std::string _fileName;
		std::unordered_map<std::string, std::shared_ptr<rawrBox::TextureBase>> _textures;

		uint32_t _loadFlags;
		uint32_t _assimpFlags;

		std::shared_ptr<rawrBox::TextureBase> importTexture(const std::string& path, const std::string& name, const std::array<aiTextureMapMode, 3>& mode);

		void generateSkeleton(std::shared_ptr<Skeleton> skeleton, const aiNode* pNode, std::shared_ptr<rawrBox::Bone> parent);

		void loadTextures(const aiScene* sc, aiMesh& assimp, std::shared_ptr<rawrBox::Mesh>& mesh);
		void loadSubmeshes(const aiScene* sc, const aiNode* nd, std::shared_ptr<Mesh> parentMesh);
		void loadAnimations(const aiScene* sc);
		void loadSkeleton(const aiScene* sc, std::shared_ptr<Mesh> mesh, const aiMesh& aiMesh);
		void loadLights(const aiScene* sc);

	public:
		~ModelImported() override;

		ModelImported(ModelImported&&) = delete;
		ModelImported& operator=(ModelImported&&) = delete;
		ModelImported(const ModelImported&) = delete;
		ModelImported& operator=(const ModelImported&) = delete;

		// Loading ----
		void load(const std::string& path, uint32_t loadFlags = ModelLoadFlags::NONE, uint32_t assimpFlags = DEFAULT_ASSIMP_FLAGS);
		// ---
	};
} // namespace rawrBox
