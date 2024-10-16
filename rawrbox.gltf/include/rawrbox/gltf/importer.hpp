#pragma once

#include <rawrbox/math/bbox.hpp>
#include <rawrbox/render/lights/types.hpp>
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

		const uint32_t CALCULATE_BBOX = 1 << 5;

		namespace Debug {
			const uint32_t PRINT_BONE_STRUCTURE = 1 << 10;
			const uint32_t PRINT_MATERIALS = 1 << 11;
			const uint32_t PRINT_ANIMATIONS = 1 << 12;
			const uint32_t PRINT_BLENDSHAPES = 1 << 13;
			const uint32_t PRINT_OPTIMIZATION_STATS = 1 << 14;
		} // namespace Debug

		namespace Optimizer {
			const uint32_t MESH = 1 << 20;
			const uint32_t SKELETON_ANIMATIONS = 1 << 21;
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

		bool transparent = false;
		float alphaCutoff = 0.5F;

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

	struct GLTFBlendShape {
		std::string name;
		float weight = 0.F;

		std::vector<rawrbox::Vector3f> pos = {};
		std::vector<rawrbox::Vector4f> norms = {};
	};

	struct GLTFNode {
		size_t index = 0;

		std::string name;
		rawrbox::Matrix4x4 matrix = {};

		GLTFNode(size_t idx, const fastgltf::Node& node) : index(idx), name(std::move(node.name)) {
			auto mtx = std::get<fastgltf::TRS>(node.transform);

			this->matrix = rawrbox::Matrix4x4::mtxSRT({mtx.scale.x(), mtx.scale.y(), mtx.scale.z()}, {mtx.rotation.x(), mtx.rotation.y(), mtx.rotation.z(), mtx.rotation.w()}, {mtx.translation.x(), mtx.translation.y(), mtx.translation.z()});
			this->matrix.toLeftHand();
		};
	};

	struct GLTFLight : public rawrbox::GLTFNode {
		rawrbox::LightType type = rawrbox::LightType::UNKNOWN;
		rawrbox::Colorf color = rawrbox::Colors::White();

		rawrbox::Vector3f pos = {};
		rawrbox::Vector3f direction = {};

		std::optional<size_t> parent = std::nullopt;

		float angleInnerCone = 0.F;
		float angleOuterCone = 0.F;

		float intensity = 1.F;
		float radius = 0.F;

		GLTFLight(size_t idx, const fastgltf::Node& node, const fastgltf::Light& light) : rawrbox::GLTFNode(idx, node) {
			this->color = rawrbox::Colorf(light.color.x(), light.color.y(), light.color.z(), 1.0F);
			this->radius = light.range.value_or(10.F);

			this->intensity = light.intensity / 12.F; // ???
			this->angleInnerCone = light.innerConeAngle.value_or(0.F);
			this->angleOuterCone = light.outerConeAngle.value_or(0.F);

			this->pos = this->matrix.getPos();
			this->direction = this->matrix.getForward();

			if (node.meshIndex) this->parent = node.meshIndex.value();

			switch (light.type) {
				case fastgltf::LightType::Directional:
					this->type = rawrbox::LightType::DIRECTIONAL;
					break;
				case fastgltf::LightType::Spot:
					this->type = rawrbox::LightType::SPOT;
					break;
				case fastgltf::LightType::Point:
					this->type = rawrbox::LightType::POINT;
					break;
			}
		};
	};

	struct GLTFPrimitive {
		rawrbox::GLTFMaterial* material = nullptr;
		std::vector<rawrbox::GLTFBlendShape> blendShapes = {};

		std::vector<rawrbox::VertexNormBoneData> vertices = {};
		std::vector<uint32_t> indices = {};
	};

	struct GLTFMesh : public rawrbox::GLTFNode {
	public:
		rawrbox::BBOX bbox = {};
		std::vector<rawrbox::GLTFPrimitive> primitives = {};

		ozz::animation::Skeleton* skeleton = nullptr;

		GLTFMesh(size_t idx, const fastgltf::Node& node) : rawrbox::GLTFNode(idx, node) {};
	};

	struct GLTFJoint : public rawrbox::GLTFNode {
		ozz::animation::Skeleton* skeleton = nullptr;

		GLTFJoint(size_t idx, const fastgltf::Node& node) : rawrbox::GLTFNode(idx, node) {};
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
		virtual void loadSkeletons(const fastgltf::Asset& scene);
		// ------------

		// ANIMATIONS ---
		virtual void loadAnimations(const fastgltf::Asset& scene);
		virtual void parseAnimations();
		// -------------

		// MODEL ---
		virtual void loadScene(const fastgltf::Asset& scene);
		virtual void loadNodes(const fastgltf::Asset& scene, const fastgltf::Node& node);

		virtual std::unique_ptr<rawrbox::GLTFMesh> extractMesh(const fastgltf::Asset& scene, const fastgltf::Node& node);

		virtual std::vector<rawrbox::VertexNormBoneData> extractVertex(const fastgltf::Asset& scene, const fastgltf::Primitive& primitive);
		virtual std::vector<uint32_t> extractIndices(const fastgltf::Asset& scene, const fastgltf::Primitive& primitive);
		// ----------

		// UTILS ---
		virtual fastgltf::sources::ByteView getSourceData(const fastgltf::Asset& scene, const fastgltf::DataSource& source);

		template <typename T, std::size_t Extent>
		fastgltf::span<T, fastgltf::dynamic_extent> subspan(fastgltf::span<T, Extent> span, size_t offset, size_t count = fastgltf::dynamic_extent) {
			if (offset >= span.size()) {
				CRITICAL_RAWRBOX("Offset is out of range");
			}

			if (count != fastgltf::dynamic_extent && count > span.size() - offset) {
				CRITICAL_RAWRBOX("Count is out of range");
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
		std::unordered_map<std::string, std::unique_ptr<rawrbox::GLTFJoint>> joints = {};

		std::unordered_map<size_t, std::unordered_set<rawrbox::GLTFMesh*>> vertexAnimation = {}; // Animation index -> mesh
		// ---------

		// LIGHTS ----
		std::vector<std::unique_ptr<rawrbox::GLTFLight>> lights = {};
		// -------------

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
