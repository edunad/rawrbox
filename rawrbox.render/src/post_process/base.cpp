
#include <rawrbox/render/plugins/post_process.hpp>
#include <rawrbox/render/post_process/base.hpp>
#include <rawrbox/render/static.hpp>

namespace rawrbox {
	void PostProcessBase::init() {
		auto* plugin = rawrbox::RENDERER->getPlugin<rawrbox::PostProcessPlugin>("PostProcess");
		if (plugin == nullptr) throw this->_logger->error("Post process plugin requires the 'PostProcess' renderer plugin!");

		this->_buffer = plugin->getBuffer();
	}

	void PostProcessBase::applyEffect(const rawrbox::TextureBase& texture) {
		if (!texture.isValid()) throw this->_logger->error("Effect texture not uploaded!");

		auto* context = rawrbox::RENDERER->context();
		context->SetPipelineState(this->_pipeline);

		// SETUP UNIFORMS ----------------------------
		{
			Diligent::MapHelper<rawrbox::BindlessPostProcessBuffer> CBConstants(context, this->_buffer, Diligent::MAP_WRITE, Diligent::MAP_FLAG_DISCARD);
			if (CBConstants == nullptr) throw _logger->error("Failed to map the postprocess constants buffer!");

			CBConstants->data = this->_data;
			CBConstants->textureID = texture.getTextureID();
			CBConstants->depthTextureID = texture.getDepthTextureID();
		}
		// -----------

		// Draw quad
		Diligent::DrawAttribs DrawAttrs;
		DrawAttrs.NumVertices = 4;
		DrawAttrs.Flags = Diligent::DRAW_FLAG_VERIFY_ALL; // Verify the state of vertex and index buffers
		context->Draw(DrawAttrs);
	}
} // namespace rawrbox
