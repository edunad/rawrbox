#include <rawrbox/render/materials/skinned.hpp>
#include <rawrbox/render/utils/pipeline.hpp>

namespace rawrbox {
	// STATIC DATA ----
	bool MaterialSkinned::_built = false;
	// ----------------

	void MaterialSkinned::init() {
		const std::string id = "Model::Skinned";

		if (!rawrbox::MaterialSkinned::_built) {
			this->_logger->info("Building {} material..", fmt::styled(id, fmt::fg(fmt::color::azure)));

			Diligent::ShaderMacroHelper helper;
			helper.AddShaderMacro("SKINNED", true);

			this->createPipelines(id, vertexBufferType::vLayout(), helper);
			rawrbox::MaterialSkinned::_built = true;
		}

		this->setupPipelines(id);
	}
} // namespace rawrbox
