
#include <rawrbox/assimp/utils/model.hpp>

namespace rawrbox {
	rawrbox::Mesh AssimpUtils::extractMesh(const rawrbox::AssimpImporter& model, size_t indx) {
		auto& meshes = model.meshes;
		if (meshes.empty() || indx < 0 || indx >= meshes.size()) throw std::runtime_error(fmt::format("[RawrBox-AssimpUtils] Failed to extract mesh '{}'!", indx));

		auto& assimpMesh = meshes[indx];

		rawrbox::Mesh mesh;
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
		mesh.vertices = assimpMesh.vertices;

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
} // namespace rawrbox
