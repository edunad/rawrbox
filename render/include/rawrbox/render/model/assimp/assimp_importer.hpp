#pragma once

#include <rawrbox/math/bbox.hpp>
#include <rawrbox/math/vector3.hpp>
#include <rawrbox/render/model/animation.hpp>
#include <rawrbox/render/model/defs.hpp>
#include <rawrbox/render/model/light/types.hpp>
#include <rawrbox/render/model/skeleton.hpp>
#include <rawrbox/render/texture/base.hpp>

#include <assimp/material.h>
#include <assimp/mesh.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <filesystem>

#define DEFAULT_ASSIMP_FLAGS (aiProcessPreset_TargetRealtime_Fast | aiProcess_GenBoundingBoxes | aiProcess_ConvertToLeftHanded | aiProcess_GlobalScale)

namespace rawrbox {
	// NOLINTBEGIN{unused-const-variable}
	namespace ModelLoadFlags {
		const uint32_t NONE = 0;
		const uint32_t IMPORT_LIGHT = 1 << 1;
		const uint32_t IMPORT_TEXTURES = 1 << 2;
		const uint32_t IMPORT_ANIMATIONS = 1 << 3;

		namespace Debug {
			const uint32_t PRINT_BONE_STRUCTURE = 1 << 10;
			const uint32_t PRINT_MATERIALS = 1 << 11;
			const uint32_t PRINT_ANIMATIONS = 1 << 12;
		} // namespace Debug

	}; // namespace ModelLoadFlags
	   // NOLINTEND{unused-const-variable}

	struct AssimpMaterial {
	public:
		std::string name = "";

		bool wireframe = false;
		bool doubleSided = false;
		uint64_t blending = BGFX_STATE_BLEND_NORMAL;

		std::shared_ptr<rawrbox::TextureBase> diffuse = nullptr;
		rawrbox::Colorf diffuseColor = rawrbox::Colors::White;

		std::shared_ptr<rawrbox::TextureBase> opacity = nullptr;

		std::shared_ptr<rawrbox::TextureBase> specular = nullptr;
		rawrbox::Colorf specularColor = rawrbox::Colors::White;
		float shininess = 25.F;

		std::shared_ptr<rawrbox::TextureBase> emissive = nullptr;
		rawrbox::Colorf emissionColor = rawrbox::Colors::White;
		float intensity = 1.F;

		explicit AssimpMaterial(std::string _name) : name(std::move(_name)){};
		~AssimpMaterial() {
			this->diffuse.reset();
			this->opacity.reset();
			this->specular.reset();
			this->emissive.reset();
		}
	};

	struct AssimpLight {
		rawrbox::LightType type = rawrbox::LightType::LIGHT_UNKNOWN;
		std::string name = "light";
		std::string parentID = "";

		rawrbox::Color diffuse = rawrbox::Colors::White;
		rawrbox::Color specular = rawrbox::Colors::White;
		rawrbox::Color ambient = rawrbox::Colors::Black;

		rawrbox::Vector3f pos = {};
		rawrbox::Vector3f direction = {};
		rawrbox::Vector3f up = {};

		float attenuationConstant = 0.F;
		float attenuationLinear = 0.F;
		float attenuationQuadratic = 0.F;

		float angleInnerCone = 0.F;
		float angleOuterCone = 0.F;
	};

	struct AssimpMesh {
	public:
		std::string name;
		rawrbox::BBOX bbox;
		rawrbox::Matrix4x4 offsetMatrix;

		std::weak_ptr<rawrbox::AssimpMaterial> material;
		rawrbox::Skeleton* skeleton = nullptr;

		bool animated = false;

		std::vector<rawrbox::VertexSkinnedLitData> vertices = {};
		std::vector<uint16_t> indices = {};

		AssimpMesh() = default;
		~AssimpMesh() {
			this->material.reset();
			this->skeleton = nullptr;

			this->vertices.clear();
			this->indices.clear();
		}
	};

	class AssimpImporter {

		// TEXTURE LOADING -----
		uint64_t assimpSamplerToBGFX(const std::array<aiTextureMapMode, 3>& mode, int axis);

		std::vector<std::shared_ptr<rawrbox::TextureBase>> importTexture(const aiMaterial* mat, aiTextureType type, bgfx::TextureFormat::Enum format = bgfx::TextureFormat::Count);
		void loadTextures(const aiScene* sc, aiMesh& assimp, rawrbox::AssimpMesh& mesh);
		/// -------

		// SKELETON LOADING -----
		void loadSkeleton(const aiScene* sc, rawrbox::AssimpMesh& mesh, const aiMesh& aiMesh);

		void generateSkeleton(rawrbox::Skeleton& skeleton, const aiNode* pNode, rawrbox::Bone& parent);
		aiNode* findRootSkeleton(const aiScene* sc, const std::string& meshName);

		bx::Easing::Enum assimpBehavior(aiAnimBehaviour b);

		void markMeshAnimated(const std::string& meshName, const std::string& search);
		void loadAnimations(const aiScene* sc);
		/// -------

		// LIGHT LOADING -----
		void loadLights(const aiScene* sc);
		/// -------

		// MESH LOADING -----
		void loadSubmeshes(const aiScene* sc, const aiNode* root);
		/// -------

		void internalLoad(const aiScene* scene, bool attemptedFallback = false);

	public:
		std::filesystem::path fileName;
		std::unordered_map<std::string, std::shared_ptr<rawrbox::AssimpMaterial>> materials = {};
		std::vector<rawrbox::AssimpMesh> meshes = {};

		uint32_t loadFlags{};
		uint32_t assimpFlags{};

		// SKINNING ----
		std::unordered_map<std::string, std::shared_ptr<rawrbox::Skeleton>> skeletons = {};

		std::unordered_map<std::string, rawrbox::AssimpMesh*> animatedMeshes = {}; // Map for quick lookup
		std::unordered_map<std::string, rawrbox::Animation> animations = {};
		std::vector<rawrbox::AssimpLight> lights = {};
		// --------

		explicit AssimpImporter(uint32_t loadFlags = ModelLoadFlags::NONE, uint32_t assimpFlags = DEFAULT_ASSIMP_FLAGS);
		~AssimpImporter();

		// Loading ----
		void load(const std::filesystem::path& path, const std::vector<uint8_t>& buffer, uint32_t loadFlags = ModelLoadFlags::NONE, uint32_t assimpFlags = DEFAULT_ASSIMP_FLAGS);
		void load(const std::filesystem::path& path, uint32_t loadFlags = ModelLoadFlags::NONE, uint32_t assimpFlags = DEFAULT_ASSIMP_FLAGS);
		// ---
	};
} // namespace rawrbox
