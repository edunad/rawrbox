
#include <rawrbox/render/materials/text.hpp>
#include <rawrbox/render/utils/pipeline.hpp>

#include <MapHelper.hpp>

namespace rawrbox {
	// STATIC DATA ----
	bool MaterialText3D::_build = false;
	// ----------------

	void MaterialText3D::init() {
		if (!_build) {
			this->_logger->info("Building {} material..", fmt::format(fmt::fg(fmt::color::azure), "Model::3DText"));

			// PIPELINE ----
			rawrbox::PipeSettings settings;
			settings.pVS = "3dtext_unlit.vsh";
			settings.pPS = "3dtext_unlit.psh";
			settings.renderTargets = rawrbox::RENDER_TARGET_TARGETS; // COLOR + GPUPick
			settings.immutableSamplers = {{Diligent::SHADER_TYPE_PIXEL, "g_Texture"}};
			settings.cull = Diligent::CULL_MODE_FRONT;
			settings.layout = rawrbox::VertexData::vLayout();
			settings.signature = rawrbox::BindlessManager::signature; // Use bindless

			settings.fill = Diligent::FILL_MODE_WIREFRAME;
			rawrbox::PipelineUtils::createPipeline("3DText::Base::Wireframe", settings);

			settings.fill = Diligent::FILL_MODE_SOLID;
			settings.blending = {Diligent::BLEND_FACTOR_SRC_ALPHA, Diligent::BLEND_FACTOR_INV_SRC_ALPHA};
			rawrbox::PipelineUtils::createPipeline("3DText::Base", settings); // ALPHA by default on text

			_build = true;
		}

		if (this->_base == nullptr) this->_base = rawrbox::PipelineUtils::getPipeline("3DText::Base");
		if (this->_base_alpha == nullptr) this->_base_alpha = this->_base;
		if (this->_wireframe == nullptr) this->_wireframe = rawrbox::PipelineUtils::getPipeline("3DText::Base::Wireframe");
	}
} // namespace rawrbox
