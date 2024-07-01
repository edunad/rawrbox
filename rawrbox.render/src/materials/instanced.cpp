#include <rawrbox/render/materials/instanced.hpp>
#include <rawrbox/render/utils/pipeline.hpp>

namespace rawrbox {
	// STATIC DATA ----
	bool MaterialInstanced::_built = false;
	// ----------------

	void MaterialInstanced::init() {
		const std::string id = "Model::Instanced";

		if (!rawrbox::MaterialInstanced::_built) {
			this->_logger->info("Building {} material..", fmt::styled(id, fmt::fg(fmt::color::azure)));

			Diligent::ShaderMacroHelper helper;
			helper.Add("INSTANCED", true);

			this->createPipelines(id, vertexBufferType::vLayout(true), helper);
			rawrbox::MaterialInstanced::_built = true;
		}

		this->setupPipelines(id);
	}
} // namespace rawrbox
