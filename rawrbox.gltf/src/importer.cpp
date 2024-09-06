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

// https://raw.githubusercontent.com/KhronosGroup/glTF/main/specification/2.0/figures/gltfOverview-2.0.0d.png
// https://github.com/NVIDIA/OptiX_Apps/blob/82b27c9c035a08289acda4f19a8a80afa99f17a4/apps/GLTF_renderer/Application.cpp#L4438

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
		    fastgltf::Options::DontRequireValidAssetMember |
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
		// TODO: PRINT METADATA

		// LOAD MATERIALS ---
		if ((this->loadFlags & rawrbox::ModelLoadFlags::IMPORT_TEXTURES) > 0) {
			this->loadTextures(scene);
			this->loadMaterials(scene);
		}
		// --------------

		// LOAD SKELETONS ---
		if ((this->loadFlags & rawrbox::ModelLoadFlags::IMPORT_ANIMATIONS) > 0) {
			this->loadSkeletons(scene);
		}
		// -------------------

		// LOAD SCENE ---
		this->loadScene(scene);
		//  ---------------

		// LOAD ANIMATIONS ---
		if ((this->loadFlags & rawrbox::ModelLoadFlags::IMPORT_ANIMATIONS) > 0) {
			this->loadAnimations(scene);
		}
		// -------------------
	}

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
			if (name.empty()) name = "Unknown";

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

	ozz::math::Transform GLTFImporter::extractTransform(const fastgltf::TRS& mtx) {
		ozz::math::Transform tr;
		tr.rotation = {mtx.rotation.x(), mtx.rotation.y(), mtx.rotation.z(), mtx.rotation.w()};
		tr.translation = {mtx.translation.x(), mtx.translation.y(), mtx.translation.z()};
		tr.scale = {mtx.scale.x(), mtx.scale.y(), mtx.scale.z()};

		return tr;
		/*if (skin.inverseBindMatrices) return {};
		const auto& matrixAccessor = scene.accessors[skin.inverseBindMatrices.value()];

		fastgltf::iterateAccessor<fastgltf::math::fmat4x4>(
		    scene, matrixAccessor,
		    [&](const fastgltf::math::fmat4x4& mtx) {
			    return rawrbox::Matrix4x4(mtx.data());
		    });

		return {};*/
	}

	void GLTFImporter::generateBones(const fastgltf::Asset& scene, const fastgltf::Node& node, ozz::animation::offline::RawSkeleton::Joint& parent) {
		for (const auto& childIndex : node.children) {
			const auto& childNode = scene.nodes[childIndex];
			std::string boneName = {childNode.name.begin(), childNode.name.end()};

			ozz::animation::offline::RawSkeleton::Joint joint;
			joint.name = boneName;
			joint.transform = this->extractTransform(std::get<fastgltf::TRS>(childNode.transform));

			this->generateBones(scene, childNode, joint);
			parent.children.push_back(joint);
		}
	}

	void GLTFImporter::loadSkeletons(const fastgltf::Asset& scene) {
		if (scene.skins.empty()) return;

		this->skeletons.resize(scene.skins.size());
		for (size_t i = 0; i < scene.skins.size(); i++) {
			const auto& skin = scene.skins[i];
			if (skin.joints.empty()) continue;

			const auto& rootJoint = scene.nodes[skin.joints[0]];

			ozz::animation::offline::RawSkeleton skeleton;
			skeleton.roots.resize(skin.joints.size());

			// ROOT JOINT ----
			skeleton.roots[0].name = rootJoint.name;
			skeleton.roots[0].transform = this->extractTransform(std::get<fastgltf::TRS>(rootJoint.transform)); // Bind Pose
			//  ----

			if (!rootJoint.children.empty()) {
				this->generateBones(scene, rootJoint, skeleton.roots[0]);
			}

			// DEBUG ----
			if ((this->loadFlags & rawrbox::ModelLoadFlags::Debug::PRINT_BONE_STRUCTURE) > 0) {
				std::function<void(ozz::animation::offline::RawSkeleton::Joint&, int, bool)> printBone;
				printBone = [this, &printBone](ozz::animation::offline::RawSkeleton::Joint& bn, int deep, bool isLast) -> void {
					std::string indent(deep * 4, ' ');
					std::string branch = isLast ? "└── " : "├── ";
					this->_logger->info("{}{}{}", indent, branch, bn.name);

					for (size_t i = 0; i < bn.children.size(); i++) {
						bool lastChild = (i == bn.children.size() - 1);
						printBone(bn.children[i], deep + 1, lastChild);
					}
				};

				printBone(skeleton.roots[0], 0, true);
				this->_logger->info("'{}' BONES", fmt::styled(this->filePath.generic_string(), fmt::fg(fmt::color::cyan)));
			}
			// -------------

			if (!skeleton.Validate()) {
				this->_logger->warn("Invalid skeleton '{}'", skin.name);
				continue;
			}

			ozz::animation::offline::SkeletonBuilder builder;
			this->skeletons[i] = builder(skeleton);
		}
	}
	// ------------

	// ANIMATIONS ---
	void GLTFImporter::loadAnimations(const fastgltf::Asset& scene) {
		if (scene.animations.empty()) return;

		this->animations.resize(scene.animations.size());
		for (size_t i = 0; i < scene.animations.size(); i++) {
			const auto& anim = scene.animations[i];
			const std::string animName = std::string(anim.name.begin(), anim.name.end());

			ozz::animation::offline::RawAnimation rawrAnim;
			rawrAnim.duration = 0.F;
			rawrAnim.tracks.resize(anim.channels.size());
			rawrAnim.name = animName;

			if ((this->loadFlags & rawrbox::ModelLoadFlags::Debug::PRINT_ANIMATIONS) > 0) {
				this->_logger->info("Found animation '{}'", fmt::styled(animName, fmt::fg(fmt::color::green_yellow)));
			}

			for (size_t c = 0; c < anim.channels.size(); c++) {
				const auto& channel = anim.channels[c];
				if (!channel.nodeIndex) continue; // Not a animation?

				auto& track = rawrAnim.tracks[c];

				const size_t nodeIndex = channel.nodeIndex.value();
				const auto& mesh = this->meshes[nodeIndex]; // Mesh being affected

				const auto& sampler = anim.samplers[channel.samplerIndex];
				const auto& timeAccessor = scene.accessors[sampler.inputAccessor];
				const auto& dataAccessor = scene.accessors[sampler.outputAccessor];

				// Build search map --
				this->trackToMesh[i].push_back(mesh.get());
				// ----------------

				if (timeAccessor.count != dataAccessor.count) // Invalid data
					continue;

				// TIME ----
				for (size_t iTime = 0; iTime < timeAccessor.count; iTime++) {
					double t = fastgltf::getAccessorElement<double>(scene, timeAccessor, iTime);
					if (t > rawrAnim.duration) rawrAnim.duration = t;

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
			}

			if (!rawrAnim.Validate()) {
				this->_logger->warn("Invalid animation '{}'", animName);
				return;
			}

			// Build the animations ---
			ozz::animation::offline::AnimationBuilder builder;
			this->animations[i] = builder(rawrAnim);
			// ---------------------------
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

		if (node.skinIndex) {
			gltfMesh->skeleton = this->skeletons[node.skinIndex.value()].get();
		}

		auto mtx = std::get<fastgltf::TRS>(node.transform);
		gltfMesh->position = {mtx.translation.x(), mtx.translation.y(), mtx.translation.z()};
		gltfMesh->scale = {mtx.scale.x(), mtx.scale.y(), mtx.scale.z()};
		gltfMesh->rotation = {mtx.rotation.x(), mtx.rotation.y(), mtx.rotation.z(), mtx.rotation.w()};

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
