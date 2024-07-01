#include <rawrbox/render/lights/manager.hpp>
#include <rawrbox/render/materials/instancedLit.hpp>
#include <rawrbox/render/plugins/clustered.hpp>
#include <rawrbox/render/utils/pipeline.hpp>

namespace rawrbox {
	// STATIC DATA ----
	bool MaterialInstancedLit::_built = false;
	// ----------------

	void MaterialInstancedLit::init() {
		const std::string id = "Model::Instanced::Lit";

		if (!rawrbox::MaterialInstancedLit::_built) {
			this->_logger->info("Building {} material..", fmt::styled(id, fmt::fg(fmt::color::azure)));

			Diligent::ShaderMacroHelper helper;
			helper.Add("INSTANCED", true);

			this->createPipelines(id, vertexBufferType::vLayout(true), helper);
			rawrbox::MaterialInstancedLit::_built = true;
		}

		this->setupPipelines(id);
	}
} // namespace rawrbox
