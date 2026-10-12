
#include <rawrbox/render/plugins/post_process.hpp>
#include <rawrbox/render/post_process/base.hpp>
#include <rawrbox/render/static.hpp>
#include <rawrbox/render/utils/barrier.hpp>

namespace rawrbox {
	void PostProcessBase::init() {
		auto* plugin = rawrbox::RENDERER->getPlugin<rawrbox::PostProcessPlugin>("PostProcess");
		if (plugin == nullptr) RAWRBOX_CRITICAL("Post process plugin requires the 'PostProcess' renderer plugin!");

		this->_buffer = plugin->getBuffer();
	}

	void PostProcessBase::draw(Diligent::IPipelineState* pipeline, const rawrbox::TextureRender& target, const rawrbox::Vector4u& textureIDs) {
		if (pipeline == nullptr) RAWRBOX_CRITICAL("Post process pipeline not initialized!");

		auto* context = rawrbox::RENDERER->context();
		auto* targetRT = target.getRT();

		// BARRIER ----
		rawrbox::BarrierUtils::barrier({{target.getTexture(), Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_RENDER_TARGET, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE}});
		// --------
		
		context->SetRenderTargets(1, &targetRT, nullptr, Diligent::RESOURCE_STATE_TRANSITION_MODE_VERIFY);
		context->SetPipelineState(pipeline);

		// SETUP UNIFORMS ----------------------------
		{
			Diligent::MapHelper<rawrbox::BindlessPostProcessBuffer> CBConstants(context, this->_buffer, Diligent::MAP_WRITE, Diligent::MAP_FLAG_DISCARD);
			if (CBConstants == nullptr) RAWRBOX_CRITICAL("Failed to map the postprocess constants buffer!");

			CBConstants->data = this->_data;
			CBConstants->textureIDs = textureIDs;
		}
		// -----------

		// Draw quad
		Diligent::DrawAttribs DrawAttrs;
		DrawAttrs.NumVertices = 4;
		DrawAttrs.Flags = Diligent::DRAW_FLAG_VERIFY_ALL; // Verify the state of vertex and index buffers
		context->Draw(DrawAttrs);

		context->SetRenderTargets(0, nullptr, nullptr, Diligent::RESOURCE_STATE_TRANSITION_MODE_NONE);

		// BARRIER ----
		rawrbox::BarrierUtils::barrier({{target.getTexture(), Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_SHADER_RESOURCE, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE}});
		// --------
	}

	void PostProcessBase::applyEffect(const rawrbox::TextureBase& source, const rawrbox::TextureRender& target, uint32_t depthID) {
		if (!source.isValid()) RAWRBOX_CRITICAL("Effect texture not uploaded!");
		this->draw(this->_pipeline, target, {source.getTextureID(), depthID, 0, 0});
	}

	void PostProcessBase::setEnabled(bool enabled) { this->_enabled = enabled; }
	bool PostProcessBase::isEnabled() const { return this->_enabled; }
} // namespace rawrbox