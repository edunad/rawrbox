#pragma once

#include <rawrbox/math/bbox.hpp>
#include <rawrbox/render/models/vertex.hpp>
#include <rawrbox/utils/logger.hpp>

#include <fastgltf/core.hpp>

#include <ozz/animation/offline/raw_animation.h>
#include <ozz/animation/offline/raw_skeleton.h>
#include <ozz/animation/runtime/animation.h>
#include <ozz/animation/runtime/skeleton.h>
#include <ozz/base/memory/unique_ptr.h>

#include <filesystem>
#include <memory>
#include <utility>

namespace rawrbox {
	// NOLINTBEGIN(unused-const-variable)
	namespace ModelLoadFlags {
		const uint32_t NONE = 0;
		const uint32_t IMPORT_LIGHT = 1 << 1;
		const uint32_t IMPORT_TEXTURES = 1 << 2;
		const uint32_t IMPORT_ANIMATIONS = 1 << 3;
		const uint32_t IMPORT_BLEND_SHAPES = 1 << 4;

		namespace Debug {
			const uint32_t PRINT_BONE_STRUCTURE = 1 << 20;
			const uint32_t PRINT_MATERIALS = 1 << 21;
			const uint32_t PRINT_ANIMATIONS = 1 << 22;
			const uint32_t PRINT_METADATA = 1 << 23;
			const uint32_t PRINT_BLENDSHAPES = 1 << 24;
		} // namespace Debug

		namespace Optimizer {
			const uint32_t SKELETON_ANIMATIONS = 1 << 30;
		} // namespace Optimizer

	}; // namespace ModelLoadFlags
	   // NOLINTEND(unused-const-variable)

	enum class GLTFImageType : uint32_t {
		WEBP = 0,
		DDS = 1,
		OTHER = 2
	};

	struct GLTFMaterial {
		std::string name;

		bool doubleSided = false;
		bool alpha = false;

		rawrbox::TextureBase* diffuse = nullptr;
		rawrbox::Colorf baseColor = rawrbox::Colors::White();

		rawrbox::TextureBase* normal = nullptr;
		rawrbox::TextureBase* specular = nullptr;
		rawrbox::TextureBase* metalRough = nullptr;

		rawrbox::Colorf specularColor = rawrbox::Colors::White();

		float roughnessFactor = 0.F;
		float metalnessFactor = 0.F;
		float specularFactor = 0.F;
		float emissionFactor = 1.F;

		rawrbox::TextureBase* emissive = nullptr;
		rawrbox::Colorf emissionColor = rawrbox::Colors::White();

		explicit GLTFMaterial(std::string _name) : name(std::move(_name)) {};
	};

	struct GLTFBlendShapes {
		std::string name;
		float weight = 0.F;

		size_t mesh_index = 0;

		std::vector<rawrbox::Vector3f> pos = {};
		std::vector<rawrbox::Vector4f> norms = {};
	};

	struct GLTFMesh {
	public:
		size_t index = 0;

		std::string name;
		rawrbox::BBOX bbox = {};
		rawrbox::Matrix4x4 matrix = {};

		rawrbox::GLTFMaterial* material = nullptr;
		ozz::animation::Skeleton* skeleton = nullptr;
		rawrbox::GLTFMesh* parent = nullptr;

		std::vector<rawrbox::VertexNormBoneData> vertices = {};
		std::vector<uint16_t> indices = {};

		rawrbox::Color color = rawrbox::Colors::White();
		GLTFMesh(std::string _name) : name(std::move(_name)) {};
	};

	struct GLTFAnimation {
		std::string name;
		float duration = 0.F;

		ozz::animation::Skeleton* skeleton = nullptr;
		std::unordered_map<std::string, ozz::animation::offline::RawAnimation::JointTrack> tracks = {};
	};

	class GLTFImporter {
	protected:
		// LOGGER ------
		std::unique_ptr<rawrbox::Logger> _logger = std::make_unique<rawrbox::Logger>("RawrBox-GLTF");
		// ------------

		// TEXTURES ----
		std::vector<rawrbox::TextureBase*> _texturesMap = {};
		// ----------

		// ANIMATIONS --
		std::vector<rawrbox::GLTFAnimation> _parsedAnimations = {};
		// ------------

		virtual void internalLoad(fastgltf::GltfDataBuffer& data);

		// POST-LOAD ---
		virtual void postLoadFixSceneNames(fastgltf::Asset& scene);
		//-----------

		// MATERIALS ---
		virtual void loadTextures(const fastgltf::Asset& scene);
		virtual void loadMaterials(const fastgltf::Asset& scene);

		virtual Diligent::SamplerDesc convertSampler(const fastgltf::Sampler& sample);
		//  -------------

		// SKELETONS --
		virtual ozz::math::Transform extractTransform(const std::variant<fastgltf::TRS, fastgltf::math::fmat4x4>& mtx);
		virtual void loadSkeletons(const fastgltf::Asset& scene);
		// ------------

		// ANIMATIONS ---
		virtual void loadAnimations(const fastgltf::Asset& scene);
		virtual void parseAnimations();
		// -------------

		// MODEL ---
		virtual void loadScene(const fastgltf::Asset& scene);
		virtual void loadMeshes(const fastgltf::Asset& scene, const fastgltf::Node& node, rawrbox::GLTFMesh* parentMesh = nullptr);

		virtual std::vector<rawrbox::VertexNormBoneData> extractVertex(const fastgltf::Asset& scene, const fastgltf::Primitive& primitive);
		virtual std::vector<uint16_t> extractIndices(const fastgltf::Asset& scene, const fastgltf::Primitive& primitive);
		// ----------

		// UTILS ---
		virtual fastgltf::sources::ByteView getSourceData(const fastgltf::Asset& scene, const fastgltf::DataSource& source);
		virtual rawrbox::Matrix4x4 toMatrix(const fastgltf::TRS& mtx);
		virtual rawrbox::Matrix4x4 toMatrix(const fastgltf::math::fmat4x4& mtx);

		template <typename T, std::size_t Extent>
		fastgltf::span<T, fastgltf::dynamic_extent> subspan(fastgltf::span<T, Extent> span, size_t offset, size_t count = fastgltf::dynamic_extent) {
			if (offset >= span.size()) {
				throw _logger->error("Offset is out of range");
			}

			if (count != fastgltf::dynamic_extent && count > span.size() - offset) {
				throw _logger->error("Count is out of range");
			}

			if (count == fastgltf::dynamic_extent) {
				count = span.size() - offset;
			}

			return fastgltf::span<T>{span.data() + offset, count};
		}
		// ------

	public:
		std::filesystem::path filePath;
		uint32_t loadFlags = 0;

		// EXTENSIONS --
		std::unordered_map<size_t, std::vector<std::string>> targetNames = {};
		// -------------

		// TEXTURES ----
		std::vector<std::unique_ptr<rawrbox::GLTFMaterial>> materials = {};
		std::vector<std::unique_ptr<rawrbox::TextureBase>> textures = {};
		// ------------

		// SKINNING ---
		std::vector<ozz::unique_ptr<ozz::animation::Skeleton>> skeletons = {};
		std::vector<ozz::unique_ptr<ozz::animation::Animation>> animations = {};

		std::unordered_map<size_t, std::unordered_set<rawrbox::GLTFMesh*>> vertexAnimation = {}; // Animation index -> mesh
		// ---------

		// BLEND SHAPES ---
		std::vector<std::unique_ptr<rawrbox::GLTFBlendShapes>> blendShapes = {};
		// ----------------

		// MODELS -------
		std::vector<std::unique_ptr<rawrbox::GLTFMesh>> meshes = {};
		// ---------------

		explicit GLTFImporter(uint32_t loadFlags = ModelLoadFlags::NONE);
		GLTFImporter(const GLTFImporter&) = delete;
		GLTFImporter(GLTFImporter&&) = delete;
		GLTFImporter& operator=(const GLTFImporter&) = delete;
		GLTFImporter& operator=(GLTFImporter&&) = delete;
		virtual ~GLTFImporter();

		// Loading ----
		virtual void load(const std::filesystem::path& path, const std::vector<uint8_t>& buffer);
		virtual void load(const std::filesystem::path& path);
		// ---
	};
} // namespace rawrbox
