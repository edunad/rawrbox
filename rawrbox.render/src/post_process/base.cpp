
#include <rawrbox/render/plugins/post_process.hpp>
#include <rawrbox/render/post_process/base.hpp>
#include <rawrbox/render/static.hpp>

namespace rawrbox {
	void PostProcessBase::init() {
		auto* plugin = rawrbox::RENDERER->getPlugin<rawrbox::PostProcessPlugin>("PostProcess");
		if (plugin == nullptr) RAWRBOX_CRITICAL("Post process plugin requires the 'PostProcess' renderer plugin!");

		this->_buffer = plugin->getBuffer();
	}

	void PostProcessBase::applyEffect(const rawrbox::TextureBase& texture) {
		if (!texture.isValid()) RAWRBOX_CRITICAL("Effect texture not uploaded!");

		auto* context = rawrbox::RENDERER->context();
		context->SetPipelineState(this->_pipeline);

		// SETUP UNIFORMS ----------------------------
		{
			Diligent::MapHelper<rawrbox::BindlessPostProcessBuffer> CBConstants(context, this->_buffer, Diligent::MAP_WRITE, Diligent::MAP_FLAG_DISCARD);
			if (CBConstants == nullptr) RAWRBOX_CRITICAL("Failed to map the postprocess constants buffer!");

			CBConstants->data = this->_data;
			CBConstants->textureIDs = {texture.getTextureID(), texture.getDepthTextureID(), 0, 0};
		}
		// -----------

		// Draw quad
		Diligent::DrawAttribs DrawAttrs;
		DrawAttrs.NumVertices = 4;
		DrawAttrs.Flags = Diligent::DRAW_FLAG_VERIFY_ALL; // Verify the state of vertex and index buffers
		context->Draw(DrawAttrs);
	}

	void PostProcessBase::setEnabled(bool enabled) { this->_enabled = enabled; }
	bool PostProcessBase::isEnabled() const { return this->_enabled; }
} // namespace rawrbox
