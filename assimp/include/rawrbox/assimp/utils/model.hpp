#pragma once

#include <rawrbox/assimp/importer.hpp>
#include <rawrbox/render/materials/base.hpp>

namespace rawrbox {

	class AssimpUtils {
	public:
		static rawrbox::Mesh extractMesh(const rawrbox::AssimpImporter& model, size_t indx);
	};
} // namespace rawrbox
