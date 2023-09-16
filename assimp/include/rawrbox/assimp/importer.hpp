#pragma once

#include <rawrbox/math/bbox.hpp>
#include <rawrbox/math/vector3.hpp>
#include <rawrbox/render/light/types.hpp>
#include <rawrbox/render/model/animation.hpp>
#include <rawrbox/render/model/defs.hpp>
#include <rawrbox/render/model/skeleton.hpp>
#include <rawrbox/render/texture/base.hpp>

#include <assimp/material.h>
#include <assimp/mesh.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <filesystem>
#include <functional>
#include <optional>

#define DEFAULT_ASSIMP_FLAGS (aiProcess_GenBoundingBoxes | aiProcess_ConvertToLeftHanded)

namespace rawrbox {
	// NOLINTBEGIN{unused-const-variable}
	namespace ModelLoadFlags {
		const uint32_t NONE = 0;
		const uint32_t IMPORT_LIGHT = 1 << 1;
		const uint32_t IMPORT_TEXTURES = 1 << 2;
		const uint32_t IMPORT_ANIMATIONS = 1 << 3;
		const uint32_t IMPORT_BLEND_SHAPES = 1 << 4;

		namespace Optimization {
			const uint32_t DISABLE = 1 << 10;
		}

		namespace Debug {
			const uint32_t PRINT_BONE_STRUCTURE = 1 << 20;
			const uint32_t PRINT_MATERIALS = 1 << 21;
			const uint32_t PRINT_ANIMATIONS = 1 << 22;
			const uint32_t PRINT_METADATA = 1 << 23;
			const uint32_t PRINT_BLENDSHAPES = 1 << 24;
		} // namespace Debug

	}; // namespace ModelLoadFlags
	   // NOLINTEND{unused-const-variable}

	using OptionalTexture = std::optional<std::unique_ptr<rawrbox::TextureBase>>;

	struct AssimpMaterial {
	public:
		std::string name = "";

		bool wireframe = false;
		bool doubleSided = false;
		uint64_t blending = BGFX_STATE_BLEND_NORMAL;

		OptionalTexture diffuse = std::nullopt;
		rawrbox::Colorf diffuseColor = rawrbox::Colors::White();

		std::unique_ptr<rawrbox::TextureBase> normal = nullptr;
		std::unique_ptr<rawrbox::TextureBase> specular = nullptr;
		rawrbox::Colorf specularColor = rawrbox::Colors::White();
		float shininess = 25.F;

		std::unique_ptr<rawrbox::TextureBase> emissive = nullptr;
		rawrbox::Colorf emissionColor = rawrbox::Colors::White();
		float intensity = 1.F;

		explicit AssimpMaterial(std::string _name) : name(std::move(_name)){};
	};

	struct AssimpLight {
		rawrbox::LightType type = rawrbox::LightType::UNKNOWN;
		std::string name = "light";
		std::string parentID = "";

		rawrbox::Colorf diffuse = rawrbox::Colors::White();
		rawrbox::Colorf specular = rawrbox::Colors::White();
		rawrbox::Colorf ambient = rawrbox::Colors::Black();

		rawrbox::Vector3f pos = {};
		rawrbox::Vector3f direction = {};
		rawrbox::Vector3f up = {};

		float attenuationConstant = 0.F;
		float attenuationLinear = 0.F;
		float attenuationQuadratic = 0.F;

		float angleInnerCone = 0.F;
		float angleOuterCone = 0.F;
	};

	struct AssimpBlendShapes {
		std::string name = "";
		float weight = 0.F;

		size_t mesh_index = 0;

		std::vector<rawrbox::Vector3f> pos = {};
		std::vector<rawrbox::Vector3f> norms = {};
	};

	struct AssimpMesh {
	public:
		std::string name;
		rawrbox::BBOX bbox = {};
		rawrbox::Matrix4x4 matrix = {};

		rawrbox::AssimpMaterial* material = nullptr;
		rawrbox::Skeleton* skeleton = nullptr;

		bool animated = false;

		std::vector<rawrbox::VertexData> vertices = {};
		std::vector<uint16_t> indices = {};

		explicit AssimpMesh(std::string _name) : name(std::move(_name)){};
	};

	class AssimpImporter {
	protected:
		uint32_t _aiAnimMeshIndex = 0;

		// TEXTURE LOADING -----
		virtual uint64_t assimpSamplerToBGFX(const std::array<aiTextureMapMode, 3>& mode, int axis);

		virtual std::vector<OptionalTexture> importTexture(const aiScene* scene, const aiMaterial* mat, aiTextureType type, bgfx::TextureFormat::Enum format = bgfx::TextureFormat::Count);
		virtual void loadTextures(const aiScene* sc, const aiMesh& assimp, rawrbox::AssimpMesh& mesh);
		/// -------

		// SKELETON LOADING -----
		virtual void loadSkeleton(const aiScene* sc, rawrbox::AssimpMesh& mesh, const aiMesh& aiMesh);

		virtual void generateSkeleton(rawrbox::Skeleton& skeleton, const aiNode* pNode, rawrbox::Bone& parent);
		virtual aiNode* findRootSkeleton(const aiScene* sc, const std::string& meshName);

		virtual bx::Easing::Enum assimpBehavior(aiAnimBehaviour b);

		virtual void markMeshAnimated(const std::string& meshName, const std::string& search);
		virtual void loadAnimations(const aiScene* sc);
		virtual void loadBlendShapes(const aiMesh& assimp, rawrbox::AssimpMesh& mesh);
		/// -------

		// LIGHT LOADING -----
		virtual void loadLights(const aiScene* sc);
		/// -------

		// MESH LOADING -----
		virtual void loadSubmeshes(const aiScene* sc, const aiNode* root);
		/// -------

		virtual void internalLoad(const aiScene* scene, bool attemptedFallback = false);

	public:
		std::filesystem::path fileName;
		std::unordered_map<std::string, std::unique_ptr<rawrbox::AssimpMaterial>> materials = {};
		std::vector<rawrbox::AssimpMesh> meshes = {};

		std::function<void(aiMetadata*)> onMetadata = nullptr;

		uint32_t loadFlags = 0;
		uint32_t assimpFlags = 0;

		// SKINNING ----
		std::unordered_map<std::string, std::unique_ptr<rawrbox::Skeleton>> skeletons = {};

		std::unordered_map<std::string, rawrbox::AssimpMesh*> animatedMeshes = {};    // Map for quick lookup
		std::unordered_map<std::string, rawrbox::AssimpBlendShapes> blendShapes = {}; // Map for quick lookup
		std::unordered_map<std::string, rawrbox::Animation> animations = {};

		std::vector<rawrbox::AssimpLight> lights = {};
		// --------

		explicit AssimpImporter(uint32_t loadFlags = ModelLoadFlags::NONE, uint32_t assimpFlags = DEFAULT_ASSIMP_FLAGS);
		AssimpImporter(const AssimpImporter&) = default;
		AssimpImporter(AssimpImporter&&) = delete;
		AssimpImporter& operator=(const AssimpImporter&) = default;
		AssimpImporter& operator=(AssimpImporter&&) = delete;
		virtual ~AssimpImporter() = default;

		// Loading ----
		virtual void load(const std::filesystem::path& path, const std::vector<uint8_t>& buffer, const std::string& hint = "");
		virtual void load(const std::filesystem::path& path);
		// ---
	};
} // namespace rawrbox
