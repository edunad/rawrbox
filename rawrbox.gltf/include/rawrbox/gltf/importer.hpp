#pragma once

#include <rawrbox/math/bbox.hpp>
#include <rawrbox/math/pi.hpp>
#include <rawrbox/math/utils/math.hpp>
#include <rawrbox/render/lights/types.hpp>
#include <rawrbox/render/models/animation.hpp>
#include <rawrbox/render/models/skeleton.hpp>
#include <rawrbox/render/models/vertex.hpp>
#include <rawrbox/utils/logger.hpp>

#include <fastgltf/core.hpp>
#include <fastgltf/tools.hpp>

#include <ozz/animation/offline/raw_animation.h>
#include <ozz/animation/offline/raw_skeleton.h>
#include <ozz/base/maths/transform.h>

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <memory>
#include <optional>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

// fastgltf accessor conversions ---
template <>
struct fastgltf::ElementTraits<ozz::math::Quaternion> : fastgltf::ElementTraitsBase<ozz::math::Quaternion, AccessorType::Vec4, float> {};

template <>
struct fastgltf::ElementTraits<ozz::math::Float3> : fastgltf::ElementTraitsBase<ozz::math::Float3, AccessorType::Vec3, float> {};

template <>
struct fastgltf::ElementTraits<rawrbox::Vector3f> : fastgltf::ElementTraitsBase<rawrbox::Vector3f, AccessorType::Vec3, float> {};

template <>
struct fastgltf::ElementTraits<rawrbox::Vector4f> : fastgltf::ElementTraitsBase<rawrbox::Vector4f, AccessorType::Vec4, float> {};
// ---------------------------------

namespace rawrbox {
	// NOLINTBEGIN(unused-const-variable)
	namespace GLTFLoadFlags {
		const uint32_t NONE = 0;

		const uint32_t IMPORT_LIGHT = 1 << 1;
		const uint32_t IMPORT_TEXTURES = 1 << 2;
		const uint32_t IMPORT_ANIMATIONS = 1 << 3;
		const uint32_t IMPORT_BLEND_SHAPES = 1 << 4;

		const uint32_t CALCULATE_BBOX = 1 << 5;
		const uint32_t VALIDATE = 1 << 6;

		namespace Debug {
			const uint32_t PRINT_BONE_STRUCTURE = 1 << 10;
			const uint32_t PRINT_MATERIALS = 1 << 11;
			const uint32_t PRINT_ANIMATIONS = 1 << 12;
			const uint32_t PRINT_BLENDSHAPES = 1 << 13;
			const uint32_t PRINT_OPTIMIZATION_STATS = 1 << 14;
		} // namespace Debug

		namespace Optimizer {
			const uint32_t MESH_OPTIMIZE = 1 << 20;
			const uint32_t MESH_SIMPLIFY = 1 << 21;

			const uint32_t SKELETON_ANIMATIONS = 1 << 22;
		} // namespace Optimizer

	}; // namespace GLTFLoadFlags
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
		size_t node = 0;

		std::string name;
		rawrbox::Matrix4x4 matrix = {};

		GLTFNode(size_t idx, size_t nodeIndex, const fastgltf::Node& node) : index(idx), node(nodeIndex), name(std::move(node.name)) {
			auto [translation, rotation, scale] = std::get<fastgltf::TRS>(node.transform);

			this->matrix = rawrbox::Matrix4x4::mtxSRT({scale.x(), scale.y(), scale.z()}, {rotation.x(), rotation.y(), rotation.z(), rotation.w()}, {translation.x(), translation.y(), translation.z()});
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

		GLTFLight(size_t idx, size_t nodeIndex, const fastgltf::Node& node, const fastgltf::Light& light) : rawrbox::GLTFNode(idx, nodeIndex, node) {
			this->color = rawrbox::Colorf(light.color.x(), light.color.y(), light.color.z(), 1.0F).toSRGB(); // KHR_lights_punctual colors are linear, light colors are sRGB
			this->radius = light.range.value_or(10.F);

			this->intensity = light.intensity / 683.F;
			this->angleInnerCone = rawrbox::MathUtils::toDeg(light.innerConeAngle.value_or(0.F)) * 2.F;
			this->angleOuterCone = rawrbox::MathUtils::toDeg(light.outerConeAngle.value_or(rawrbox::pi<float> / 4.F)) * 2.F;

			this->pos = this->matrix.getPos();
			this->direction = -this->matrix.getForward(); // KHR_lights_punctual inverted z

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

		rawrbox::Skeleton* skeleton = nullptr;

		GLTFMesh(size_t idx, size_t nodeIndex, const fastgltf::Node& node) : rawrbox::GLTFNode(idx, nodeIndex, node) {};
	};

	struct GLTFAnimation {
		std::string name;
		float duration = 0.F;

		std::unordered_map<size_t, ozz::animation::offline::RawAnimation::JointTrack> tracks = {};
	};

	class GLTFImporter {
	protected:
		// LOGGER ------
		std::unique_ptr<rawrbox::Logger> _logger = std::make_unique<rawrbox::Logger>("RawrBox-GLTF");
		// ------------

		// TEXTURES ----
		std::vector<rawrbox::TextureBase*> _texturesMap = {};
		// ----------

		// HIERARCHY ---
		std::vector<std::optional<size_t>> _nodeParents = {};
		std::unordered_map<size_t, rawrbox::GLTFMesh*> _nodeMeshes = {};
		// -------------

		// SKELETONS ---
		std::vector<rawrbox::Skeleton*> _skinSkeletons = {};
		std::unordered_map<rawrbox::Skeleton*, std::vector<size_t>> _skeletonJoints = {};
		std::unordered_map<size_t, std::vector<rawrbox::Skeleton*>> _nodeSkeletons = {};
		// -------------

		// ANIMATIONS --
		std::vector<rawrbox::GLTFAnimation> _parsedAnimations = {};
		// ------------

		virtual void internalLoad(fastgltf::GltfDataBuffer& data);

		// POST-LOAD ---
		virtual void postLoadFixSceneNames(fastgltf::Asset& scene);
		virtual bool buildHierarchy(const fastgltf::Asset& scene);
		//-----------

		// MATERIALS ---
		virtual void loadTextures(const fastgltf::Asset& scene);
		virtual void loadMaterials(const fastgltf::Asset& scene);

		virtual Diligent::SamplerDesc convertSampler(const fastgltf::Sampler& sample);
		//  -------------

		// SKELETONS --
		virtual void loadSkeletons(const fastgltf::Asset& scene);
		virtual ozz::animation::offline::RawSkeleton::Joint buildJoint(const fastgltf::Asset& scene, size_t nodeIndex, const std::unordered_set<size_t>& nodes, std::vector<size_t>& jointNodes);
		virtual void printJoint(const ozz::animation::offline::RawSkeleton::Joint& joint, int depth, bool isLast);

		[[nodiscard]] virtual ozz::math::Transform getRestPose(const fastgltf::Node& node) const;
		// ------------

		// ANIMATIONS ---
		virtual void loadAnimations(const fastgltf::Asset& scene);
		virtual void buildAnimations(const fastgltf::Asset& scene);

		static void fillRestPose(ozz::animation::offline::RawAnimation::JointTrack& track, const ozz::math::Transform& rest);
		static void toLeftHand(ozz::animation::offline::RawAnimation::JointTrack& track);

		// GLTF cubic spline
		static ozz::math::Float3 hermite(const ozz::math::Float3& p0, const ozz::math::Float3& m0, const ozz::math::Float3& p1, const ozz::math::Float3& m1, float t, float interval);
		static ozz::math::Quaternion hermite(const ozz::math::Quaternion& p0, const ozz::math::Quaternion& m0, const ozz::math::Quaternion& p1, const ozz::math::Quaternion& m1, float t, float interval);

		// Converts a channel into ozz keys. ozz only interpolates linearly, so STEP becomes hold keys and CubicSpline is baked at animationSampleRate
		template <typename T, typename Key>
		void extractKeys(const fastgltf::Asset& scene, const fastgltf::Accessor& timeAccessor, const fastgltf::Accessor& dataAccessor, fastgltf::AnimationInterpolation interpolation, float holdOffset, ozz::vector<Key>& keys, const std::string& animName) {
			const bool cubic = interpolation == fastgltf::AnimationInterpolation::CubicSpline;
			const bool step = interpolation == fastgltf::AnimationInterpolation::Step;
			const size_t stride = cubic ? 3 : 1; // CubicSpline: [in-tangent, value, out-tangent] per key

			if (dataAccessor.count != timeAccessor.count * stride) {
				this->_logger->warn("Invalid data for animation '{}', dataAccessor and timeAccessor do not match!", animName);
				return;
			}

			bool warnedOrder = false;
			float previousTime = -1.F;

			for (size_t k = 0; k < timeAccessor.count; k++) {
				const float t = fastgltf::getAccessorElement<float>(scene, timeAccessor, k);
				if (t < 0.F || t <= previousTime) { // ozz requires strictly ascending keys in [0, duration]
					if (!warnedOrder) {
						this->_logger->warn("Animation '{}' has negative or non-ascending key times, dropping keys", animName);
						warnedOrder = true;
					}

					continue;
				}

				previousTime = t;

				const T value = fastgltf::getAccessorElement<T>(scene, dataAccessor, k * stride + (cubic ? 1 : 0));
				keys.push_back({t, value});

				if (k + 1 >= timeAccessor.count) continue;

				const float next = fastgltf::getAccessorElement<float>(scene, timeAccessor, k + 1);
				const float interval = next - t;
				if (interval <= 0.F) continue;

				if (step) {
					if (interval > holdOffset * 2.F) { // Hold the value until right before the next key
						keys.push_back({next - holdOffset, value});
						previousTime = next - holdOffset;
					}
				} else if (cubic) {
					const T outTangent = fastgltf::getAccessorElement<T>(scene, dataAccessor, k * 3 + 2);
					const T nextValue = fastgltf::getAccessorElement<T>(scene, dataAccessor, (k + 1) * 3 + 1);
					const T inTangent = fastgltf::getAccessorElement<T>(scene, dataAccessor, (k + 1) * 3);

					const auto samples = static_cast<size_t>(std::ceil(interval * std::max(this->animationSampleRate, 0.F)));
					for (size_t sample = 1; sample < samples; sample++) {
						const float ratio = static_cast<float>(sample) / static_cast<float>(samples);
						const float time = t + ratio * interval;
						if (time <= previousTime || time >= next) continue; // Float rounding, keep the keys strictly ascending

						keys.push_back({time, rawrbox::GLTFImporter::hermite(value, outTangent, nextValue, inTangent, ratio, interval)});
						previousTime = time;
					}
				}
			}
		}
		// -------------

		// MODEL ---
		virtual void loadScene(const fastgltf::Asset& scene);
		virtual void loadNodes(const fastgltf::Asset& scene, size_t nodeIndex);

		virtual std::unique_ptr<rawrbox::GLTFMesh> extractMesh(const fastgltf::Asset& scene, size_t nodeIndex, const fastgltf::Node& node);

		virtual std::vector<rawrbox::VertexNormBoneData> extractVertex(const fastgltf::Asset& scene, const fastgltf::Primitive& primitive, std::optional<size_t> skinJoints);
		virtual std::vector<uint32_t> extractIndices(const fastgltf::Asset& scene, const fastgltf::Primitive& primitive);
		// ----------

		// UTILS ---
		virtual fastgltf::sources::ByteView getSourceData(const fastgltf::Asset& scene, const fastgltf::DataSource& source);
		[[nodiscard]] virtual bool validAccessor(const fastgltf::Asset& scene, size_t index, fastgltf::AccessorType type) const;

		template <typename T, std::size_t Extent>
		fastgltf::span<T, fastgltf::dynamic_extent> subspan(fastgltf::span<T, Extent> span, size_t offset, size_t count = fastgltf::dynamic_extent) {
			if (offset >= span.size()) {
				RAWRBOX_CRITICAL("Offset is out of range");
			}

			if (count != fastgltf::dynamic_extent && count > span.size() - offset) {
				RAWRBOX_CRITICAL("Count is out of range");
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

		float animationSampleRate = 30.F;

		// EXTENSIONS --
		std::unordered_map<size_t, std::vector<std::string>> targetNames = {};
		// -------------

		// TEXTURES ----
		std::vector<std::unique_ptr<rawrbox::GLTFMaterial>> materials = {};
		std::vector<std::unique_ptr<rawrbox::TextureBase>> textures = {};
		// ------------

		// SKINNING ---
		std::vector<std::unique_ptr<rawrbox::Skeleton>> skeletons = {};
		std::vector<std::unique_ptr<rawrbox::Animation>> animations = {};
		// ---------

		// LIGHTS ----
		std::vector<std::unique_ptr<rawrbox::GLTFLight>> lights = {};
		// -------------

		// MODELS -------
		std::vector<std::unique_ptr<rawrbox::GLTFMesh>> meshes = {};
		// ---------------

		explicit GLTFImporter(uint32_t loadFlags = GLTFLoadFlags::NONE);
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
