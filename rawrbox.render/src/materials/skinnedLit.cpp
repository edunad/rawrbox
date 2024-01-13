
#include <rawrbox/render/materials/skinnedLit.hpp>

namespace rawrbox {
	// STATIC DATA ----
	bool MaterialSkinnedLit::_built = false;
	// ----------------

	void MaterialSkinnedLit::init() {
		const std::string id = "Model::Skinned::Lit";

		if (!this->_built) {
			this->_logger->info("Building {} material..", fmt::format(fmt::fg(fmt::color::azure), id));

			Diligent::ShaderMacroHelper helper;
			helper.AddShaderMacro("MAX_BONES", rawrbox::MAX_BONES_PER_MODEL);
			helper.AddShaderMacro("NUM_BONES_PER_VERTEX", rawrbox::MAX_BONES_PER_VERTEX);
			helper.AddShaderMacro("SKINNED", true);

			this->createPipelines(id, vertexBufferType::vLayout(), helper);
			this->_built = true;
		}

		this->setupPipelines(id);
	}
} // namespace rawrbox
