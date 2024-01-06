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

		if (!this->_built) {
			fmt::print("[RawrBox-MaterialInstancedLit] Building material..\n");

			Diligent::ShaderMacroHelper helper;
			helper.AddShaderMacro("INSTANCED", true);

			this->createUniforms();
			this->createPipelines(id, vertexBufferType::vLayout(true), helper);

			this->_built = true;
		}

		this->setupPipelines(id);
	}
} // namespace rawrbox