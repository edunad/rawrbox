#include <rawrbox/gltf/importer.hpp>
#include <rawrbox/render/models/utils/optimization.hpp>
#include <rawrbox/render/textures/webp.hpp>
#include <rawrbox/utils/file.hpp>

#include <fastgltf/tools.hpp>
#include <fastgltf/types.hpp>
#include <magic_enum/magic_enum.hpp>

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

		if ((this->loadFlags & rawrbox::GLTFLoadFlags::IMPORT_TEXTURES) > 0) {
			extensions |= fastgltf::Extensions::KHR_materials_unlit |
				      fastgltf::Extensions::KHR_materials_specular | fastgltf::Extensions::KHR_texture_basisu |
				      fastgltf::Extensions::EXT_texture_webp | fastgltf::Extensions::KHR_materials_emissive_strength;

			gltfOptions |= fastgltf::Options::LoadExternalImages; // Handle loading for us
		}

		if ((this->loadFlags & rawrbox::GLTFLoadFlags::IMPORT_LIGHT) > 0) {
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
				if ((importer->loadFlags & rawrbox::GLTFLoadFlags::IMPORT_BLEND_SHAPES) > 0) {
					auto arr = extras->at_key("targetNames").get_array();
					if (arr.error() != simdjson::error_code::SUCCESS) return;

					for (auto target : arr) {
						auto targetName = std::string(target.get_string().value());
						if (targetName.empty()) targetName = fmt::format("blendshape_{}", importer->targetNames.size());

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

		// LOAD MATERIALS ---
		if ((this->loadFlags & rawrbox::GLTFLoadFlags::IMPORT_TEXTURES) > 0) {
			this->loadTextures(scene);
			this->loadMaterials(scene);
		}
		// --------------

		// LOAD SCENE ---
		this->loadScene(scene);
		//  ---------------

		// LOAD SKELETONS & ANIMATIONS ---
		if ((this->loadFlags & rawrbox::GLTFLoadFlags::IMPORT_ANIMATIONS) > 0) {
			this->loadSkeletons(scene);
			this->loadAnimations(scene);
		}
		// -------------------
	}

	// POST-LOAD ---
	void GLTFImporter::postLoadFixSceneNames(fastgltf::Asset& scene) {
		const bool importAnims = (this->loadFlags & rawrbox::GLTFLoadFlags::IMPORT_ANIMATIONS) > 0;
		const bool importTextures = (this->loadFlags & rawrbox::GLTFLoadFlags::IMPORT_TEXTURES) > 0;
		const bool importLights = (this->loadFlags & rawrbox::GLTFLoadFlags::IMPORT_LIGHT) > 0;

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
			mat->alphaCutoff = material.alphaCutoff;
			mat->transparent = material.alphaMode != fastgltf::AlphaMode::Opaque;
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
	void GLTFImporter::loadSkeletons(const fastgltf::Asset& scene) {
		if (scene.skins.empty()) return;
		ozz::animation::offline::SkeletonBuilder builder;

		// UTILS -------

		// GENERATE CHILDREN BONES ---
		std::function<void(const fastgltf::Node& node, ozz::animation::offline::RawSkeleton::Joint& parent)> generateBones;
		generateBones = [this, &scene, &generateBones](const fastgltf::Node& node, ozz::animation::offline::RawSkeleton::Joint& parent) {
			for (const auto& childIndex : node.children) {
				const auto& childNode = scene.nodes[childIndex];
				const std::string jointName = std::string(childNode.name);

				ozz::animation::offline::RawSkeleton::Joint childJoint = {};
				childJoint.name = jointName;

				// REGISTER JOINT ---
				this->joints[jointName] = std::make_unique<rawrbox::GLTFJoint>(this->joints.size(), childNode);
				// ------------------

				generateBones(childNode, childJoint);
				parent.children.push_back(childJoint);
			}
		};
		// -----------------------

		// DEBUG ----
		std::function<void(const ozz::animation::offline::RawSkeleton::Joint&, int, bool)> printBone;
		if ((this->loadFlags & rawrbox::GLTFLoadFlags::Debug::PRINT_BONE_STRUCTURE) > 0) {
			printBone = [this, &printBone](const ozz::animation::offline::RawSkeleton::Joint& bn, int depth, bool isLast) -> void {
				std::string indent(depth * 4, ' ');
				std::string branch = isLast ? "└── " : "├── ";
				this->_logger->debug("{}{}{}", indent, branch, bn.name);

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

			for (const auto& rootJoint : skin.joints) {
				auto& root = scene.nodes[rootJoint];
				const std::string rootName = std::string(root.name);

				if (this->joints.contains(rootName)) continue; // This is the only way to determine root joints

				ozz::animation::offline::RawSkeleton::Joint rootBone;
				rootBone.name = rootName;

				// REGISTER JOINT ---
				this->joints[rootName] = std::make_unique<rawrbox::GLTFJoint>(this->joints.size(), root);
				// ------------------

				if (!root.children.empty()) generateBones(root, rootBone);

				rawSkeleton.roots.push_back(rootBone);
			}
			// ----------------------

			// DEBUG ----
			if (printBone != nullptr) {
				this->_logger->debug("Found skeleton '{}' ->", fmt::styled(skin.name, fmt::fg(fmt::color::green_yellow)));
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
				const auto& node = scene.nodes[joint];
				const std::string jointName = std::string(node.name);

				auto fnd = this->joints.find(jointName);
				if (fnd == this->joints.end()) RAWRBOX_CRITICAL("Invalid joint '{}', could not find in skeleton", jointName);

				fnd->second->skeleton = this->skeletons[i].get();
			}
			// --------------
		}

		// Apply skins on meshes --
		for (size_t i = 0; i < scene.nodes.size(); i++) {
			const auto& node = scene.nodes[i];
			if (!node.skinIndex) continue;

			size_t skinIndex = node.skinIndex.value();
			if (skinIndex >= this->skeletons.size()) {
				this->_logger->warn("Invalid skeleton index '{}' on mesh {}", skinIndex, i);
				continue;
			}

			this->meshes[node.meshIndex.value()]->skeleton = this->skeletons[skinIndex].get();
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

			if ((this->loadFlags & rawrbox::GLTFLoadFlags::Debug::PRINT_ANIMATIONS) > 0) {
				this->_logger->debug("Found animation '{}'", fmt::styled(gltfAnim.name, fmt::fg(fmt::color::green_yellow)));
			}

			for (size_t c = 0; c < anim.channels.size(); c++) {
				const auto& channel = anim.channels[c];

				if (!channel.nodeIndex) continue;
				const size_t nodeIndex = channel.nodeIndex.value();

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

				const auto& node = scene.nodes[nodeIndex]; // Node being affected

				if (node.meshIndex) {                                            // Vertex animation
					const auto& mesh = this->meshes[node.meshIndex.value()]; // Mesh being affected
					this->vertexAnimation[i].insert(mesh.get());
				} else {
					auto fnd = this->joints.find(std::string(node.name));

					if (fnd != this->joints.end()) {
						if (gltfAnim.skeleton != nullptr && gltfAnim.skeleton != fnd->second->skeleton) {
							this->_logger->warn("Animation '{}' contains 2 or more skeletons, this is not supported! Please split the animation per skeleton", gltfAnim.name);
							continue;
						} else {
							gltfAnim.skeleton = fnd->second->skeleton;
						}
					}
				}
				// ----------------

				// TIME ----
				auto nodeName = std::string(node.name);
				auto& track = gltfAnim.tracks[nodeName];

				for (size_t iTime = 0; iTime < timeAccessor.count; iTime++) {
					float t = fastgltf::getAccessorElement<float>(scene, timeAccessor, iTime);
					if (t > gltfAnim.duration) gltfAnim.duration = t; // Calculate the total animation time

					switch (channel.path) {
						case fastgltf::AnimationPath::Translation:
							{
								ozz::math::Float3 key = fastgltf::getAccessorElement<ozz::math::Float3>(scene, dataAccessor, iTime);
								if (gltfAnim.skeleton == nullptr) key.z = -key.z; // Convert to left-hand coordinate system

								track.translations.emplace_back(t, key);
								break;
							}
						case fastgltf::AnimationPath::Rotation:
							{
								ozz::math::Quaternion key = fastgltf::getAccessorElement<ozz::math::Quaternion>(scene, dataAccessor, iTime);
								if (gltfAnim.skeleton == nullptr) { // Convert to left-hand coordinate system
									key.x = -key.x;
									key.y = -key.y;
								}

								track.rotations.emplace_back(t, key);
								break;
							}
						case fastgltf::AnimationPath::Scale:
							{
								ozz::math::Float3 key = fastgltf::getAccessorElement<ozz::math::Float3>(scene, dataAccessor, iTime);
								if (gltfAnim.skeleton == nullptr) key.z = -key.z; // Convert to left-hand coordinate system

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

	void GLTFImporter::parseAnimations() { // I don't like this extra step, but OZZ requires tracks to have all bones and be in order..
		if (this->_parsedAnimations.empty()) return;
		ozz::animation::offline::AnimationBuilder builder;

		this->_logger->debug("Building {} animations...", this->_parsedAnimations.size());

		this->animations.resize(this->_parsedAnimations.size());
		for (size_t i = 0; i < this->_parsedAnimations.size(); i++) {
			auto& anim = this->_parsedAnimations[i];

			ozz::animation::offline::RawAnimation rawrAnim;
			rawrAnim.duration = anim.duration;
			rawrAnim.name = anim.name;

			if (anim.skeleton != nullptr) {
				for (std::string bone : anim.skeleton->joint_names()) {
					rawrAnim.tracks.emplace_back(anim.tracks[bone]); // Ensure all joints have a track and it's ordered based on skeleton
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
			if ((this->loadFlags & rawrbox::GLTFLoadFlags::Optimizer::SKELETON_ANIMATIONS) > 0 && anim.skeleton != nullptr) {
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
				this->loadNodes(scene, scene.nodes[nodeIndex]);
			}
		}
	}

	void GLTFImporter::loadNodes(const fastgltf::Asset& scene, const fastgltf::Node& node) {
		if (node.lightIndex) {
			this->lights.push_back(std::make_unique<rawrbox::GLTFLight>(this->lights.size(), node, scene.lights[node.lightIndex.value()]));
		} else if (node.meshIndex) {
			auto mesh = this->extractMesh(scene, node);
			if (mesh != nullptr) this->meshes.push_back(std::move(mesh));
		}

		// Children ---
		for (const auto& children : node.children) {
			this->loadNodes(scene, scene.nodes[children]);
		}
		// ---
	}

	std::unique_ptr<rawrbox::GLTFMesh> GLTFImporter::extractMesh(const fastgltf::Asset& scene, const fastgltf::Node& node) {
		auto gltfMesh = std::make_unique<rawrbox::GLTFMesh>(this->meshes.size(), node);

		size_t meshIndex = node.meshIndex.value();
		const auto& mesh = scene.meshes[meshIndex];

		const bool importBlendShapes = (this->loadFlags & rawrbox::GLTFLoadFlags::IMPORT_BLEND_SHAPES) > 0;

		// SUB-MESHES ----
		gltfMesh->primitives.resize(mesh.primitives.size());

		for (size_t i = 0; i < mesh.primitives.size(); i++) {
			const auto& primitive = mesh.primitives[i];
			if (primitive.type != fastgltf::PrimitiveType::Triangles) {
				this->_logger->warn("Primitive type '{}' not supported", magic_enum::enum_name(primitive.type));
				continue;
			}

			rawrbox::GLTFPrimitive& rawrPrimitive = gltfMesh->primitives[i];
			rawrPrimitive.material = primitive.materialIndex.has_value() ? this->materials[primitive.materialIndex.value()].get() : nullptr;

			// BLEND SHAPES --
			if (importBlendShapes && !primitive.targets.empty()) {
				rawrPrimitive.blendShapes.resize(primitive.targets.size());

				for (size_t o = 0; o < primitive.targets.size(); o++) {
					const auto& blendNames = this->targetNames[meshIndex];
					if (blendNames.empty()) RAWRBOX_CRITICAL("Invalid blend shape names for mesh '{}'", gltfMesh->name);

					rawrbox::GLTFBlendShape& shape = rawrPrimitive.blendShapes[o];
					shape.name = fmt::format("{}-{}", gltfMesh->name, blendNames[o]);
					shape.weight = mesh.weights[o]; // Default weight

					// POSITION ---
					const auto* positionTarget = primitive.findTargetAttribute(o, "POSITION");
					if (positionTarget != nullptr) {
						const auto& positionAccessor = scene.accessors[positionTarget->accessorIndex];

						shape.pos.resize(positionAccessor.count);
						fastgltf::iterateAccessorWithIndex<fastgltf::math::fvec3>(scene, positionAccessor, [&](fastgltf::math::fvec3 pos, size_t index) {
							shape.pos[index] = rawrbox::Vector3f(pos.x(), pos.y(), pos.z());
						});
					}
					// ----------------

					// NORMAL ---
					const auto* normalTarget = primitive.findTargetAttribute(o, "NORMAL");
					if (normalTarget != nullptr) {
						const auto& normAccessor = scene.accessors[normalTarget->accessorIndex];

						shape.norms.resize(normAccessor.count);
						fastgltf::iterateAccessorWithIndex<fastgltf::math::fvec3>(scene, normAccessor, [&](fastgltf::math::fvec3 norm, size_t index) {
							shape.norms[index] = rawrbox::Vector4f(norm.x(), norm.y(), norm.z(), 0.0F);
						});
					}
					// ----------------
				}
			}

			// ---------------

			// VERTICES ---
			rawrPrimitive.vertices = this->extractVertex(scene, primitive);
			rawrPrimitive.indices = this->extractIndices(scene, primitive);
			// -----------

			// OPTIMIZATION ---
			if ((this->loadFlags & rawrbox::GLTFLoadFlags::Optimizer::MESH) > 0) {
				if (rawrPrimitive.blendShapes.empty()) {
					auto startVert = rawrPrimitive.vertices.size();
					auto startInd = rawrPrimitive.indices.size();

					rawrbox::MeshOptimization::optimize(rawrPrimitive.vertices, rawrPrimitive.indices);
					rawrbox::MeshOptimization::simplify(rawrPrimitive.vertices, rawrPrimitive.indices);

					if ((this->loadFlags & rawrbox::GLTFLoadFlags::Debug::PRINT_OPTIMIZATION_STATS) > 0) {
						if (startVert != rawrPrimitive.vertices.size() || startInd != rawrPrimitive.indices.size()) {
							this->_logger->debug("Optimized mesh '{}'\n\tVertices -> {} to {}\n\tIndices -> {} to {}", fmt::styled(gltfMesh->name, fmt::fg(fmt::color::cyan)), startVert, rawrPrimitive.vertices.size(), startInd, rawrPrimitive.indices.size());
						}
					}
				} else {
					this->_logger->warn("Mesh '{}' has blend shapes, optimization is not supported!", gltfMesh->name);
				}
			}
			// ----------------

			// BBOX CALCULATION --
			if ((this->loadFlags & rawrbox::GLTFLoadFlags::CALCULATE_BBOX) > 0) {
				gltfMesh->bbox.min = rawrbox::Vector3f(std::numeric_limits<float>::max());
				gltfMesh->bbox.max = rawrbox::Vector3f(std::numeric_limits<float>::min());

				for (const auto& vertex : rawrPrimitive.vertices) {
					gltfMesh->bbox.min = gltfMesh->bbox.min.min(vertex.position);
					gltfMesh->bbox.max = gltfMesh->bbox.max.max(vertex.position);
				}

				gltfMesh->bbox.size = gltfMesh->bbox.max - gltfMesh->bbox.min;
			}
			// -------------
		}
		// -------------------

		return gltfMesh;
	}

	std::vector<rawrbox::VertexNormBoneData> GLTFImporter::extractVertex(const fastgltf::Asset& scene, const fastgltf::Primitive& primitive) {
		std::vector<rawrbox::VertexNormBoneData> verts = {};

		// POSITION ----
		const auto* positionAttribute = primitive.findAttribute("POSITION");
		if (positionAttribute == nullptr) RAWRBOX_CRITICAL("Invalid gltf model, missing 'POSITION' attribute!"); // All models have POSITION

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

			fastgltf::iterateAccessorWithIndex<fastgltf::math::fvec4>(
			    scene, tangentAccessor,
			    [&](const fastgltf::math::fvec4& tangent, std::size_t idx) {
				    verts[idx].tangent = rawrbox::PackUtils::packNormal(tangent.x(), tangent.y(), tangent.z(), tangent.w() > 0.0F ? 1.0F : -1.0F);
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
		if ((this->loadFlags & rawrbox::GLTFLoadFlags::IMPORT_ANIMATIONS) > 0) {
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

		// Invert the winding order for left-handed coordinate system
		for (size_t i = 0; i < indices.size(); i += 3) {
			std::swap(indices[i + 1], indices[i + 2]);
		}
		// ----------

		return indices;
	}
	// ----------

	// UTILS ---
	fastgltf::sources::ByteView GLTFImporter::getSourceData(const fastgltf::Asset& scene, const fastgltf::DataSource& source) {
		return std::visit(fastgltf::visitor{
				      [&](auto& /*arg*/) -> fastgltf::sources::ByteView {
					      RAWRBOX_CRITICAL("Invalid data");
				      },
				      [&](std::monostate) -> fastgltf::sources::ByteView {
					      RAWRBOX_CRITICAL("Invalid data");
				      },
				      [&](fastgltf::sources::Fallback) -> fastgltf::sources::ByteView {
					      RAWRBOX_CRITICAL("Invalid data");
				      },
				      [&](const fastgltf::sources::BufferView& buffer_view) -> fastgltf::sources::ByteView {
					      const fastgltf::BufferView& view = scene.bufferViews.at(buffer_view.bufferViewIndex);
					      const fastgltf::Buffer& buffer = scene.buffers.at(view.bufferIndex);

					      auto data = this->getSourceData(scene, buffer.data);
					      return {subspan(data.bytes, view.byteOffset, view.byteLength), buffer_view.mimeType};
				      },
				      [&](const fastgltf::sources::URI& /*filePath*/) -> fastgltf::sources::ByteView {
					      RAWRBOX_CRITICAL("Use fastgltf::Options::LoadExternalImages instead!");
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
					      RAWRBOX_CRITICAL("Invalid data");
				      },
				      [&](fastgltf::sources::ByteView& byte_view) -> fastgltf::sources::ByteView {
					      return {byte_view.bytes, byte_view.mimeType};
				      }},
		    source);
	}
	// ----------
	// -------------

	// PUBLIC -------
	GLTFImporter::GLTFImporter(uint32_t loadFlags) : loadFlags(loadFlags) {}
	GLTFImporter::~GLTFImporter() {
		this->_logger.reset();

		this->meshes.clear(); // Clear old meshes
		this->joints.clear(); // Clear old joints
		this->lights.clear(); // Clear old lights

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
		if (!std::filesystem::exists(path)) RAWRBOX_CRITICAL("File '{}' does not exist!", path.generic_string());

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
