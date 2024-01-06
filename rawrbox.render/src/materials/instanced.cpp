#include <rawrbox/render/materials/instanced.hpp>
#include <rawrbox/render/utils/pipeline.hpp>

namespace rawrbox {
	// STATIC DATA ----
	bool MaterialInstanced::_built = false;
	// ----------------

	void MaterialInstanced::init() {
		const std::string id = "Model::Instanced";

		if (!this->_built) {
			fmt::print("[RawrBox-MaterialInstanced] Building material..\n");

			Diligent::ShaderMacroHelper helper;
			helper.AddShaderMacro("INSTANCED", true);

			this->createUniforms();
			this->createPipelines(id, vertexBufferType::vLayout(true), helper);

			this->_built = true;
		}

		this->setupPipelines(id);
	}
} // namespace rawrbox
