#include <rawrbox/gltf/importer.hpp>
#include <rawrbox/render/textures/webp.hpp>
#include <rawrbox/utils/file.hpp>

#include <fastgltf/tools.hpp>
#include <fastgltf/types.hpp>
#include <magic_enum.hpp>

#include <meshoptimizer.h>
#include <ozz/animation/offline/animation_builder.h>
#include <ozz/animation/offline/animation_optimizer.h>
#include <ozz/animation/offline/raw_track.h>
#include <ozz/animation/offline/skeleton_builder.h>
#include <simdjson.h>

#include <variant>

template <>
struct fastgltf::ElementTraits<ozz::math::Quaternion> : fastgltf::ElementTraitsBase<ozz::math::Quaternion, AccessorType::Vec4, float> {};

template <>
struct fastgltf::ElementTraits<ozz::math::Float3> : fastgltf::ElementTraitsBase<ozz::math::Float3, AccessorType::Vec3, float> {};

template <>
struct fastgltf::ElementTraits<rawrbox::Vector3f> : fastgltf::ElementTraitsBase<rawrbox::Vector3f, AccessorType::Vec3, float> {};

template <>
struct fastgltf::ElementTraits<rawrbox::Vector4f> : fastgltf::ElementTraitsBase<rawrbox::Vector4f, AccessorType::Vec4, float> {};

namespace rawrbox {
	// PRIVATE -----
	void GLTFImporter::internalLoad(fastgltf::GltfDataBuffer& data) {
		auto extensions =
		    fastgltf::Extensions::KHR_mesh_quantization;

		auto gltfOptions =
		    fastgltf::Options::DecomposeNodeMatrices |
		    fastgltf::Options::LoadExternalBuffers;

		if ((this->loadFlags & rawrbox::ModelLoadFlags::IMPORT_TEXTURES) > 0) {
			extensions |= fastgltf::Extensions::KHR_materials_unlit |
				      fastgltf::Extensions::KHR_materials_specular | fastgltf::Extensions::KHR_texture_basisu |
				      fastgltf::Extensions::EXT_texture_webp | fastgltf::Extensions::KHR_materials_emissive_strength;

			gltfOptions |= fastgltf::Options::LoadExternalImages; // Handle loading for us
		}

		if ((this->loadFlags & rawrbox::ModelLoadFlags::IMPORT_LIGHT) > 0) {
			extensions |= fastgltf::Extensions::KHR_lights_punctual;
		}

		fastgltf::Parser parser(extensions);
		parser.setUserPointer(this);

		// Custom extras ----
		parser.setExtrasParseCallback([](simdjson::dom::object* extras, std::size_t objectIndex, fastgltf::Category objectType, void* userPointer) {
			if (extras == nullptr) return;

			auto* importer = static_cast<GLTFImporter*>(userPointer);
			if (importer == nullptr) return;

			if (objectType == fastgltf::Category::Meshes) {
				if ((importer->loadFlags & rawrbox::ModelLoadFlags::IMPORT_BLEND_SHAPES) > 0) {
					auto arr = extras->at_key("targetNames").get_array();
					if (arr.error() != simdjson::error_code::SUCCESS) return;

					for (auto target : arr) {
						auto targetName = std::string(target.get_string().value());
						if (targetName.empty()) targetName = fmt::format("blendshape_{}", importer->blendShapes.size());

						importer->targetNames[objectIndex].push_back(targetName);
					}
				}
			}
		});
		// ------------------

		auto asset = parser.loadGltf(data, this->filePath.parent_path(), gltfOptions);
		if (asset.error() != fastgltf::Error::None) {
			this->_logger->warn("{}", fastgltf::getErrorMessage(asset.error()));
			return;
		}

		fastgltf::Asset& scene = asset.get();

		// POST-LOAD ---
		this->postLoadFixSceneNames(scene);
		// ---------------

		// OPTIMIZATION --

		// ----------

		// LOAD MATERIALS ---
		if ((this->loadFlags & rawrbox::ModelLoadFlags::IMPORT_TEXTURES) > 0) {
			this->loadTextures(scene);
			this->loadMaterials(scene);
		}
		// --------------

		// LOAD SCENE ---
		this->loadScene(scene);
		//  ---------------

		// LOAD SKELETONS & ANIMATIONS ---
		if ((this->loadFlags & rawrbox::ModelLoadFlags::IMPORT_ANIMATIONS) > 0) {
			this->loadSkeletons(scene);
			this->loadAnimations(scene);
		}
		// -------------------
	}

	// POST-LOAD ---
	void GLTFImporter::postLoadFixSceneNames(fastgltf::Asset& scene) {
		const bool importAnims = (this->loadFlags & rawrbox::ModelLoadFlags::IMPORT_ANIMATIONS) > 0;
		const bool importTextures = (this->loadFlags & rawrbox::ModelLoadFlags::IMPORT_TEXTURES) > 0;
		const bool importLights = (this->loadFlags & rawrbox::ModelLoadFlags::IMPORT_LIGHT) > 0;

		// Fix scenes ---
		for (size_t i = 0; i < scene.scenes.size(); i++) {
			auto& scenes = scene.scenes[i];
			if (scenes.name.empty()) scenes.name = fmt::format("scene_{}", i);
		}
		// --------------

		// Fix nodes ---
		std::unordered_set<std::string> nodeNames;
		for (size_t i = 0; i < scene.nodes.size(); i++) {
			auto& nodes = scene.nodes[i];
			auto name = std::string(nodes.name);

			if (nodes.name.empty()) nodes.name = fmt::format("node_{}", i);
			if (nodeNames.contains(name)) {
				name = fmt::format("{}_{}", name, i);
				this->_logger->warn("Duplicate node name '{}', this is not supported!, renaming to '{}'", nodes.name, name);
			}

			nodes.name = name;
			nodeNames.insert(name);
		}
		// --------------

		if (importTextures) {
			// Fix images ---
			for (size_t i = 0; i < scene.images.size(); i++) {
				auto& image = scene.images[i];
				if (image.name.empty()) image.name = fmt::format("image_{}", i);
			}
			// --------------
		}

		if (importAnims) {
			// Fix animations ---
			for (size_t i = 0; i < scene.animations.size(); i++) {
				auto& anim = scene.animations[i];
				if (anim.name.empty()) anim.name = fmt::format("animation_{}", i);
			}
			// -------------------

			// Fix skins ---
			for (size_t i = 0; i < scene.skins.size(); i++) {
				auto& skin = scene.skins[i];
				if (skin.name.empty()) skin.name = fmt::format("skin_{}", i);
			}
			// ---------------
		}

		if (importLights) {
			// Fix lights ---
			for (size_t i = 0; i < scene.lights.size(); i++) {
				auto& light = scene.lights[i];
				if (light.name.empty()) light.name = fmt::format("light_{}", i);
			}
			// --------------
		}
	}
	//-----------

	// MATERIALS ---
	void GLTFImporter::loadTextures(const fastgltf::Asset& scene) {
		this->textures.resize(scene.textures.size());
		this->_texturesMap.resize(scene.textures.size(), nullptr);

		for (size_t i = 0; i < scene.textures.size(); i++) {
			const auto& gltfTexture = scene.textures[i];

			std::optional<std::pair<size_t, rawrbox::GLTFImageType>> imgIndex = std::nullopt;

			if (gltfTexture.basisuImageIndex.has_value()) {
				imgIndex = {gltfTexture.basisuImageIndex.value(), rawrbox::GLTFImageType::OTHER};
			} else if (gltfTexture.imageIndex.has_value()) {
				imgIndex = {gltfTexture.imageIndex.value(), rawrbox::GLTFImageType::OTHER};
			} else if (gltfTexture.webpImageIndex.has_value()) {
				imgIndex = {gltfTexture.webpImageIndex.value(), rawrbox::GLTFImageType::WEBP};
			} else if (gltfTexture.ddsImageIndex.has_value()) {
				// imgIndex = {gltfTexture.ddsImageIndex.value(), rawrbox::GLTFImageType::DDS};  // TODO: SUPPORT DDS
				continue;
			}

			std::string name(gltfTexture.name);
			if (!imgIndex.has_value()) {
				this->_logger->warn("Unsupported texture '{} -> {}'", i, name);
				continue;
			}

			auto& index = imgIndex.value();
			auto& image = this->textures[index.first];
			if (image != nullptr) {
				this->_texturesMap[i] = image.get();
				continue; // Already loaded
			}

			// Grab image data ---
			fastgltf::sources::ByteView imageData = this->getSourceData(scene, scene.images[index.first].data);
			if (imageData.bytes.empty()) {
				this->_logger->warn("Failed to load texture '{}'", name);
				continue;
			}
			// ------------------

			const auto* bah = std::bit_cast<const uint8_t*>(imageData.bytes.data());
			std::unique_ptr<rawrbox::TextureBase> texture = nullptr;

			switch (index.second) {
				case rawrbox::GLTFImageType::WEBP:
					texture = std::make_unique<rawrbox::TextureWEBP>(name, bah, static_cast<int>(imageData.bytes.size()));
					break;
				case GLTFImageType::DDS:
					this->_logger->warn("Unsupported texture '{} -> {}'", i, name);
					continue;
				case GLTFImageType::OTHER:
					texture = std::make_unique<rawrbox::TextureImage>(bah, static_cast<int>(imageData.bytes.size()));
					break;
			}

			texture->setName(name);

			if (gltfTexture.samplerIndex) {
				texture->setSampler(this->convertSampler(scene.samplers.at(gltfTexture.samplerIndex.value())));
			}

			texture->upload();

			// Register ----
			image = std::move(texture);
			this->_texturesMap[i] = image.get();
			// ---
		}
	}

	void GLTFImporter::loadMaterials(const fastgltf::Asset& scene) {
		this->materials.reserve(scene.materials.size());

		for (const auto& material : scene.materials) {
			auto mat = std::make_unique<rawrbox::GLTFMaterial>(material.name.c_str());

			// CULLING ----
			mat->doubleSided = material.doubleSided;
			// -----------

			// TRANSPARENCY ----
			mat->alpha = material.alphaMode != fastgltf::AlphaMode::Opaque;
			// ---------

			// Texture Loading ---

			// BASE ---
			float alpha = material.pbrData.baseColorFactor.w();
			if (material.pbrData.baseColorTexture.has_value()) {
				const auto& texture = this->_texturesMap[material.pbrData.baseColorTexture->textureIndex];
				mat->diffuse = texture == nullptr ? rawrbox::MISSING_TEXTURE.get() : texture;
			} else {
				mat->diffuse = rawrbox::WHITE_TEXTURE.get();
			}

			mat->baseColor = rawrbox::Colorf(material.pbrData.baseColorFactor.x(), material.pbrData.baseColorFactor.y(), material.pbrData.baseColorFactor.z(), alpha);
			// ----

			// METALIC ---
			if (material.pbrData.metallicRoughnessTexture.has_value()) {
				const auto& texture = this->_texturesMap[material.pbrData.metallicRoughnessTexture->textureIndex];
				mat->metalRough = texture == nullptr ? rawrbox::NORMAL_TEXTURE.get() : texture;
			}

			mat->metalnessFactor = material.pbrData.metallicFactor;
			mat->roughnessFactor = material.pbrData.roughnessFactor;
			// --------------------

			// NORMAL ---
			if (material.normalTexture.has_value()) {
				const auto& texture = this->_texturesMap[material.normalTexture->textureIndex];
				mat->normal = texture == nullptr ? rawrbox::NORMAL_TEXTURE.get() : texture;
			}
			// --------------------

			// EMISSION ---
			if (material.emissiveTexture) {
				const auto& texture = this->_texturesMap[material.emissiveTexture->textureIndex];
				mat->emissive = texture == nullptr ? rawrbox::BLACK_TEXTURE.get() : texture;
			}

			mat->emissionFactor = material.emissiveStrength;
			mat->emissionColor = rawrbox::Colorf(material.emissiveFactor.x(), material.emissiveFactor.y(), material.emissiveFactor.z(), alpha);
			// ---------

			// SPECULAR ---
			if (material.specular != nullptr) {
				if (material.specular->specularTexture) {
					const auto& texture = this->_texturesMap[material.specular->specularTexture->textureIndex];
					mat->specular = texture == nullptr ? rawrbox::MISSING_TEXTURE.get() : texture;
				}

				mat->specularColor = rawrbox::Colorf(material.specular->specularColorFactor.x(), material.specular->specularColorFactor.y(), material.specular->specularColorFactor.z(), alpha);
				mat->specularFactor = material.specular->specularFactor;
			}
			// --------------------

			this->materials.emplace_back(std::move(mat));
		}
	}

	Diligent::SamplerDesc GLTFImporter::convertSampler(const fastgltf::Sampler& sample) {
		auto mag = sample.magFilter.value_or(fastgltf::Filter::Nearest);
		auto min = sample.minFilter.value_or(fastgltf::Filter::Nearest);
		auto bit = sample.minFilter.value_or(fastgltf::Filter::Nearest);

		return {min == fastgltf::Filter::Nearest ? Diligent::FILTER_TYPE_POINT : Diligent::FILTER_TYPE_LINEAR,
		    mag == fastgltf::Filter::Nearest ? Diligent::FILTER_TYPE_POINT : Diligent::FILTER_TYPE_LINEAR,
		    bit == fastgltf::Filter::Nearest ? Diligent::FILTER_TYPE_POINT : Diligent::FILTER_TYPE_LINEAR};
	}
	// -------------

	// SKELETONS --
	ozz::math::Transform GLTFImporter::extractTransform(const std::variant<fastgltf::TRS, fastgltf::math::fmat4x4>& mtx) {
		ozz::math::Transform tr = ozz::math::Transform::identity();

		if (std::holds_alternative<fastgltf::TRS>(mtx)) {
			const auto& trs = std::get<fastgltf::TRS>(mtx);

			tr.translation = {trs.translation.x(), trs.translation.y(), trs.translation.z()};
			tr.rotation = {trs.rotation.x(), trs.rotation.y(), trs.rotation.z(), trs.rotation.w()};
			tr.scale = {trs.scale.x(), trs.scale.y(), trs.scale.z()};
		}

		return tr;
	}

	void GLTFImporter::loadSkeletons(const fastgltf::Asset& scene) {
		if (scene.skins.empty()) return;
		ozz::animation::offline::SkeletonBuilder builder;

		// UTILS -------

		// GENERATE CHILDREN BONES ---
		std::function<void(const fastgltf::Node& node, ozz::animation::offline::RawSkeleton::Joint& parent, std::unordered_set<size_t>& processedJoints)> generateBones;
		generateBones = [this, &scene, &generateBones](const fastgltf::Node& node, ozz::animation::offline::RawSkeleton::Joint& parent, std::unordered_set<size_t>& processedJoints) {
			for (const auto& childIndex : node.children) {
				const auto& childNode = scene.nodes[childIndex];
				if (childIndex > this->meshes.size()) throw this->_logger->error("Missing child index '{}' mesh", childIndex);

				ozz::animation::offline::RawSkeleton::Joint childJoint = {};
				childJoint.name = std::string(childNode.name);
				childJoint.transform = extractTransform(childNode.transform);

				processedJoints.insert(childIndex);
				generateBones(childNode, childJoint, processedJoints);

				parent.children.push_back(childJoint);
			}
		};
		// -----------------------

		// DEBUG ----
		std::function<void(const ozz::animation::offline::RawSkeleton::Joint&, int, bool)> printBone;
		if ((this->loadFlags & rawrbox::ModelLoadFlags::Debug::PRINT_BONE_STRUCTURE) > 0) {
			printBone = [this, &printBone](const ozz::animation::offline::RawSkeleton::Joint& bn, int depth, bool isLast) -> void {
				std::string indent(depth * 4, ' ');
				std::string branch = isLast ? "└── " : "├── ";
				this->_logger->info("{}{}{}", indent, branch, bn.name);

				for (size_t i = 0; i < bn.children.size(); i++) {
					bool lastChild = (i == bn.children.size() - 1);
					printBone(bn.children[i], depth + 1, lastChild);
				}
			};
		}
		// -------------
		// -----------------------

		this->skeletons.resize(scene.skins.size());
		for (size_t i = 0; i < scene.skins.size(); i++) {
			const auto& skin = scene.skins[i];
			auto skinName = std::string(skin.name);

			if (skin.joints.empty()) {
				this->_logger->warn("Skin '{}' has no joints, skipping...", skin.name);
				continue;
			}

			if (!skin.inverseBindMatrices.has_value()) {
				this->_logger->warn("Skin '{}' has no inverse bind matrices, skipping...", skin.name);
				continue;
			}

			// EXTRACT INVERTED BIND MATRICES ----
			std::vector<std::array<float, 16>> inverseBindMatrices(skin.joints.size());

			fastgltf::iterateAccessorWithIndex<fastgltf::math::fmat4x4>(
			    scene, scene.accessors[skin.inverseBindMatrices.value()],
			    [&](const fastgltf::math::fmat4x4& mtx, size_t index) {
				    std::memcpy(inverseBindMatrices[index].data(), mtx.data(), sizeof(float) * mtx.size());
			    });
			// ---------------------------

			// BUILD SKELETON STRUCTURE ----
			ozz::animation::offline::RawSkeleton rawSkeleton;
			std::unordered_set<size_t> processedBones;

			for (const auto& rootJoint : skin.joints) {
				auto& root = scene.nodes[rootJoint];
				if (processedBones.contains(rootJoint)) continue; // This is the only way to determine root joints

				ozz::animation::offline::RawSkeleton::Joint rootBone;
				rootBone.name = root.name;
				rootBone.transform = this->extractTransform(root.transform);

				processedBones.insert(rootJoint); // Track it so we can track the root joints
				if (!root.children.empty()) generateBones(root, rootBone, processedBones);

				rawSkeleton.roots.push_back(rootBone);
			}
			// ----------------------

			// DEBUG ----
			if (printBone != nullptr) {
				this->_logger->info("Found skeleton '{}' ->", fmt::styled(skin.name, fmt::fg(fmt::color::green_yellow)));
				for (const ozz::animation::offline::RawSkeleton::Joint& root : rawSkeleton.roots) {
					printBone(root, 0, true);
				}
			}
			// ------------

			// BUILD ---
			auto buildSkeleton = builder(rawSkeleton);
			if (buildSkeleton == nullptr) {
				this->_logger->warn("Failed to build skeleton '{}'", skin.name);
				continue;
			}

			this->skeletons[i] = std::move(buildSkeleton);
			this->skeletons[i]->inverseBindMatrices = inverseBindMatrices;
			// ----------------------

			// Apply skins on bones --
			for (auto& joint : skin.joints) {
				if (joint >= this->meshes.size()) {
					this->_logger->warn("Invalid mesh index '{}'", joint);
					continue;
				}

				this->meshes[joint]->skeleton = this->skeletons[i].get();
			}
			// --------------
		}

		// Apply skins on meshes --
		for (auto& node : scene.nodes) {
			if (!node.skinIndex || !node.meshIndex) continue;
			this->meshes[node.meshIndex.value()]->skeleton = this->skeletons[node.skinIndex.value()].get();
		}
		// --------------
	}
	// ------------

	// ANIMATIONS ---
	void GLTFImporter::loadAnimations(const fastgltf::Asset& scene) {
		if (scene.animations.empty()) return;

		this->_parsedAnimations.resize(scene.animations.size());
		for (size_t i = 0; i < scene.animations.size(); i++) {
			const auto& anim = scene.animations[i];
			if (anim.channels.empty()) continue;

			rawrbox::GLTFAnimation& gltfAnim = this->_parsedAnimations[i];
			gltfAnim.name = std::string(anim.name.begin(), anim.name.end());
			gltfAnim.duration = 0.001F;

			if ((this->loadFlags & rawrbox::ModelLoadFlags::Debug::PRINT_ANIMATIONS) > 0) {
				this->_logger->info("Found animation '{}'", fmt::styled(gltfAnim.name, fmt::fg(fmt::color::green_yellow)));
			}

			for (size_t c = 0; c < anim.channels.size(); c++) {
				const auto& channel = anim.channels[c];
				if (!channel.nodeIndex) continue;

				const auto& sampler = anim.samplers[channel.samplerIndex];
				const auto& timeAccessor = scene.accessors[sampler.inputAccessor];
				const auto& dataAccessor = scene.accessors[sampler.outputAccessor];

				if (timeAccessor.count != dataAccessor.count) {
					this->_logger->warn("Invalid data for animation '{}', dataAccessor and timeAccessor do not match!", gltfAnim.name);
					continue;
				}

				if (sampler.interpolation == fastgltf::AnimationInterpolation::CubicSpline) {
					this->_logger->warn("Unsupported channel interpolation 'CubicSpline' for animation '{}'", gltfAnim.name);
					continue;
				}

				const size_t nodeIndex = channel.nodeIndex.value();
				const auto& mesh = this->meshes[nodeIndex]; // Mesh being affected

				const auto& node = scene.nodes[nodeIndex]; // Node being affected
				auto nodeName = std::string(node.name);
				auto transform = this->extractTransform(node.transform);

				// Check if it's a skeleton animation
				if (mesh->skeleton != nullptr) {
					if (gltfAnim.skeleton != nullptr && gltfAnim.skeleton != mesh->skeleton) {
						this->_logger->warn("Animation '{}' contains 2 or more skeletons, this is not supported! Please split the animation per skeleton", gltfAnim.name);
						continue;
					} else {
						gltfAnim.skeleton = mesh->skeleton;
					}
				} else {
					this->vertexAnimation[i].insert(mesh.get());
				}
				// ----------------

				// TIME ----
				auto& track = gltfAnim.tracks[nodeName];

				for (size_t iTime = 0; iTime < timeAccessor.count; iTime++) {
					double t = fastgltf::getAccessorElement<double>(scene, timeAccessor, iTime);
					if (t > gltfAnim.duration) gltfAnim.duration = t; // Calculate the total animation time

					switch (channel.path) {
						case fastgltf::AnimationPath::Translation:
							{
								ozz::math::Float3 key = fastgltf::getAccessorElement<ozz::math::Float3>(scene, dataAccessor, iTime);
								track.translations.emplace_back(t, key);
								break;
							}
						case fastgltf::AnimationPath::Rotation:
							{
								ozz::math::Quaternion key = fastgltf::getAccessorElement<ozz::math::Quaternion>(scene, dataAccessor, iTime);
								track.rotations.emplace_back(t, key);
								break;
							}
						case fastgltf::AnimationPath::Scale:
							{
								ozz::math::Float3 key = fastgltf::getAccessorElement<ozz::math::Float3>(scene, dataAccessor, iTime);
								track.scales.emplace_back(t, key);
								break;
							}
						case fastgltf::AnimationPath::Weights:
							this->_logger->warn("Unsupported channel path 'Weights' for animation '{}'", gltfAnim.name);
							break; // TODO: SUPPORT BLEND SHAPES
						default:
							// Handle other cases
							break;
					}
				}
				// -------------
			}
		}

		// Ok, we done calculating the animations, time to parse ---
		this->parseAnimations();
		//- -----------------------------------------------------------
	}

	void GLTFImporter::parseAnimations() {
		if (this->_parsedAnimations.empty()) return;
		ozz::animation::offline::AnimationBuilder builder;

		this->_logger->info("Building {} animations...", this->_parsedAnimations.size());

		this->animations.resize(this->_parsedAnimations.size());
		for (size_t i = 0; i < this->_parsedAnimations.size(); i++) {
			auto& anim = this->_parsedAnimations[i];

			ozz::animation::offline::RawAnimation rawrAnim;
			rawrAnim.duration = anim.duration;
			rawrAnim.name = anim.name;

			if (anim.skeleton != nullptr) {
				for (std::string bone : anim.skeleton->joint_names()) {
					rawrAnim.tracks.emplace_back(anim.tracks[bone]); // All bones need to be tracked for skeleton animations
				}
			} else {
				for (const auto& track : anim.tracks) {
					rawrAnim.tracks.emplace_back(track.second);
				}
			}

			if (rawrAnim.tracks.empty()) {
				this->_logger->warn("Animation '{}' has no tracks, skipping...", rawrAnim.name);
				continue;
			}

			// Optimize skeleton
			if ((this->loadFlags & rawrbox::ModelLoadFlags::Optimizer::SKELETON_ANIMATIONS) > 0 && anim.skeleton != nullptr) {
				ozz::animation::offline::AnimationOptimizer optimizer;
				ozz::animation::offline::RawAnimation input = rawrAnim;

				if (!optimizer(input, *anim.skeleton, &rawrAnim)) {
					this->_logger->warn("Failed to optimize animation '{}'", rawrAnim.name);
				}
			}
			// ------------

			// Build the animations ---
			auto buildAnim = builder(rawrAnim);
			if (buildAnim == nullptr) {
				this->_logger->warn("Failed to build animation '{}'", rawrAnim.name);
				continue;
			}

			this->animations[i] = std::move(buildAnim);
			this->animations[i]->type = anim.skeleton != nullptr ? ozz::animation::AnimationType::SKELETON : ozz::animation::AnimationType::VERTEX;
			// ----------------------
		}
	}
	// -------------

	// MODEL ---
	void GLTFImporter::loadScene(const fastgltf::Asset& scene) {
		for (const auto& rootScenes : scene.scenes) {
			for (const auto& nodeIndex : rootScenes.nodeIndices) {
				this->loadMeshes(scene, scene.nodes[nodeIndex], nullptr);
			}
		}
	}

	void GLTFImporter::loadMeshes(const fastgltf::Asset& scene, const fastgltf::Node& node, rawrbox::GLTFMesh* parentMesh) {
		auto gltfMesh = std::make_unique<rawrbox::GLTFMesh>(std::string{node.name.begin(), node.name.end()});

		gltfMesh->parent = parentMesh;
		gltfMesh->index = this->meshes.size();
		gltfMesh->matrix = this->toMatrix(std::get<fastgltf::TRS>(node.transform));

		if (node.meshIndex) {
			const bool importBlendShapes = (this->loadFlags & rawrbox::ModelLoadFlags::IMPORT_BLEND_SHAPES) > 0;
			const auto& mesh = scene.meshes[node.meshIndex.value()];

			// Weights --
			std::vector<std::unique_ptr<rawrbox::GLTFBlendShapes>> blendShapes = {};

			if (importBlendShapes && !mesh.weights.empty()) {
				auto& blendNames = this->targetNames[node.meshIndex.value()];
				if (blendNames.empty()) {
					this->_logger->warn("Mesh '{}' has no blend shape name, skipping...", gltfMesh->name);
					return;
				}

				if (blendNames.size() != mesh.weights.size()) {
					this->_logger->warn("Mesh '{}' has {} blend shape names, but only {} weights, skipping...", gltfMesh->name, blendNames.size(), mesh.weights.size());
					return;
				}

				blendShapes.resize(blendNames.size());
				for (size_t i = 0; i < blendNames.size(); i++) {
					const auto& name = blendNames[i];
					auto shape = std::make_unique<rawrbox::GLTFBlendShapes>();

					shape->name = fmt::format("{}-{}", gltfMesh->name, name);
					shape->mesh_index = gltfMesh->index;
					shape->weight = mesh.weights[shape->mesh_index]; // Default weight

					blendShapes[i] = std::move(shape);
				}
			}
			// ----------

			// Primitives
			for (const auto& primitive : mesh.primitives) {
				if (primitive.type != fastgltf::PrimitiveType::Triangles) {
					this->_logger->warn("Primitive type '{}' not supported", magic_enum::enum_name(primitive.type));
					continue;
				}

				// Textures --
				if (primitive.materialIndex) {
					if ((this->loadFlags & rawrbox::ModelLoadFlags::IMPORT_TEXTURES) > 0) {
						auto* material = this->materials[primitive.materialIndex.value()].get();

						if (gltfMesh->material != nullptr && gltfMesh->material != material) {
							this->_logger->warn("Tried to load 2 different materials on the same submesh, this isn't supported!");
						} else {
							gltfMesh->material = material;
						}
					}
				}
				// ----------

				// BLEND SHAPES --
				if (importBlendShapes && !primitive.targets.empty()) {
					for (size_t i = 0; i < primitive.targets.size(); i++) {
						if (blendShapes[i] == nullptr) throw this->_logger->error("Invalid blend shape '{}'", i);
						const auto& targets = primitive.targets[i];

						// POSITION ---
						const auto* positionTarget = primitive.findTargetAttribute(i, "POSITION");
						if (positionTarget != nullptr && positionTarget != targets.end()) {
							const auto& positionAccessor = scene.accessors[positionTarget->accessorIndex];
							blendShapes[i]->pos.resize(positionAccessor.count);

							fastgltf::iterateAccessorWithIndex<fastgltf::math::fvec3>(scene, positionAccessor, [&](fastgltf::math::fvec3 pos, size_t index) {
								blendShapes[i]->pos[index] = rawrbox::Vector3f(pos.x(), pos.y(), pos.z());
							});
						}
						// ----------------

						// NORMAL ---
						const auto* normalTarget = primitive.findTargetAttribute(i, "NORMAL");
						if (normalTarget != nullptr && normalTarget != targets.end()) {
							const auto& normAccessor = scene.accessors[normalTarget->accessorIndex];
							blendShapes[i]->norms.resize(normAccessor.count);

							fastgltf::iterateAccessorWithIndex<fastgltf::math::fvec3>(scene, normAccessor, [&](fastgltf::math::fvec3 norm, size_t index) {
								blendShapes[i]->norms[index] = rawrbox::Vector4f(norm.x(), norm.y(), norm.z(), 0.0F);
							});
						}
						// ----------------
					}
				}
				// ----------------

				std::vector<rawrbox::VertexNormBoneData> verts = this->extractVertex(scene, primitive);
				std::vector<uint32_t> indices = this->extractIndices(scene, primitive);

				if ((this->loadFlags & rawrbox::ModelLoadFlags::Optimizer::MESH) > 0) {
					auto startVert = verts.size();
					auto startInd = indices.size();

					this->optimizeMesh(verts, indices);
					this->simplifyMesh(verts, indices);

					if (startVert != verts.size() || startInd != indices.size()) {
						this->_logger->info("Optimized mesh '{}'\n\tVertices -> {} to {}\n\tIndices -> {} to {}", fmt::styled(gltfMesh->name, fmt::fg(fmt::color::cyan)), startVert, verts.size(), startInd, indices.size());
					}
				}

				gltfMesh->vertices.insert(gltfMesh->vertices.end(), verts.begin(), verts.end());
				gltfMesh->indices.insert(gltfMesh->indices.end(), indices.begin(), indices.end());
			}
			/// -------

			if (!blendShapes.empty()) {
				const bool debugBlend = (this->loadFlags & rawrbox::ModelLoadFlags::Debug::PRINT_BLENDSHAPES) > 0;

				if (debugBlend) this->_logger->info("'{}' BLEND SHAPES", fmt::styled(mesh.name, fmt::fg(fmt::color::cyan)));
				for (auto& shape : blendShapes) {
					if (debugBlend) {
						auto nm = rawrbox::StrUtils::split(shape->name, '-');
						fmt::print("\t---> {} \n", fmt::styled(nm.back(), fmt::fg(fmt::color::cyan)));
					}

					this->blendShapes.emplace_back(std::move(shape));
				}
			}
		}

		// Add meshes ---
		auto* mdlGet = gltfMesh.get();
		this->meshes.emplace_back(std::move(gltfMesh));
		// ----

		// Children ---
		for (const auto& children : node.children) {
			this->loadMeshes(scene, scene.nodes[children], mdlGet);
		}
		// ---
	}

	std::vector<rawrbox::VertexNormBoneData> GLTFImporter::extractVertex(const fastgltf::Asset& scene, const fastgltf::Primitive& primitive) {
		std::vector<rawrbox::VertexNormBoneData> verts = {};

		// POSITION ----
		const auto* positionAttribute = primitive.findAttribute("POSITION");
		if (positionAttribute == nullptr) throw this->_logger->error("Invalid gltf model, missing 'POSITION' attribute!"); // All models have POSITION

		const auto& positionAccessor = scene.accessors[positionAttribute->accessorIndex];
		verts.resize(positionAccessor.count);
		fastgltf::iterateAccessorWithIndex<fastgltf::math::fvec3>(scene, positionAccessor, [&](fastgltf::math::fvec3 pos, std::size_t idx) {
			verts[idx].position = rawrbox::Vector3f(pos.x(), pos.y(), pos.z());
		});
		// ------------

		// NORMALS ----
		const auto* normalAttribute = primitive.findAttribute("NORMAL");

		if (normalAttribute != nullptr && normalAttribute != primitive.attributes.end()) {
			const auto& normalAccessor = scene.accessors[normalAttribute->accessorIndex];
			fastgltf::iterateAccessorWithIndex<fastgltf::math::fvec3>(scene, normalAccessor, [&](fastgltf::math::fvec3 normal, std::size_t idx) {
				verts[idx].normal = rawrbox::PackUtils::packNormal(normal.x(), normal.y(), normal.z());
			});
		}
		// ------------

		// TANGENTS ----
		const auto* tangentAttribute = primitive.findAttribute("TANGENT");

		if (tangentAttribute != nullptr && tangentAttribute != primitive.attributes.end()) {
			const auto& tangentAccessor = scene.accessors[tangentAttribute->accessorIndex];

			fastgltf::iterateAccessorWithIndex<fastgltf::math::fvec3>(
			    scene, tangentAccessor,
			    [&](const fastgltf::math::fvec3& tangent, std::size_t idx) {
				    verts[idx].tangent = rawrbox::PackUtils::packNormal(tangent.x(), tangent.y(), tangent.z());
			    });
		}
		// calculate tangent using
		//  ------------

		// UV -----
		const auto* uvIt = primitive.findAttribute("TEXCOORD_0");
		if (uvIt != nullptr && uvIt != primitive.attributes.end()) {
			const auto& uvAccessor = scene.accessors[uvIt->accessorIndex];
			fastgltf::iterateAccessorWithIndex<fastgltf::math::fvec2>(scene, uvAccessor, [&](fastgltf::math::fvec2 uv, std::size_t idx) {
				verts[idx].uv = rawrbox::Vector2f(uv.x(), uv.y());
			});
		}
		// -------------

		// BONES -----
		if ((this->loadFlags & rawrbox::ModelLoadFlags::IMPORT_ANIMATIONS) > 0) {
			const auto* jointIt = primitive.findAttribute("JOINTS_0");
			const auto* weightIt = primitive.findAttribute("WEIGHTS_0");

			if ((jointIt != nullptr && jointIt != primitive.attributes.end()) && (weightIt != nullptr && weightIt != primitive.attributes.end())) {
				const auto& jointAccessor = scene.accessors[jointIt->accessorIndex];
				const auto& weightAccessor = scene.accessors[weightIt->accessorIndex];

				fastgltf::iterateAccessorWithIndex<fastgltf::math::uvec4>(scene, jointAccessor, [&](fastgltf::math::uvec4 joints, std::size_t idx) {
					verts[idx].bone_indices = {joints.x(), joints.y(), joints.z(), joints.w()};
				});

				fastgltf::iterateAccessorWithIndex<fastgltf::math::fvec4>(scene, weightAccessor, [&](fastgltf::math::fvec4 weights, std::size_t idx) {
					verts[idx].bone_weights = {weights.x(), weights.y(), weights.z(), weights.w()};
				});
			}
		}
		// -------------

		return verts;
	}

	std::vector<uint32_t> GLTFImporter::extractIndices(const fastgltf::Asset& scene, const fastgltf::Primitive& primitive) {
		auto indices = std::vector<uint32_t>();

		const auto& accessor = scene.accessors[primitive.indicesAccessor.value()];
		indices.resize(accessor.count);

		// INDICES ----
		fastgltf::iterateAccessorWithIndex<uint32_t>(scene, accessor, [&](uint32_t indice, size_t idx) { indices[idx] = indice; });
		// -----------

		return indices;
	}
	// ----------

	// UTILS ---
	fastgltf::sources::ByteView GLTFImporter::getSourceData(const fastgltf::Asset& scene, const fastgltf::DataSource& source) {
		return std::visit(fastgltf::visitor{
				      [&](auto& /*arg*/) -> fastgltf::sources::ByteView {
					      throw this->_logger->error("Invalid data");
				      },
				      [&](std::monostate) -> fastgltf::sources::ByteView {
					      throw this->_logger->error("Invalid data");
				      },
				      [&](fastgltf::sources::Fallback) -> fastgltf::sources::ByteView {
					      throw this->_logger->error("Invalid data");
				      },
				      [&](const fastgltf::sources::BufferView& buffer_view) -> fastgltf::sources::ByteView {
					      const fastgltf::BufferView& view = scene.bufferViews.at(buffer_view.bufferViewIndex);
					      const fastgltf::Buffer& buffer = scene.buffers.at(view.bufferIndex);

					      auto data = this->getSourceData(scene, buffer.data);
					      return {subspan(data.bytes, view.byteOffset, view.byteLength), buffer_view.mimeType};
				      },
				      [&](const fastgltf::sources::URI& /*filePath*/) -> fastgltf::sources::ByteView {
					      throw this->_logger->error("Use fastgltf::Options::LoadExternalImages instead!");
				      },
				      [&](const fastgltf::sources::Vector& vector) -> fastgltf::sources::ByteView {
					      fastgltf::span<const std::byte> data{std::bit_cast<const std::byte*>(vector.bytes.data()), vector.bytes.size()};
					      return {data, vector.mimeType};
				      },
				      [&](const fastgltf::sources::Array& array) -> fastgltf::sources::ByteView {
					      fastgltf::span<const std::byte> data{std::bit_cast<const std::byte*>(array.bytes.data()), array.bytes.size()};
					      return {data, array.mimeType};
				      },
				      [&](const fastgltf::sources::CustomBuffer& /*custom_buffer*/) -> fastgltf::sources::ByteView {
					      throw this->_logger->error("Invalid data");
				      },
				      [&](fastgltf::sources::ByteView& byte_view) -> fastgltf::sources::ByteView {
					      return {byte_view.bytes, byte_view.mimeType};
				      }},
		    source);
	}

	rawrbox::Matrix4x4 GLTFImporter::toMatrix(const fastgltf::TRS& mtx) {
		return rawrbox::Matrix4x4::mtxSRT({mtx.scale.x(), mtx.scale.y(), mtx.scale.z()}, {mtx.rotation.x(), mtx.rotation.y(), mtx.rotation.z(), mtx.rotation.w()}, {mtx.translation.x(), mtx.translation.y(), mtx.translation.z()});
	}

	rawrbox::Matrix4x4 GLTFImporter::toMatrix(const fastgltf::math::fmat4x4& mtx) {
		return rawrbox::Matrix4x4(mtx.data());
	}
	// ----------

	// OPTIMIZATION ---
	void GLTFImporter::optimizeMesh(std::vector<rawrbox::VertexNormBoneData>& verts, std::vector<uint32_t>& indices) {
		size_t indiceSize = indices.size();
		std::vector<uint32_t> remap(indiceSize);

		// Calculate vertex map ---
		size_t newVerticeCount = meshopt_generateVertexRemap(&remap[0], &indices[0], indiceSize, &verts[0], indiceSize, sizeof(rawrbox::VertexNormBoneData));
		// ------

		// Optimize index & vertex buffers ---
		std::vector<uint32_t> resultIndices(indiceSize);
		std::vector<rawrbox::VertexNormBoneData> resultVertices(newVerticeCount);

		meshopt_remapIndexBuffer(&resultIndices[0], &indices[0], indiceSize, &remap[0]);
		meshopt_remapVertexBuffer(&resultVertices[0], &verts[0], indiceSize, sizeof(rawrbox::VertexNormBoneData), &remap[0]);
		// -------------

		// Vertex cache
		meshopt_optimizeVertexCache(&resultIndices[0], &resultIndices[0], indiceSize, newVerticeCount);

		// Overdraw
		meshopt_optimizeOverdraw(&resultIndices[0], &resultIndices[0], indiceSize, &resultVertices[0].position.x, newVerticeCount, sizeof(rawrbox::VertexNormBoneData), 1.05f);

		// Vertex fetch
		meshopt_optimizeVertexFetch(&resultVertices[0], &resultIndices[0], indiceSize, &resultVertices[0], newVerticeCount, sizeof(rawrbox::VertexNormBoneData));

		verts.swap(resultVertices);
		indices.swap(resultIndices);
	}

	void GLTFImporter::simplifyMesh(std::vector<rawrbox::VertexNormBoneData>& verts, std::vector<uint32_t>& indices, float complexity_threshold) {
		const size_t index_count = indices.size();
		const size_t vertex_count = verts.size();

		const size_t target_index_count = index_count * complexity_threshold;
		constexpr float target_error = 1e-2f;
		constexpr unsigned int options = 0;

		std::vector<uint32_t> lod_indices(index_count);
		float lod_error = 0.0f;

		lod_indices.resize(meshopt_simplify(lod_indices.data(), indices.data(), index_count,
		    reinterpret_cast<const float*>(verts.data()),
		    vertex_count, sizeof(rawrbox::VertexNormBoneData), target_index_count,
		    target_error, options, &lod_error));

		indices.swap(lod_indices);
	}

	// -----------
	// -------------

	// PUBLIC -------
	GLTFImporter::GLTFImporter(uint32_t loadFlags) : loadFlags(loadFlags) {}
	GLTFImporter::~GLTFImporter() {
		this->_logger.reset();

		this->meshes.clear(); // Clear old meshes

		this->textures.clear();     // Clear old textures
		this->_texturesMap.clear(); // Clear old textures

		this->vertexAnimation.clear();
		this->animations.clear();
		this->skeletons.clear();

		this->materials.clear(); // Clear old materials
	}

	void GLTFImporter::load(const std::filesystem::path& path, const std::vector<uint8_t>& buffer) {
		this->filePath = path;

		auto b = buffer;
		if (!b.empty()) {
			const auto* bah = std::bit_cast<const std::byte*>(b.data());

			if (path.extension() == ".gltf") {
				this->load(path); // GLTF has external dependencies, not sure how to load them using file from memory
			} else {
				auto data = fastgltf::GltfDataBuffer::FromBytes(bah, static_cast<uint32_t>(b.size()));
				if (data.error() != fastgltf::Error::None) {
					this->_logger->warn("Failed to load '{}' ──> {}\n  └── Loading fallback model!", this->filePath.generic_string(), fastgltf::getErrorMessage(data.error()));
					return;
				}

				this->internalLoad(data.get());
			}
		} else {
			this->load(path);
		}
	}

	void GLTFImporter::load(const std::filesystem::path& path) {
		if (!std::filesystem::exists(path)) throw this->_logger->error("File '{}' does not exist!", path.generic_string());

		this->filePath = path;

		auto data = fastgltf::GltfDataBuffer::FromPath(path);
		if (data.error() != fastgltf::Error::None) {
			this->_logger->warn("Failed to load '{}' ──> {}\n  └── Loading fallback model!", this->filePath.generic_string(), fastgltf::getErrorMessage(data.error()));
			return;
		}

		this->internalLoad(data.get());
	}
	// ----------------

} // namespace rawrbox
