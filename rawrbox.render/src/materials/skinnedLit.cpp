
#include <rawrbox/render/materials/skinnedLit.hpp>

namespace rawrbox {
	// STATIC DATA ----
	bool MaterialSkinnedLit::_built = false;
	// ----------------

	void MaterialSkinnedLit::init() {
		const std::string id = "Model::Skinned::Lit";

		if (!rawrbox::MaterialSkinnedLit::_built) {
			this->_logger->info("Building {} material..", fmt::format(fmt::fg(fmt::color::azure), id));

			Diligent::ShaderMacroHelper helper;
			helper.AddShaderMacro("SKINNED", true);

			this->createPipelines(id, vertexBufferType::vLayout(), helper);
			rawrbox::MaterialSkinnedLit::_built = true;
		}

		this->setupPipelines(id);
	}
} // namespace rawrbox
