#pragma once

#include <rawrbox/assimp/importer.hpp>
#include <rawrbox/render/materials/unlit.hpp>

#include <cpptrace/cpptrace.hpp>

namespace rawrbox {

	class AssimpUtils {
	public:
		template <typename M = MaterialUnlit>
		static rawrbox::Mesh<typename M::vertexBufferType> extractMesh(const rawrbox::AssimpImporter& model, size_t indx) {
			auto& meshes = model.meshes;
			if (meshes.empty() || indx < 0 || indx >= meshes.size()) throw cpptrace::logic_error(fmt::format("[RawrBox-AssimpUtils] Failed to extract mesh '{}'!", indx));

			auto& assimpMesh = meshes[indx];

			rawrbox::Mesh<typename M::vertexBufferType> mesh;
			mesh.name = assimpMesh.name;
			mesh.bbox = assimpMesh.bbox;
			mesh.matrix = assimpMesh.matrix;

			// Textures ---
			if (assimpMesh.material != nullptr) {
				auto mat = assimpMesh.material;
				mesh.setWireframe(mat->wireframe);
				mesh.setTransparentBlending(mat->alpha);
				mesh.setCulling(mat->doubleSided ? Diligent::CULL_MODE::CULL_MODE_NONE : Diligent::CULL_MODE::CULL_MODE_BACK);

				// DIFFUSE -----
				if (mat->diffuse.has_value()) {
					mesh.setTexture(mat->diffuse.value() == nullptr ? rawrbox::MISSING_TEXTURE.get() : mat->diffuse.value().get());
				} else {
					mesh.setTexture(rawrbox::WHITE_TEXTURE.get()); // Default
				}

				mesh.setColor(mat->diffuseColor);
				// --------

				// NORMAL -----
				if (mat->normal != nullptr) {
					mesh.setNormalTexture(mat->normal.get());
				}
				// --------

				// SPECULAR -----
				if (mat->specular != nullptr) {
					mesh.setSpecularTexture(mat->specular.get(), mat->shininess);
				}
				// --------

				// EMISSION -----
				if (mat->emissive != nullptr) {
					mesh.setEmissionTexture(mat->emissive.get(), mat->intensity);
				}
				// --------
			}
			// ------------

			mesh.indices = assimpMesh.indices;

			if constexpr (supportsNormals<typename M::vertexBufferType> && supportsBones<typename M::vertexBufferType>) {
				mesh.vertices = assimpMesh.vertices;
			} else {
				mesh.vertices.reserve(assimpMesh.vertices.size());
				for (auto& v : assimpMesh.vertices) {
					if constexpr (supportsNormals<typename M::vertexBufferType>) {
						mesh.vertices.push_back(rawrbox::VertexNormData(v.position, v.uv, v.normal, v.tangent, v.color));
					} else if constexpr (supportsBones<typename M::vertexBufferType>) {
						auto data = rawrbox::VertexBoneData(v.position, v.uv, v.color);
						data.bone_indices = v.bone_indices;
						data.bone_weights = v.bone_weights;

						mesh.vertices.push_back(data);
					} else {
						mesh.vertices.push_back(rawrbox::VertexData(v.position, v.uv, v.color));
					}
				}
			}

			if (mesh.vertices.empty()) throw cpptrace::logic_error(fmt::format("[RawrBox-Assimp] Failed to extract model '{}'", model.fileName.generic_string()));

			// Bones
			if (assimpMesh.skeleton != nullptr) {
				mesh.skeleton = assimpMesh.skeleton;
				mesh.setOptimizable(false);
			}
			// -------------------

			mesh.baseVertex = 0;
			mesh.baseIndex = 0;
			mesh.totalVertex = static_cast<uint16_t>(mesh.vertices.size());
			mesh.totalIndex = static_cast<uint16_t>(mesh.indices.size());

			return mesh;
		}
	};
} // namespace rawrbox