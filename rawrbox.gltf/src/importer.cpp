#include <rawrbox/gltf/importer.hpp>
#include <rawrbox/render/models/utils/optimization.hpp>
#include <rawrbox/render/textures/webp.hpp>
#include <rawrbox/utils/file.hpp>

#include <fastgltf/tools.hpp>
#include <fastgltf/types.hpp>
#include <magic_enum/magic_enum.hpp>

#include <ozz/animation/offline/animation_builder.h>
#include <ozz/animation/offline/animation_optimizer.h>
#include <ozz/animation/offline/skeleton_builder.h>
#include <simdjson.h>

#include <algorithm>
#include <cmath>
#include <variant>

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
		if (!this->buildHierarchy(scene)) {
			this->_logger->warn("Invalid model node hierarchy!");
			return;
		}
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
		std::unordered_set<std::string> nodeNames = {};
		for (size_t i = 0; i < scene.nodes.size(); i++) {
			auto& node = scene.nodes[i];

			std::string name = node.name.empty() ? fmt::format("node_{}", i) : std::string(node.name);
			if (nodeNames.contains(name)) {
				name = fmt::format("{}_{}", name, i);
				this->_logger->warn("Duplicate node name '{}', this is not supported!, renaming to '{}'", node.name, name);
			}

			node.name = name;
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

	bool GLTFImporter::buildHierarchy(const fastgltf::Asset& scene) {
		this->_nodeParents.assign(scene.nodes.size(), std::nullopt);

		for (size_t i = 0; i < scene.nodes.size(); i++) {
			for (const auto& child : scene.nodes[i].children) {
				if (child >= scene.nodes.size()) {
					this->_logger->warn("Node '{}' has an invalid child '{}'", i, child);
					return false;
				}

				if (this->_nodeParents[child].has_value()) {
					this->_logger->warn("Node '{}' has multiple parents. This is not supported!", child, this->_nodeParents[child].value(), i);
					return false;
				}

				this->_nodeParents[child] = i;
			}
		}

		return true;
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
			switch (material.alphaMode) {
				case fastgltf::AlphaMode::Blend:
					mat->alphaCutoff = 0.0039F;
					break;
				case fastgltf::AlphaMode::Mask:
				default: // OPAQUE
					mat->alphaCutoff = material.alphaCutoff;
					break;
			}

			mat->transparent = material.alphaMode == fastgltf::AlphaMode::Blend;
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

			mat->baseColor = rawrbox::Colorf(material.pbrData.baseColorFactor.x(), material.pbrData.baseColorFactor.y(), material.pbrData.baseColorFactor.z(), alpha).toSRGB();
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

		const bool printBones = (this->loadFlags & rawrbox::GLTFLoadFlags::Debug::PRINT_BONE_STRUCTURE) > 0;

		ozz::animation::offline::SkeletonBuilder builder;
		this->_skinSkeletons.assign(scene.skins.size(), nullptr);

		for (size_t i = 0; i < scene.skins.size(); i++) {
			const auto& skin = scene.skins[i];
			const std::string skinName = std::string(skin.name);

			if (skin.joints.empty()) {
				this->_logger->warn("Skin '{}' has no joints, skipping...", skinName);
				continue;
			}

			if (!skin.inverseBindMatrices.has_value()) {
				this->_logger->warn("Skin '{}' has no inverse bind matrices, skipping...", skinName);
				continue;
			}

			// INVERSE BIND MATRICES ----
			if (!this->isValid(scene, skin.inverseBindMatrices.value(), fastgltf::AccessorType::Mat4)) {
				this->_logger->warn("Skin '{}' has an matrices, skipping...", skinName);
				continue;
			}

			const auto& inverseAccessor = scene.accessors[skin.inverseBindMatrices.value()];
			if (inverseAccessor.count < skin.joints.size()) {
				this->_logger->warn("Skin '{}' joints {} do not match {} inverse bind matrices, skipping...", skinName, skin.joints.size(), inverseAccessor.count);
				continue;
			}

			std::vector<rawrbox::Matrix4x4> inverseBindMatrices(skin.joints.size());
			fastgltf::iterateAccessorWithIndex<fastgltf::math::fmat4x4>(scene, inverseAccessor, [&inverseBindMatrices](const fastgltf::math::fmat4x4& mtx, size_t index) {
				if (index >= inverseBindMatrices.size()) return;
				inverseBindMatrices[index] = rawrbox::Matrix4x4(mtx.data());
			});
			// ---------------------------

			// SHARED SKINS (optimize same skins)  ----
			rawrbox::Skeleton* shared = nullptr;
			for (size_t o = 0; o < i && shared == nullptr; o++) {
				rawrbox::Skeleton* other = this->_skinSkeletons[o];
				if (other == nullptr) continue;

				const auto& otherSkin = scene.skins[o];
				if (otherSkin.joints.size() != skin.joints.size() || !std::equal(otherSkin.joints.begin(), otherSkin.joints.end(), skin.joints.begin())) continue;

				bool sameBind = true;
				for (size_t k = 0; k < inverseBindMatrices.size() && sameBind; k++) {
					sameBind = other->getInverseBindMatrix(k) == inverseBindMatrices[k];
				}

				if (sameBind) shared = other;
			}

			if (shared != nullptr) {
				this->_skinSkeletons[i] = shared;
				continue;
			}
			// -----------------------------------

			// Ozz convertion ----
			if (skin.joints.size() > RB_RENDER_MAX_BONES_PER_MODEL) {
				this->_logger->warn("Skin '{}' has {} joints, max is {}, skipping...", skinName, skin.joints.size(), RB_RENDER_MAX_BONES_PER_MODEL);
				continue;
			}

			bool validJoints = true;
			std::unordered_set<size_t> nodes = {};

			for (const auto& joint : skin.joints) {
				if (joint >= scene.nodes.size()) {
					this->_logger->warn("Skin '{}' has invalid joint node {}, skipping...", skinName, joint);
					validJoints = false;
					break;
				}

				std::optional<size_t> current = joint;
				while (current.has_value() && nodes.insert(current.value()).second) {
					current = this->_nodeParents[current.value()];
				}
			}

			if (!validJoints) continue;

			std::vector<size_t> roots = {};
			for (size_t n = 0; n < scene.nodes.size(); n++) {
				if (!nodes.contains(n) || this->_nodeParents[n].has_value()) continue;
				roots.push_back(n);
			}
			// ----------------------------

			// BUILD ----
			ozz::animation::offline::RawSkeleton rawSkeleton = {};
			std::vector<size_t> jointNodes = {};

			for (const auto& root : roots) {
				rawSkeleton.roots.push_back(this->buildJoint(scene, root, nodes, jointNodes));
			}

			if (printBones) {
				this->_logger->debug("Found skeleton '{}' ->", fmt::styled(skinName, fmt::fg(fmt::color::green_yellow)));
				for (size_t r = 0; r < rawSkeleton.roots.size(); r++) {
					this->printJoint(rawSkeleton.roots[r], 0, r == rawSkeleton.roots.size() - 1);
				}
			}

			auto ozzSkeleton = builder(rawSkeleton);
			if (ozzSkeleton == nullptr) {
				this->_logger->warn("Failed to build skeleton '{}'", skinName);
				continue;
			}

			// Validation ---
			if (static_cast<size_t>(ozzSkeleton->num_joints()) != jointNodes.size()) {
				this->_logger->warn("Skeleton '{}' joint count mismatch ({} != {})", skinName, ozzSkeleton->num_joints(), jointNodes.size());
				continue;
			}

			bool validOrder = true;
			const auto jointNames = ozzSkeleton->joint_names();
			for (size_t j = 0; j < jointNodes.size(); j++) {
				if (scene.nodes[jointNodes[j]].name == std::string_view(jointNames[j])) continue;

				this->_logger->warn("Skeleton '{}' joint order mismatch at {} ('{}' != '{}'), skipping...", skinName, j, scene.nodes[jointNodes[j]].name, jointNames[j]);
				validOrder = false;
				break;
			}

			if (!validOrder) continue;
			// -----------

			// REMAP ----
			bool validRemap = true;
			std::vector<int> jointRemap(skin.joints.size());

			for (size_t k = 0; k < skin.joints.size(); k++) {
				auto fnd = std::ranges::find(jointNodes, skin.joints[k]);
				if (fnd == jointNodes.end()) {
					this->_logger->warn("Skin '{}' joint node '{}' not found in the skeleton, skipping...", skinName, skin.joints[k]);
					validRemap = false;
					break;
				}

				jointRemap[k] = static_cast<int>(std::distance(jointNodes.begin(), fnd));
			}

			if (!validRemap) continue;
			// -----------------------------------------------------------------------

			auto skeleton = std::make_unique<rawrbox::Skeleton>(skinName, std::move(ozzSkeleton), std::move(inverseBindMatrices), std::move(jointRemap));
			rawrbox::Skeleton* ptr = skeleton.get();

			for (const auto& node : jointNodes) {
				this->_nodeSkeletons[node].push_back(ptr);
			}

			this->_skeletonJoints[ptr] = std::move(jointNodes);
			this->_skinSkeletons[i] = ptr;

			this->skeletons.push_back(std::move(skeleton));
		}

		// APPLY SKINS ON MESHES ----
		for (size_t i = 0; i < scene.nodes.size(); i++) {
			const auto& node = scene.nodes[i];
			if (!node.skinIndex.has_value()) continue;

			auto fnd = this->_nodeMeshes.find(i);
			if (fnd == this->_nodeMeshes.end()) continue; // Invalid mesh / skipped

			const size_t skinIndex = node.skinIndex.value();
			if (skinIndex >= this->_skinSkeletons.size() || this->_skinSkeletons[skinIndex] == nullptr) {
				this->_logger->warn("Invalid skin index '{}' on mesh '{}'", skinIndex, fnd->second->name);
				continue;
			}

			fnd->second->skeleton = this->_skinSkeletons[skinIndex];
			fnd->second->matrix = rawrbox::Matrix4x4::mtxLeftHand(rawrbox::Matrix4x4());
		}
		// --------------
	}

	ozz::animation::offline::RawSkeleton::Joint GLTFImporter::buildJoint(const fastgltf::Asset& scene, size_t nodeIndex, const std::unordered_set<size_t>& nodes, std::vector<size_t>& jointNodes) {
		const auto& node = scene.nodes[nodeIndex];

		ozz::animation::offline::RawSkeleton::Joint joint = {};
		joint.name = std::string(node.name).c_str();
		joint.transform = this->getRestPose(node);

		jointNodes.push_back(nodeIndex); // Ozz joint order

		for (const auto& child : node.children) {
			if (!nodes.contains(child)) continue; // Invalid joint
			joint.children.push_back(this->buildJoint(scene, child, nodes, jointNodes));
		}

		return joint;
	}

	void GLTFImporter::printJoint(const ozz::animation::offline::RawSkeleton::Joint& joint, int depth, bool isLast) {
		const std::string indent(static_cast<size_t>(depth) * 4, ' ');
		const std::string branch = isLast ? "└── " : "├── ";
		this->_logger->debug("{}{}{}", indent, branch, joint.name.c_str());

		for (size_t i = 0; i < joint.children.size(); i++) {
			this->printJoint(joint.children[i], depth + 1, i == joint.children.size() - 1);
		}
	}

	ozz::math::Transform GLTFImporter::getRestPose(const fastgltf::Node& node) const {
		const auto& trs = std::get<fastgltf::TRS>(node.transform); // fastgltf::Options::DecomposeNodeMatrices

		ozz::math::Transform transform = ozz::math::Transform::identity();
		transform.translation = ozz::math::Float3(trs.translation.x(), trs.translation.y(), trs.translation.z());
		transform.rotation = ozz::math::Quaternion(trs.rotation.x(), trs.rotation.y(), trs.rotation.z(), trs.rotation.w());
		transform.scale = ozz::math::Float3(trs.scale.x(), trs.scale.y(), trs.scale.z());

		return transform;
	}
	// ------------

	// ANIMATIONS ---
	void GLTFImporter::loadAnimations(const fastgltf::Asset& scene) {
		if (scene.animations.empty()) return;

		const bool printAnims = (this->loadFlags & rawrbox::GLTFLoadFlags::Debug::PRINT_ANIMATIONS) > 0;

		this->_parsedAnimations.reserve(scene.animations.size());
		for (size_t i = 0; i < scene.animations.size(); i++) {
			const auto& anim = scene.animations[i];
			if (anim.channels.empty()) {
				this->_logger->warn("Animation '{}' has no channels, skipping...", anim.name);
				continue;
			}

			rawrbox::GLTFAnimation gltfAnim = {};
			gltfAnim.name = std::string(anim.name);

			if (printAnims) {
				this->_logger->debug("Found animation '{}'", fmt::styled(gltfAnim.name, fmt::fg(fmt::color::green_yellow)));
			}

			// VALIDATE ---
			const auto validChannel = [this, &scene, &anim](const fastgltf::AnimationChannel& channel) {
				if (channel.samplerIndex >= anim.samplers.size()) return false;

				const auto& sampler = anim.samplers[channel.samplerIndex];
				if (!this->isValid(scene, sampler.inputAccessor, fastgltf::AccessorType::Scalar)) return false;

				switch (channel.path) {
					case fastgltf::AnimationPath::Translation:
					case fastgltf::AnimationPath::Scale:
						return this->isValid(scene, sampler.outputAccessor, fastgltf::AccessorType::Vec3);
					case fastgltf::AnimationPath::Rotation:
						return this->isValid(scene, sampler.outputAccessor, fastgltf::AccessorType::Vec4);
					default:
						return sampler.outputAccessor < scene.accessors.size();
				}
			};
			// ------------

			// DURATION ---
			for (const auto& channel : anim.channels) {
				if (!validChannel(channel)) continue;

				const auto& timeAccessor = scene.accessors[anim.samplers[channel.samplerIndex].inputAccessor];
				fastgltf::iterateAccessor<float>(scene, timeAccessor, [&gltfAnim](float time) { gltfAnim.duration = std::max(gltfAnim.duration, time); });
			}

			gltfAnim.duration = std::max(gltfAnim.duration, 0.001F); // ozz duration cannot be 0
			const float holdOffset = gltfAnim.duration * 0.0001F;
			// ----------------------------------------

			for (const auto& channel : anim.channels) {
				if (!channel.nodeIndex.has_value()) continue;

				const size_t nodeIndex = channel.nodeIndex.value();
				if (nodeIndex >= scene.nodes.size()) {
					this->_logger->warn("Animation '{}' targets invalid node {}", gltfAnim.name, nodeIndex);
					continue;
				}

				if (channel.path == fastgltf::AnimationPath::Weights) {
					this->_logger->warn("Unsupported channel path 'Weights' for animation '{}'", gltfAnim.name); // TODO: SUPPORT BLEND SHAPES
					continue;
				}

				if (!validChannel(channel)) {
					this->_logger->warn("Animation '{}' has a channel with an invalid sampler or accessor, skipping channel...", gltfAnim.name);
					continue;
				}

				const auto& sampler = anim.samplers[channel.samplerIndex];
				const auto& timeAccessor = scene.accessors[sampler.inputAccessor];
				const auto& dataAccessor = scene.accessors[sampler.outputAccessor];

				auto& track = gltfAnim.tracks[nodeIndex];
				switch (channel.path) {
					case fastgltf::AnimationPath::Translation:
						this->extractKeys<ozz::math::Float3>(scene, timeAccessor, dataAccessor, sampler.interpolation, holdOffset, track.translations, gltfAnim.name);
						break;
					case fastgltf::AnimationPath::Rotation:
						this->extractKeys<ozz::math::Quaternion>(scene, timeAccessor, dataAccessor, sampler.interpolation, holdOffset, track.rotations, gltfAnim.name);
						break;
					case fastgltf::AnimationPath::Scale:
						this->extractKeys<ozz::math::Float3>(scene, timeAccessor, dataAccessor, sampler.interpolation, holdOffset, track.scales, gltfAnim.name);
						break;
					default:
						break;
				}
			}

			this->_parsedAnimations.push_back(std::move(gltfAnim));
		}

		this->buildAnimations(scene);
	}

	void GLTFImporter::buildAnimations(const fastgltf::Asset& scene) {
		if (this->_parsedAnimations.empty()) return;
		ozz::animation::offline::AnimationBuilder builder;

		const bool optimize = (this->loadFlags & rawrbox::GLTFLoadFlags::Optimizer::SKELETON_ANIMATIONS) > 0;
		const bool printAnims = (this->loadFlags & rawrbox::GLTFLoadFlags::Debug::PRINT_ANIMATIONS) > 0;

		this->_logger->debug("Building {} animations...", this->_parsedAnimations.size());
		for (const auto& anim : this->_parsedAnimations) {
			auto animation = std::make_unique<rawrbox::Animation>(anim.name);

			// TARGETS ----
			std::vector<size_t> targetNodes = {};
			targetNodes.reserve(anim.tracks.size());
			for (const auto& track : anim.tracks) {
				targetNodes.push_back(track.first);
			}

			std::ranges::sort(targetNodes);

			std::vector<rawrbox::Skeleton*> targetSkeletons = {};
			std::vector<size_t> meshNodes = {};

			for (const auto& node : targetNodes) {
				bool targeted = false;

				if (auto fndSkeletons = this->_nodeSkeletons.find(node); fndSkeletons != this->_nodeSkeletons.end()) {
					targeted = true;

					for (auto* skeleton : fndSkeletons->second) {
						if (std::ranges::find(targetSkeletons, skeleton) != targetSkeletons.end()) continue;
						targetSkeletons.push_back(skeleton);
					}
				}

				if (auto fndMesh = this->_nodeMeshes.find(node); fndMesh != this->_nodeMeshes.end()) {
					targeted = true;
					if (fndMesh->second->skeleton == nullptr) meshNodes.push_back(node);
				}

				if (!targeted) this->_logger->debug("Animation '{}' targets node '{}' a invalid mesh / joint! Skipping", anim.name, scene.nodes[node].name);
			}
			// -----------------------------------------------------------------------

			// SKELETON PARTS ----
			for (auto* skeleton : targetSkeletons) {
				auto fndJoints = this->_skeletonJoints.find(skeleton);
				if (fndJoints == this->_skeletonJoints.end()) continue;

				const auto& jointNodes = fndJoints->second;

				ozz::animation::offline::RawAnimation rawAnim = {};
				rawAnim.name = anim.name.c_str();
				rawAnim.duration = anim.duration;
				rawAnim.tracks.reserve(jointNodes.size());

				for (const auto& node : jointNodes) {
					auto fnd = anim.tracks.find(node);

					ozz::animation::offline::RawAnimation::JointTrack track = fnd != anim.tracks.end() ? fnd->second : ozz::animation::offline::RawAnimation::JointTrack{};
					this->fillRestPose(track, this->getRestPose(scene.nodes[node]));

					rawAnim.tracks.push_back(std::move(track));
				}

				if (optimize) {
					ozz::animation::offline::AnimationOptimizer optimizer;
					ozz::animation::offline::RawAnimation input = rawAnim;

					if (!optimizer(input, skeleton->getSkeleton(), &rawAnim)) {
						this->_logger->warn("Failed to optimize animation '{}' for skeleton '{}'", anim.name, skeleton->name);
					}
				}

				auto built = builder(rawAnim);
				if (built == nullptr) {
					this->_logger->warn("Failed to build animation '{}' for skeleton '{}'", anim.name, skeleton->name);
					continue;
				}

				rawrbox::AnimationPart part = {};
				part.type = rawrbox::AnimationType::SKELETON;
				part.animation = std::move(built);
				part.skeleton = skeleton;

				animation->addPart(std::move(part));
			}
			// -------------------

			// VERTEX ANIMATION ----
			if (!meshNodes.empty()) {
				ozz::animation::offline::RawAnimation rawAnim = {};

				rawAnim.name = anim.name.c_str();
				rawAnim.duration = anim.duration;
				rawAnim.tracks.reserve(meshNodes.size());

				std::vector<uint32_t> meshIDs = {};
				meshIDs.reserve(meshNodes.size());

				for (const auto& node : meshNodes) {
					ozz::animation::offline::RawAnimation::JointTrack track = anim.tracks.at(node);

					this->fillRestPose(track, this->getRestPose(scene.nodes[node]));
					this->toLeftHand(track);

					rawAnim.tracks.push_back(std::move(track));
					meshIDs.push_back(static_cast<uint32_t>(this->_nodeMeshes.at(node)->index));
				}

				auto built = builder(rawAnim);
				if (built == nullptr) {
					this->_logger->warn("Failed to build vertex animation '{}'", anim.name);
				} else {
					rawrbox::AnimationPart part = {};
					part.type = rawrbox::AnimationType::VERTEX;
					part.animation = std::move(built);
					part.meshes = std::move(meshIDs);

					animation->addPart(std::move(part));
				}
			}
			// ----------------

			if (animation->empty()) {
				this->_logger->warn("Animation '{}' has no valid targets, skipping...", anim.name);
				continue;
			}

			if (printAnims) {
				this->_logger->debug("Built animation '{}' -> {} skeleton(s), {} vertex track(s), {:.2f}s", fmt::styled(anim.name, fmt::fg(fmt::color::green_yellow)), targetSkeletons.size(), meshNodes.size(), animation->duration);
			}

			this->animations.push_back(std::move(animation));
		}
	}

	void GLTFImporter::fillRestPose(ozz::animation::offline::RawAnimation::JointTrack& track, const ozz::math::Transform& rest) {
		if (track.translations.empty()) track.translations.push_back({0.F, rest.translation});
		if (track.rotations.empty()) track.rotations.push_back({0.F, rest.rotation});
		if (track.scales.empty()) track.scales.push_back({0.F, rest.scale});
	}

	void GLTFImporter::toLeftHand(ozz::animation::offline::RawAnimation::JointTrack& track) {
		for (auto& [time, value] : track.translations) {
			value.z = -value.z;
		}

		for (auto& [time, value] : track.rotations) {
			value.x = -value.x;
			value.y = -value.y;
		}

		for (auto& [time, value] : track.scales) {
			value.z = -value.z;
		}
	}

	ozz::math::Float3 GLTFImporter::hermite(const ozz::math::Float3& p0, const ozz::math::Float3& m0, const ozz::math::Float3& p1, const ozz::math::Float3& m1, float t, float interval) {
		ozz::math::Float3 out = ozz::math::Float3::zero();
		rawrbox::MathUtils::hermite(&p0.x, &m0.x, &p1.x, &m1.x, t, interval, &out.x, 3);

		return out;
	}

	ozz::math::Quaternion GLTFImporter::hermite(const ozz::math::Quaternion& p0, const ozz::math::Quaternion& m0, const ozz::math::Quaternion& p1, const ozz::math::Quaternion& m1, float t, float interval) {
		ozz::math::Quaternion out = ozz::math::Quaternion::identity();
		rawrbox::MathUtils::hermite(&p0.x, &m0.x, &p1.x, &m1.x, t, interval, &out.x, 4);
		return ozz::math::NormalizeSafe(out, ozz::math::Quaternion::identity());
	}

	// -------------

	// MODEL ---
	void GLTFImporter::loadScene(const fastgltf::Asset& scene) {
		for (const auto& rootScenes : scene.scenes) {
			for (const auto& nodeIndex : rootScenes.nodeIndices) {
				this->loadNodes(scene, nodeIndex);
			}
		}
	}

	void GLTFImporter::loadNodes(const fastgltf::Asset& scene, size_t nodeIndex) {
		if (nodeIndex >= scene.nodes.size()) {
			this->_logger->warn("Invalid node index {}, skipping...", nodeIndex);
			return;
		}

		const auto& node = scene.nodes[nodeIndex];

		if (node.lightIndex) {
			this->lights.push_back(std::make_unique<rawrbox::GLTFLight>(this->lights.size(), nodeIndex, node, scene.lights[node.lightIndex.value()]));
		}

		if (node.meshIndex) {
			if (this->_nodeMeshes.contains(nodeIndex)) {
				this->_logger->warn("Node '{}' is referenced more than once, skipping duplicate...", node.name);
			} else {
				auto mesh = this->extractMesh(scene, nodeIndex, node);
				if (mesh != nullptr) {
					this->_nodeMeshes[nodeIndex] = mesh.get();
					this->meshes.push_back(std::move(mesh));
				}
			}
		}

		// Children ---
		for (const auto& children : node.children) {
			this->loadNodes(scene, children);
		}
		// ---
	}

	std::unique_ptr<rawrbox::GLTFMesh> GLTFImporter::extractMesh(const fastgltf::Asset& scene, size_t nodeIndex, const fastgltf::Node& node) {
		auto gltfMesh = std::make_unique<rawrbox::GLTFMesh>(this->meshes.size(), nodeIndex, node);

		std::optional<size_t> skinJoints = std::nullopt;
		if (node.skinIndex.has_value() && node.skinIndex.value() < scene.skins.size()) skinJoints = scene.skins[node.skinIndex.value()].joints.size();

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
			rawrPrimitive.vertices = this->extractVertex(scene, primitive, skinJoints);
			rawrPrimitive.indices = this->extractIndices(scene, primitive);
			// -----------

			// OPTIMIZATION ---
			const bool meshOptimize = (this->loadFlags & rawrbox::GLTFLoadFlags::Optimizer::MESH_OPTIMIZE) > 0;
			const bool meshSimplify = (this->loadFlags & rawrbox::GLTFLoadFlags::Optimizer::MESH_SIMPLIFY) > 0;

			if (meshOptimize || meshSimplify) {
				if (rawrPrimitive.blendShapes.empty()) {
					auto startVert = rawrPrimitive.vertices.size();
					auto startInd = rawrPrimitive.indices.size();

					if (meshOptimize) {
						const bool transparent = rawrPrimitive.material != nullptr && rawrPrimitive.material->transparent;
						rawrbox::MeshOptimization::optimize(rawrPrimitive.vertices, rawrPrimitive.indices, !transparent);
					}

					if (meshSimplify) {
						rawrbox::MeshOptimization::simplify(rawrPrimitive.vertices, rawrPrimitive.indices);
					}

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

	std::vector<rawrbox::VertexNormBoneData> GLTFImporter::extractVertex(const fastgltf::Asset& scene, const fastgltf::Primitive& primitive, std::optional<size_t> skinJoints) {
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
				verts[idx].normal = rawrbox::PackUtils::packOCTNormal(normal.x(), normal.y(), normal.z());
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
				    verts[idx].tangent = rawrbox::PackUtils::packOCTTangent(tangent.x(), tangent.y(), tangent.z(), tangent.w() >= 0.0F ? 1.0F : -1.0F);
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

				const uint32_t maxJoints = static_cast<uint32_t>(std::min<size_t>(skinJoints.value_or(RB_RENDER_MAX_BONES_PER_MODEL), RB_RENDER_MAX_BONES_PER_MODEL));

				fastgltf::iterateAccessorWithIndex<fastgltf::math::uvec4>(scene, jointAccessor, [&](fastgltf::math::uvec4 joints, std::size_t idx) {
					std::array<uint32_t, RB_MAX_BONES_PER_VERTEX> indices = {joints.x(), joints.y(), joints.z(), joints.w()};

					for (auto& joint : indices) {
						if (joint < maxJoints) continue;
						this->_logger->warn("Joint index {} exceeds the joint limit ({})", joint, maxJoints);

						joint = 0;
					}

					verts[idx].bone_indices = rawrbox::PackUtils::packBoneIndices(indices);
				});

				fastgltf::iterateAccessorWithIndex<fastgltf::math::fvec4>(scene, weightAccessor, [&](fastgltf::math::fvec4 weights, std::size_t idx) {
					verts[idx].bone_weights = rawrbox::PackUtils::packBoneWeights({weights.x(), weights.y(), weights.z(), weights.w()});
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
	bool GLTFImporter::isValid(const fastgltf::Asset& scene, size_t index, fastgltf::AccessorType type) const {
		if ((this->loadFlags & rawrbox::GLTFLoadFlags::VALIDATE) == 0) return true;
		if (index >= scene.accessors.size()) return false;

		return scene.accessors[index].type == type;
	}

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
		this->lights.clear(); // Clear old lights

		this->textures.clear();     // Clear old textures
		this->_texturesMap.clear(); // Clear old textures

		this->_nodeParents.clear();
		this->_nodeMeshes.clear();

		this->_parsedAnimations.clear();
		this->animations.clear();

		this->_skinSkeletons.clear();
		this->_skeletonJoints.clear();
		this->_nodeSkeletons.clear();
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
