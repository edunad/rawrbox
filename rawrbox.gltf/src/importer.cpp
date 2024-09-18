#include <rawrbox/gltf/importer.hpp>
#include <rawrbox/render/textures/webp.hpp>
#include <rawrbox/utils/file.hpp>

#include <fastgltf/tools.hpp>
#include <fastgltf/types.hpp>
#include <magic_enum.hpp>

#include <ozz/animation/offline/animation_builder.h>
#include <ozz/animation/offline/skeleton_builder.h>
#include <ozz/animation/offline/track_optimizer.h>

#include <variant>

template <>
struct fastgltf::ElementTraits<ozz::math::Quaternion> : fastgltf::ElementTraitsBase<ozz::math::Quaternion, AccessorType::Vec4, float> {};

template <>
struct fastgltf::ElementTraits<ozz::math::Float3> : fastgltf::ElementTraitsBase<ozz::math::Float3, AccessorType::Vec3, float> {};

template <>
struct fastgltf::ElementTraits<rawrbox::Vector3f> : fastgltf::ElementTraitsBase<rawrbox::Vector3f, AccessorType::Vec3, float> {};

template <>
struct fastgltf::ElementTraits<rawrbox::Vector4f> : fastgltf::ElementTraitsBase<rawrbox::Vector4f, AccessorType::Vec4, float> {};

// https://github.com/Deweh/NativeAnimationFrameworkSF/blob/7b8ad3d4de0b7752dde302c1f64cf819b07586c0/Plugin/src/Serialization/GLTFImport.cpp#L233
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

		auto asset = parser.loadGltf(data, this->filePath.parent_path(), gltfOptions);
		if (asset.error() != fastgltf::Error::None) {
			this->_logger->warn("{}", fastgltf::getErrorMessage(asset.error()));
			return;
		}

#ifdef _DEBUG
		/*auto err = fastgltf::validate(asset.get());
		if (err != fastgltf::Error::None) {
			this->_logger->warn("{}", fastgltf::getErrorMessage(asset.error()));
			return;
		}*/
#endif

		fastgltf::Asset& scene = asset.get();

		// POST-LOAD ---
		this->postLoadFixSceneNames(scene);
		// ---------------

		// TODO: PRINT METADATA

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
		for (size_t i = 0; i < scene.nodes.size(); i++) {
			auto& nodes = scene.nodes[i];
			if (nodes.name.empty()) nodes.name = fmt::format("node_{}", i);
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
				imgIndex = {gltfTexture.imageIndex.value(), rawrbox::GLTFImageType::WEBP};
			} else if (gltfTexture.ddsImageIndex.has_value()) {
				// imgIndex = {gltfTexture.imageIndex.value(), rawrbox::GLTFImageType::DDS};  // TODO: SUPPORT DDS
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
					break;
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

		this->skeletons.resize(scene.skins.size());
		for (size_t i = 0; i < scene.skins.size(); i++) {
			const auto& skin = scene.skins[i];

			if (skin.joints.empty()) {
				this->_logger->warn("Skin '{}' has no joints, skipping...", skin.name);
				continue;
			}

			if (!skin.inverseBindMatrices.has_value()) {
				this->_logger->warn("Skin '{}' has no inverse bind matrices, skipping...", skin.name);
				continue;
			}

			// ----------------
			std::vector<std::array<float, 16>> inverseBindMatrices(skin.joints.size());
			// std::vector<std::array<float, 16>> cleanInverseMtx = {};

			fastgltf::iterateAccessorWithIndex<fastgltf::math::fmat4x4>(
			    scene, scene.accessors[skin.inverseBindMatrices.value()],
			    [&](const fastgltf::math::fmat4x4& mtx, size_t index) {
				    std::memcpy(inverseBindMatrices[index].data(), mtx.data(), sizeof(float) * mtx.size());
			    });
			// ---------------------------

			ozz::animation::offline::RawSkeleton rawSkeleton;

			// ROOT JOINT ----
			const auto& rootJoint = scene.nodes[skin.joints[0]];

			ozz::animation::offline::RawSkeleton::Joint rootBone;
			rootBone.name = rootJoint.name;
			rootBone.transform = this->extractTransform(rootJoint.transform);

			// cleanInverseMtx.push_back(inverseBindMatrices[skin.joints[0]]);
			//   -----------------

			// GENERATE CHILDREN BONES ---
			if (!rootJoint.children.empty()) {
				std::function<void(const fastgltf::Node& node, ozz::animation::offline::RawSkeleton::Joint& parent)> generateBones;

				generateBones = [this, &scene, i, &generateBones](const fastgltf::Node& node, ozz::animation::offline::RawSkeleton::Joint& parent) {
					/*for (const auto& childIndex : node.children) {
						const auto& childNode = scene.nodes[childIndex];
						if (childIndex > this->meshes.size()) throw this->_logger->error("Missing child index '{}' mesh", childIndex);

						ozz::animation::offline::RawSkeleton::Joint childJoint = {};
						childJoint.name = std::string(childNode.name);
						childJoint.transform = extractTransform(childNode.transform);

						cleanInverseMtx.push_back(inverseBindMatrices[childIndex]);

						generateBones(childNode, childJoint);
						parent.children.push_back(childJoint);
					}*/
				};

				generateBones(rootJoint, rootBone);
			}
			// -----------------------

			rawSkeleton.roots.push_back(rootBone);

			// DEBUG ----
			if ((this->loadFlags & rawrbox::ModelLoadFlags::Debug::PRINT_BONE_STRUCTURE) > 0) {
				std::function<void(ozz::animation::offline::RawSkeleton::Joint&, int, bool)> printBone;
				printBone = [this, &printBone](ozz::animation::offline::RawSkeleton::Joint& bn, int depth, bool isLast) -> void {
					std::string indent(depth * 4, ' ');
					std::string branch = isLast ? "└── " : "├── ";
					this->_logger->info("{}{}{}", indent, branch, bn.name);

					for (size_t i = 0; i < bn.children.size(); i++) {
						bool lastChild = (i == bn.children.size() - 1);
						printBone(bn.children[i], depth + 1, lastChild);
					}
				};
				this->_logger->info("Found skeleton '{}'", skin.name);
				printBone(rootBone, 0, true);
			}
			// -------------

			// Build the skeletons ---
			auto buildSkeleton = builder(rawSkeleton);
			if (buildSkeleton == nullptr) {
				this->_logger->warn("Failed to build skeleton '{}'", skin.name);
				continue;
			}

			this->skeletons[i] = std::move(buildSkeleton);
			this->skeletons[i]->inverseBindMatrices = inverseBindMatrices;
			//  ----------------------

			// Apply skins on bones -- (probably not needed)
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
		/*for (auto& mesh : this->meshes) {
			mesh->skeleton = this->skeletons[0].get();
		}*/
		// --------------
	}
	// ------------

	// ANIMATIONS ---
	void GLTFImporter::loadAnimations(const fastgltf::Asset& scene) {
		if (scene.animations.empty()) return;

		this->_parsedAnimations.resize(scene.animations.size());
		for (size_t i = 0; i < scene.animations.size(); i++) {
			const auto& anim = scene.animations[i];

			auto& gltfAnim = this->_parsedAnimations[i];
			gltfAnim.name = std::string(anim.name.begin(), anim.name.end());
			gltfAnim.duration = 0.F;

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
					this->_logger->warn("Invalid data for animation '{}'", gltfAnim.name);
					continue;
				}

				const size_t nodeIndex = channel.nodeIndex.value();
				const auto& mesh = this->meshes[nodeIndex]; // Mesh being affected
				const auto& node = scene.nodes[nodeIndex];  // Node being affected

				// Check if it's a skeleton animation
				if (mesh->skeleton != nullptr) {
					if (gltfAnim.skeleton != nullptr && gltfAnim.skeleton != mesh->skeleton) {
						this->_logger->warn("Animation '{}' contains 2 or more skeletons, this is not supported! Please split the animation per skeleton", gltfAnim.name);
						continue;
					} else {
						gltfAnim.skeleton = mesh->skeleton;
					}
				}
				// ----------------

				auto& track = gltfAnim.tracks[mesh->name];

				// TIME ----
				for (size_t iTime = 0; iTime < timeAccessor.count; iTime++) {
					double t = fastgltf::getAccessorElement<double>(scene, timeAccessor, iTime);
					if (t > gltfAnim.duration) gltfAnim.duration = t; // Calculate the total animation time

					switch (channel.path) {
						case fastgltf::AnimationPath::Translation:
							{
								ozz::math::Float3 newPos = fastgltf::getAccessorElement<ozz::math::Float3>(scene, dataAccessor, iTime);
								track.translations.emplace_back(t, newPos);
								break;
							}
						case fastgltf::AnimationPath::Rotation:
							{
								ozz::math::Quaternion newRot = fastgltf::getAccessorElement<ozz::math::Quaternion>(scene, dataAccessor, iTime);
								track.rotations.emplace_back(t, newRot);
								break;
							}
						case fastgltf::AnimationPath::Scale:
							{
								ozz::math::Float3 newScale = fastgltf::getAccessorElement<ozz::math::Float3>(scene, dataAccessor, iTime);
								track.scales.emplace_back(t, newScale);
								break;
							}
							break;
						case fastgltf::AnimationPath::Weights:
							break; // TODO: SUPPORT BLEND SHAPES
						default:
							// Handle other cases
							break;
					}
				}
				// -------------

				// FIX MISSING TRACKS ---
				if (gltfAnim.skeleton != nullptr) {
					auto& rTl = track.rotations;
					auto& pTl = track.translations;
					auto& sTl = track.scales;

					auto transform = this->extractTransform(node.transform);

					if (rTl.empty()) rTl.emplace_back(0.0f, transform.rotation);
					if (pTl.empty()) pTl.emplace_back(0.0f, transform.translation);
					if (sTl.empty()) sTl.emplace_back(0.0f, transform.scale);
				}
				// ----------------------
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
				for (auto& o : anim.skeleton->joint_names()) {
					auto jointName = std::string(o);
					rawrAnim.tracks.emplace_back(anim.tracks[jointName]);
				}
			} else {
				continue;
				/*for (const auto& track : anim.tracks) {
					rawrAnim.tracks.emplace_back(track.second);
				}*/
			}

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
			if (rootScenes.nodeIndices.size() == 1) {
				this->loadMeshes(scene, scene.nodes[0]); // ROOT
			} else {
				for (const auto& nodeIndex : rootScenes.nodeIndices) {
					this->loadMeshes(scene, scene.nodes[nodeIndex], nullptr);
				}
			}
		}
	}

	void GLTFImporter::loadMeshes(const fastgltf::Asset& scene, const fastgltf::Node& node, rawrbox::GLTFMesh* parentMesh) {
		auto gltfMesh = std::make_unique<rawrbox::GLTFMesh>(std::string{node.name.begin(), node.name.end()});

		gltfMesh->parent = parentMesh;
		gltfMesh->index = this->meshes.size();
		gltfMesh->matrix = this->toMatrix(std::get<fastgltf::TRS>(node.transform));

		if (node.meshIndex) {
			const auto& mesh = scene.meshes[node.meshIndex.value()];

			// Primitives
			for (const auto& primitive : mesh.primitives) {
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

				if (primitive.type != fastgltf::PrimitiveType::Triangles) {
					this->_logger->warn("Primitive type '{}' not supported", magic_enum::enum_name(primitive.type));
					continue;
				}

				std::vector<rawrbox::VertexNormBoneData> verts = this->extractVertex(scene, primitive);
				gltfMesh->vertices.insert(gltfMesh->vertices.end(), verts.begin(), verts.end());

				std::vector<uint16_t> indices = this->extractIndices(scene, primitive);
				gltfMesh->indices.insert(gltfMesh->indices.end(), indices.begin(), indices.end());
			}
			/// -------
		}

		// BLEND SHAPES ---
		// https://github.com/assimp/assimp/blob/master/code/AssetLib/glTF2/glTF2Importer.cpp#L637
		/*if ((this->loadFlags & rawrbox::ModelLoadFlags::IMPORT_BLEND_SHAPES) > 0 && !primitive.targets.empty()) {
			for (auto& targets : primitive.targets) {
			}
		}*/
		// --------------

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

	std::vector<uint16_t> GLTFImporter::extractIndices(const fastgltf::Asset& scene, const fastgltf::Primitive& primitive) {
		auto indices = std::vector<uint16_t>();

		const auto& accessor = scene.accessors[primitive.indicesAccessor.value()];
		indices.resize(accessor.count);

		// INDICES ----
		// TODO: SUPPORT UINT32 FOR INDICES
		if (accessor.componentType == fastgltf::ComponentType::UnsignedByte || accessor.componentType == fastgltf::ComponentType::UnsignedShort) {
			fastgltf::iterateAccessorWithIndex<uint16_t>(scene, accessor, [&](uint16_t indice, size_t idx) { indices[idx] = indice; });
		}
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

	// -------------

	// PUBLIC -------

	GLTFImporter::GLTFImporter(uint32_t loadFlags) : loadFlags(loadFlags) {}
	GLTFImporter::~GLTFImporter() {
		this->_logger.reset();

		this->meshes.clear(); // Clear old meshes

		this->textures.clear();     // Clear old textures
		this->_texturesMap.clear(); // Clear old textures

		this->trackToMesh.clear();
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
