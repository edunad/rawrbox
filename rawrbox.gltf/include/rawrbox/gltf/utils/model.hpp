#pragma once

#include <rawrbox/gltf/importer.hpp>
#include <rawrbox/render/materials/unlit.hpp>

namespace rawrbox {

	class GLTFUtils {
	public:
		template <typename M = rawrbox::MaterialUnlit>
			requires(std::derived_from<M, rawrbox::MaterialBase>)
		static rawrbox::Mesh<typename M::vertexBufferType> extractMesh(const rawrbox::GLTFImporter& model, size_t indx) {
			const auto& meshes = model.meshes;
			if (meshes.empty() || indx >= meshes.size()) throw rawrbox::Logger::err("RawrBox-GLTFUtils", "Failed to extract mesh '{}'!", indx);

			const auto& gltfMesh = meshes[indx];

			rawrbox::Mesh<typename M::vertexBufferType> mesh;
			mesh.name = gltfMesh.name;
			mesh.bbox = gltfMesh.bbox;
			mesh.matrix = gltfMesh.matrix;

			// Textures ---
			if (gltfMesh.material != nullptr) {
				auto* mat = gltfMesh.material;

				mesh.setTransparent(mat->alpha);
				mesh.setCulling(mat->doubleSided ? Diligent::CULL_MODE::CULL_MODE_NONE : Diligent::CULL_MODE::CULL_MODE_BACK);

				// DIFFUSE -----
				if (mat->diffuse != nullptr) {
					mesh.setTexture(mat->diffuse);
				} else {
					mesh.setTexture(rawrbox::MISSING_TEXTURE.get());
				}

				mesh.setColor(mat->baseColor);
				// --------

				/*mesh.setWireframe(mat->wireframe);
				mesh.setTransparent(mat->alpha);
				mesh.setCulling(mat->doubleSided ? Diligent::CULL_MODE::CULL_MODE_NONE : Diligent::CULL_MODE::CULL_MODE_BACK);

				// DIFFUSE -----
				if (mat->diffuse.has_value()) {
					mesh.setTexture(mat->diffuse.value() == nullptr ? rawrbox::MISSING_TEXTURE.get() : mat->diffuse.value().get());
				} else {
					mesh.setTexture(rawrbox::WHITE_TEXTURE.get()); // Default
				}

				mesh.setColor(mat->baseColor);
				// --------

				// NORMAL -----
				if (mat->normal != nullptr) {
					mesh.setNormalTexture(mat->normal.get());
				}
				// --------

				// METAL / ROUGH -----
				if (mat->metalRough != nullptr) {
					mesh.setRoughtMetalTexture(mat->metalRough.get(), mat->roughnessFactor, mat->metalnessFactor);
				}
				// --------

				// EMISSION -----
				if (mat->emissive != nullptr) {
					mesh.setEmissionTexture(mat->emissive.get(), mat->emissionFactor);
				}
				// --------*/
			} else {
				mesh.setColor(gltfMesh.color);
			}
			// ------------

			mesh.indices = gltfMesh.indices;

			if constexpr (supportsNormals<typename M::vertexBufferType> && supportsBones<typename M::vertexBufferType>) {
				mesh.vertices = gltfMesh.vertices;
			} else {
				mesh.vertices.reserve(gltfMesh.vertices.size());

				for (const auto& v : gltfMesh.vertices) {
					if constexpr (supportsNormals<typename M::vertexBufferType>) {
						mesh.vertices.push_back(rawrbox::VertexNormData(v.position, v.uv, v.normal, v.tangent));
					} else if constexpr (supportsBones<typename M::vertexBufferType>) {
						auto data = rawrbox::VertexBoneData(v.position, v.uv);
						data.bone_indices = v.bone_indices;
						data.bone_weights = v.bone_weights;

						mesh.vertices.push_back(data);
					} else if constexpr (supportsUVs<typename M::vertexBufferType>) {
						mesh.vertices.push_back(rawrbox::VertexUVData(v.position, v.uv));
					} else {
						mesh.vertices.push_back(rawrbox::VertexData(v.position));
					}
				}
			}

			if (mesh.vertices.empty()) throw rawrbox::Logger::err("RawrBox-GLTFUtils", "Failed to extract model '{}'!", model.filePath.generic_string());

			// Bones
			/*if (gltfMesh.skeleton != nullptr) {
				mesh.skeleton = gltfMesh.skeleton;
				mesh.setOptimizable(false);
			}*/
			// -------------------

			mesh.baseVertex = 0;
			mesh.baseIndex = 0;
			mesh.totalVertex = static_cast<uint16_t>(mesh.vertices.size());
			mesh.totalIndex = static_cast<uint16_t>(mesh.indices.size());

			return mesh;
		}
	};
} // namespace rawrbox
