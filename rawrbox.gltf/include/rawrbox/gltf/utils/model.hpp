#pragma once

#include <rawrbox/gltf/importer.hpp>
#include <rawrbox/render/materials/unlit.hpp>

namespace rawrbox {

	class GLTFUtils {
	public:
		template <typename M = rawrbox::MaterialUnlit>
			requires(std::derived_from<M, rawrbox::MaterialBase>)
		static rawrbox::Mesh<typename M::vertexBufferType> extractMesh(const rawrbox::GLTFMesh& gltfMesh, const rawrbox::GLTFPrimitive& primitive) {
			rawrbox::Mesh<typename M::vertexBufferType> mesh;

			mesh.name = gltfMesh.name;
			mesh.bbox = gltfMesh.bbox;
			mesh.matrix = gltfMesh.matrix;

			// Textures ---
			if (primitive.material != nullptr) {
				auto* mat = primitive.material;

				mesh.setAlphaCutoff(mat->alphaCutoff);
				mesh.setTransparent(mat->transparent);
				mesh.setCulling(mat->doubleSided ? Diligent::CULL_MODE::CULL_MODE_NONE : Diligent::CULL_MODE::CULL_MODE_BACK);

				// DIFFUSE -----
				if (mat->diffuse != nullptr) {
					mesh.setTexture(mat->diffuse);
				} else {
					mesh.setTexture(rawrbox::MISSING_TEXTURE.get());
				}

				mesh.setColor(mat->baseColor);
				// --------

				// NORMAL -----
				if (mat->normal != nullptr) {
					mesh.setNormalTexture(mat->normal);
				}
				// --------

				// METAL / ROUGH -----
				if (mat->metalRough != nullptr) {
					mesh.setRoughtMetalTexture(mat->metalRough, mat->roughnessFactor, mat->metalnessFactor);
				}
				// --------

				// EMISSION -----
				if (mat->emissive != nullptr) {
					mesh.setEmissionTexture(mat->emissive, mat->emissionFactor);
				}
				// --------
			}
			// ------------

			mesh.indices = primitive.indices;

			if constexpr (supportsNormals<typename M::vertexBufferType> && supportsBones<typename M::vertexBufferType>) {
				mesh.vertices = primitive.vertices;
			} else {
				mesh.vertices.reserve(primitive.vertices.size());

				for (const auto& v : primitive.vertices) {
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

			mesh.baseVertex = 0;
			mesh.baseIndex = 0;
			mesh.totalVertex = static_cast<uint32_t>(mesh.vertices.size());
			mesh.totalIndex = static_cast<uint32_t>(mesh.indices.size());

			// Bones
			if (gltfMesh.skeleton != nullptr) {
				mesh.skeleton = gltfMesh.skeleton;
				mesh.setMergeable(false); // Don't merge meshes
			}
			// -------------------

			return mesh;
		}
	};
} // namespace rawrbox
