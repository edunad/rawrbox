
#include <rawrbox/render/materials/text.hpp>
#include <rawrbox/render/utils/pipeline.hpp>

#include <MapHelper.hpp>
// #include <Platforms/Basic/interface/DebugUtilities.hpp>

namespace rawrbox {
	// STATIC DATA ----
	Diligent::RefCntAutoPtr<Diligent::IBuffer> MaterialText3D::_uniforms;
	Diligent::RefCntAutoPtr<Diligent::IBuffer> MaterialText3D::_uniforms_pixel;

	bool MaterialText3D::_build = false;
	// ----------------

	void MaterialText3D::createUniforms() {
		if (_uniforms != nullptr || _uniforms_pixel != nullptr) return;

		// Uniforms -------
		{
			Diligent::BufferDesc CBDesc;
			CBDesc.Name = "rawrbox::MaterialText3D::Vertex::Uniforms";
			CBDesc.Size = sizeof(rawrbox::MaterialTextUniforms);
			CBDesc.Usage = Diligent::USAGE_DYNAMIC;
			CBDesc.BindFlags = Diligent::BIND_UNIFORM_BUFFER;
			CBDesc.CPUAccessFlags = Diligent::CPU_ACCESS_WRITE;

			rawrbox::RENDERER->device()->CreateBuffer(CBDesc, nullptr, &this->_uniforms);
		}
		// ------------

		// Pixel Uniforms -------
		{
			Diligent::BufferDesc CBDesc;
			CBDesc.Name = "rawrbox::MaterialText3D::Pixel::Uniforms";
			CBDesc.Size = sizeof(rawrbox::MaterialBasePixelUniforms);
			CBDesc.Usage = Diligent::USAGE_DYNAMIC;
			CBDesc.BindFlags = Diligent::BIND_UNIFORM_BUFFER;
			CBDesc.CPUAccessFlags = Diligent::CPU_ACCESS_WRITE;

			rawrbox::RENDERER->device()->CreateBuffer(CBDesc, nullptr, &this->_uniforms_pixel);
		}
		// ------------
	}

	void MaterialText3D::init() {
		if (!_build) {
			fmt::print("[RawrBox-MaterialText3D] Building material..\n");
			this->createUniforms();

			// PIPELINE ----
			rawrbox::PipeSettings settings;
			settings.pVS = "3dtext_unlit.vsh";
			settings.pPS = "3dtext_unlit.psh";
			settings.immutableSamplers = {{Diligent::SHADER_TYPE_PIXEL, "g_Texture"}};
			settings.cull = Diligent::CULL_MODE_FRONT;
			settings.layout = rawrbox::VertexData::vLayout();
			settings.signature = rawrbox::PipelineUtils::signature;

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
