#include <rawrbox/gltf/importer.hpp>
#include <rawrbox/render/textures/webp.hpp>
#include <rawrbox/utils/file.hpp>

#include <fastgltf/tools.hpp>
#include <fastgltf/types.hpp>
#include <magic_enum.hpp>

#include <variant>
// https://raw.githubusercontent.com/KhronosGroup/glTF/main/specification/2.0/figures/gltfOverview-2.0.0d.png

namespace rawrbox {
	// PRIVATE -----
	void GLTFImporter::internalLoad(fastgltf::GltfDataBuffer& data) {
		fastgltf::Parser parser{
		    fastgltf::Extensions::KHR_mesh_quantization |
		    fastgltf::Extensions::KHR_materials_unlit |
		    fastgltf::Extensions::KHR_materials_specular |
		    fastgltf::Extensions::KHR_texture_basisu |
		    fastgltf::Extensions::EXT_texture_webp |
		    fastgltf::Extensions::KHR_materials_emissive_strength};

		auto gltfOptions =
		    fastgltf::Options::DontRequireValidAssetMember |
		    fastgltf::Options::AllowDouble |
		    fastgltf::Options::LoadExternalBuffers;

		if ((this->loadFlags & rawrbox::ModelLoadFlags::IMPORT_TEXTURES) > 0) {
			gltfOptions |= fastgltf::Options::LoadExternalImages; // Handle loading for us
		}

		auto asset = parser.loadGltf(data, this->filePath.parent_path(), gltfOptions);
		if (asset.error() != fastgltf::Error::None) {
			this->_logger->warn("{}", fastgltf::getErrorMessage(asset.error()));
			return;
		}

		fastgltf::Asset& scene = asset.get();
		// TODO: PRINT METADATA

		// LOAD MATERIALS ---
		if ((this->loadFlags & rawrbox::ModelLoadFlags::IMPORT_TEXTURES) > 0) {
			this->loadTextures(scene);
			this->loadMaterials(scene);
		}
		// ---------------

		// LOAD MODELS ---
		this->loadSubmeshes(scene);
		// ---------------
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
			if (material.pbrData.baseColorTexture.has_value()) {
				const auto& texture = this->_texturesMap[material.pbrData.baseColorTexture->textureIndex];
				mat->diffuse = texture == nullptr ? rawrbox::MISSING_TEXTURE.get() : texture;
			} else {
				mat->diffuse = rawrbox::WHITE_TEXTURE.get();
			}
			// ----

			// METALIC ---
			if (material.pbrData.metallicRoughnessTexture.has_value()) {
				const auto& texture = this->_texturesMap[material.pbrData.metallicRoughnessTexture->textureIndex];
				mat->metalRough = texture == nullptr ? rawrbox::NORMAL_TEXTURE.get() : texture;
			}
			// --------------------

			// NORMAL ---
			if (material.normalTexture.has_value()) {
				const auto& texture = this->_texturesMap[material.normalTexture->textureIndex];
				mat->normal = texture == nullptr ? rawrbox::NORMAL_TEXTURE.get() : texture;
			}
			// --------------------
			// --------------------

			// COLORS ---
			float alpha = material.pbrData.baseColorFactor[3];

			mat->baseColor = rawrbox::Colorf(material.pbrData.baseColorFactor[0], material.pbrData.baseColorFactor[1], material.pbrData.baseColorFactor[2], alpha);
			mat->metalnessFactor = material.pbrData.metallicFactor;
			mat->roughnessFactor = material.pbrData.roughnessFactor;

			mat->emissionFactor = material.emissiveStrength;
			mat->emissionColor = rawrbox::Colorf(material.emissiveFactor[0], material.emissiveFactor[1], material.emissiveFactor[2], alpha);
			// ---------

			// SPECULAR ---
			if (material.specular != nullptr) {
				mat->specularColor = rawrbox::Colorf(material.specular->specularColorFactor[0], material.specular->specularColorFactor[1], material.specular->specularColorFactor[2], alpha);
				mat->specularFactor = material.specular->specularFactor;

				if (material.specular->specularTexture.has_value()) {
					const auto& texture = this->_texturesMap[material.specular->specularTexture->textureIndex];
					mat->specular = texture == nullptr ? rawrbox::MISSING_TEXTURE.get() : texture;
				}
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

	// MODEL ---
	void GLTFImporter::loadSubmeshes(const fastgltf::Asset& scene) {
		for (const auto& node : scene.nodes) {
			if (!node.meshIndex.has_value()) continue;

			const auto& mesh = scene.meshes[node.meshIndex.value()];
			rawrbox::GLTFMesh gltfMesh = {{node.name.begin(), node.name.end()}};

			// Primitives
			for (const auto& primitive : mesh.primitives) {
				// Textures --
				if (primitive.materialIndex) {
					if ((this->loadFlags & rawrbox::ModelLoadFlags::IMPORT_TEXTURES) > 0) {
						auto* material = this->materials[primitive.materialIndex.value()].get();

						if (gltfMesh.material != nullptr && gltfMesh.material != material) {
							this->_logger->warn("Tried to load 2 different materials on the same submesh, this isn't supported!");
						} else {
							gltfMesh.material = material;
						}
					}
				}
				// ----------

				if (primitive.type != fastgltf::PrimitiveType::Triangles) {
					this->_logger->warn("Primitive type '{}' not supported", magic_enum::enum_name(primitive.type));
					continue;
				}

				std::vector<rawrbox::VertexNormBoneData> verts = this->extractVertex(scene, primitive);
				gltfMesh.vertices.insert(gltfMesh.vertices.end(), verts.begin(), verts.end());

				std::vector<uint16_t> indices = this->extractIndices(scene, primitive);
				gltfMesh.indices.insert(gltfMesh.indices.end(), indices.begin(), indices.end());
			}
			/// -------

			// Transform
			auto TRS = std::get<fastgltf::TRS>(node.transform);
			rawrbox::Vector4f rot = {TRS.rotation.x(), TRS.rotation.y(), TRS.rotation.z(), TRS.rotation.w()};
			rawrbox::Vector3f pos = {TRS.translation.x(), TRS.translation.y(), TRS.translation.z()};
			rawrbox::Vector3f scale = {TRS.scale.x(), TRS.scale.y(), TRS.scale.z()};

			gltfMesh.matrix = rawrbox::Matrix4x4::mtxSRT(scale, rot, pos);
			// --------

			// Bones
			if ((this->loadFlags & rawrbox::ModelLoadFlags::IMPORT_ANIMATIONS) > 0 /*&& aiMesh.HasBones()*/) {
				// this->loadSkeleton(sc, mesh, aiMesh);
			}
			// -------------------

			// Blendshapes
			if ((this->loadFlags & rawrbox::ModelLoadFlags::IMPORT_BLEND_SHAPES) > 0) {
				// this->loadBlendShapes(aiMesh, mesh);
			}
			// -------------------

			this->meshes.push_back(gltfMesh);
		}
	}

	std::vector<rawrbox::VertexNormBoneData> GLTFImporter::extractVertex(const fastgltf::Asset& scene, const fastgltf::Primitive& primitive) {
		std::vector<rawrbox::VertexNormBoneData> verts = {};

		// POSITION ----
		const auto* positionAttribute = primitive.findAttribute("POSITION");
		if (positionAttribute == nullptr) throw this->_logger->error("Invalid gltf model, missing 'POSITION' attribute!"); // All models have POSITION

		const auto& positionAccessor = scene.accessors[positionAttribute->accessorIndex];
		const auto& indicesAccessor = scene.accessors[primitive.indicesAccessor.value()];

		verts.resize(positionAccessor.count);
		fastgltf::iterateAccessorWithIndex<fastgltf::math::fvec3>(scene, positionAccessor, [&](fastgltf::math::fvec3 pos, std::size_t idx) {
			verts[idx].position = rawrbox::Vector3f(pos.x(), pos.y(), pos.z());
		});
		// ------------

		// NORMALS ----
		const auto* normalAttribute = primitive.findAttribute("NORMAL");
		if (normalAttribute != nullptr) {
			const auto& normalAccessor = scene.accessors[normalAttribute->accessorIndex];
			fastgltf::iterateAccessorWithIndex<fastgltf::math::fvec3>(scene, normalAccessor, [&](fastgltf::math::fvec3 normal, std::size_t idx) {
				verts[idx].normal = rawrbox::PackUtils::packNormal(normal.x(), normal.y(), normal.z());
			});
		}
		// ------------

		// https://github.com/NVIDIA/OptiX_Apps/blob/82b27c9c035a08289acda4f19a8a80afa99f17a4/apps/GLTF_renderer/Application.cpp#L4438

		// TANGENTS ----
		/*const auto* tangentAttribute = primitive.findAttribute("TANGENT");
		if (tangentAttribute != nullptr) {
			const auto& tangentAccessor = scene.accessors[tangentAttribute->accessorIndex];

			fastgltf::iterateAccessorWithIndex<fastgltf::math::fvec3>(
			    scene, tangentAccessor,
			    [&](const fastgltf::math::fvec3& tangent, std::size_t idx) { verts[idx].tangent = rawrbox::PackUtils::packNormal(tangent.x(), tangent.y(), tangent.z()); });
		}*/
		//  ------------

		// UV -----
		const auto* uvIt = primitive.findAttribute("TEXCOORD_0");
		if (uvIt != nullptr) {
			const auto& uvAccessor = scene.accessors[uvIt->accessorIndex];
			fastgltf::iterateAccessorWithIndex<fastgltf::math::fvec2>(scene, uvAccessor, [&](fastgltf::math::fvec2 uv, std::size_t idx) {
				verts[idx].uv = rawrbox::Vector2f(uv.x(), uv.y());
			});
		}
		// -------------

		// JOINTS -----
		/*const auto* jointIt = primitive.findAttribute("JOINTS_");
		if (jointIt != nullptr) {
		}*/
		// -------------

		// WEIGHTS -----
		/*const auto* weightIt = primitive.findAttribute("WEIGHTS_");
		if (weightIt != nullptr) {
		}*/
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
	// ----------

	// -------------

	// PUBLIC -------

	GLTFImporter::GLTFImporter(uint32_t loadFlags) : loadFlags(loadFlags) {
		this->meshes.clear(); // Clear old meshes

		this->textures.clear();     // Clear old textures
		this->_texturesMap.clear(); // Clear old textures

		this->materials.clear(); // Clear old materials
	}

	GLTFImporter::~GLTFImporter() {
		this->_logger.reset();

		this->meshes.clear(); // Clear old meshes

		this->textures.clear();     // Clear old textures
		this->_texturesMap.clear(); // Clear old textures

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
