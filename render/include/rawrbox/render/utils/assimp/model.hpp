#pragma once

#include <rawrbox/render/model/assimp/assimp_importer.hpp>
#include <rawrbox/render/model/material/base.hpp>

namespace rawrbox {

	class AssimpUtils {
	public:
		// ---
		template <typename M = rawrbox::MaterialBase>
		static rawrbox::Mesh<typename M::vertexBufferType> extractMesh(const rawrbox::AssimpImporter& model, size_t indx) {
			auto& meshes = model.meshes;
			if (meshes.empty() || indx < 0 || indx >= meshes.size()) throw std::runtime_error(fmt::format("[RawrBox-AssimpUtils] Failed to extract mesh '{}'!", indx));

			auto& assimpMesh = meshes[indx];

			rawrbox::Mesh<typename M::vertexBufferType> mesh;
			mesh.name = assimpMesh.name;
			mesh.bbox = assimpMesh.bbox;
			mesh.matrix = assimpMesh.matrix;

			// Textures ---
			if (assimpMesh.material != nullptr) {
				auto mat = assimpMesh.material;
				mesh.setWireframe(mat->wireframe);
				mesh.setBlend(mat->blending);
				mesh.setCulling(mat->doubleSided ? 0 : BGFX_STATE_CULL_CCW);

				// DIFFUSE -----
				if (mat->diffuse.has_value()) {
					mesh.setTexture(mat->diffuse.value() == nullptr ? rawrbox::MISSING_TEXTURE.get() : mat->diffuse.value().get());
				} else {
					mesh.setTexture(rawrbox::WHITE_TEXTURE.get()); // Default
				}

				mesh.setColor(mat->diffuseColor);
				// --------

				if constexpr (supportsNormals<typename M::vertexBufferType>) {
					// SPECULAR -----
					if (mat->specular != nullptr) {
						mesh.setSpecularTexture(mat->specular.get(), mat->shininess);
					}

					mesh.setSpecularColor(mat->specularColor);
					// --------

					// EMISSION -----
					if (mat->emissive != nullptr) {
						mesh.setEmissionTexture(mat->emissive.get(), mat->intensity);
					}

					mesh.setEmissionColor(mat->emissionColor);
					// --------

					// OPACITY -----
					if (mat->opacity != nullptr) {
						mesh.setOpacityTexture(mat->opacity.get());
					}
					// --------
				}
				// --------
			}
			// ------------

			mesh.indices = assimpMesh.indices;
			for (auto& vert : assimpMesh.vertices) {
				typename M::vertexBufferType m;
				m.position = vert.position;
				m.uv = vert.uv;
				m.abgr = vert.abgr;

				if constexpr (supportsNormals<typename M::vertexBufferType>) {
					m.normal = vert.normal;
				}

				if constexpr (supportsBones<typename M::vertexBufferType>) {
					m.index = vert.index;
					m.bone_indices = vert.bone_indices;
					m.bone_weights = vert.bone_weights;
				}

				mesh.vertices.push_back(m);
			}

			// Bones
			if constexpr (supportsBones<typename M::vertexBufferType>) {
				if (assimpMesh.skeleton != nullptr) {
					mesh.skeleton = assimpMesh.skeleton;
					mesh.setOptimizable(false);
				}
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
