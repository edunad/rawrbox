#include <rawrbox/render/materials/skinned.hpp>
#include <rawrbox/render/utils/pipeline.hpp>

namespace rawrbox {
	// STATIC DATA ----
	bool MaterialSkinned::_built = false;
#ifdef _DEBUG
	bool MaterialSkinned::DEBUG_MODE = false;
#endif
	// ----------------

	void MaterialSkinned::init() {
		const std::string id = "Model::Skinned";

		if (!rawrbox::MaterialSkinned::_built) {
			this->_logger->info("Building {} material..", fmt::styled(id, fmt::fg(fmt::color::azure)));

			Diligent::ShaderMacroHelper helper;
			helper.AddShaderMacro("SKINNED", true);

#ifdef _DEBUG
			if (rawrbox::MaterialSkinned::DEBUG_MODE) helper.AddShaderMacro("SHADER_DEBUG", true);
#endif

			this->createPipelines(id, vertexBufferType::vLayout(), helper);
			rawrbox::MaterialSkinned::_built = true;
		}

		this->setupPipelines(id);
	}
} // namespace rawrbox
