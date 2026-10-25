
#include <rawrbox/render/plugins/post_process.hpp>
#include <rawrbox/render/post_process/base.hpp>
#include <rawrbox/render/static.hpp>
#include <rawrbox/render/utils/barrier.hpp>
#include <rawrbox/render/utils/pipeline.hpp>

namespace rawrbox {
	void PostProcessBase::init() {
		const auto* plugin = rawrbox::RENDERER->getPlugin<rawrbox::PostProcessPlugin>("PostProcess");
		if (plugin == nullptr) RAWRBOX_CRITICAL("Post process plugin requires the 'PostProcess' renderer plugin!");

		this->_buffer = plugin->getBuffer();
	}

	Diligent::IPipelineState* PostProcessBase::createPipeline(const std::string& id, const std::string& pixelShader, const Diligent::ShaderMacroHelper& macros) {
		rawrbox::PipeSettings settings;
		
		settings.depth = Diligent::COMPARISON_FUNC_UNKNOWN;
		settings.depthWrite = false;
		settings.depthFormat = false;
		settings.blending = {.src = Diligent::BLEND_FACTOR_UNDEFINED, .dest = Diligent::BLEND_FACTOR_UNDEFINED};
		settings.cull = Diligent::CULL_MODE_NONE;
		settings.topology = Diligent::PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
		settings.pVS = "quad/quad.vsh";
		settings.pPS = pixelShader;
		settings.macros = macros;
		settings.signatures = {rawrbox::BindlessManager::signature};

		return rawrbox::PipelineUtils::createPipeline(id, settings);
	}

	void PostProcessBase::draw(Diligent::IPipelineState* pipeline, const rawrbox::TextureRender& target, const rawrbox::Vector4u& textureIDs) {
		if (pipeline == nullptr) RAWRBOX_CRITICAL("Post process pipeline not initialized!");

		auto* context = rawrbox::RENDERER->context();
		auto* targetRT = target.getRT();

		// SETUP UNIFORMS ----------------------------
		const rawrbox::BindlessPostProcessBuffer constants = {.data = this->_data, .textureIDs = textureIDs};
		context->UpdateBuffer(this->_buffer, 0, sizeof(rawrbox::BindlessPostProcessBuffer), &constants, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

		const Diligent::StateTransitionDesc constantsBarrier = {this->_buffer, Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_CONSTANT_BUFFER, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE};
		context->TransitionResourceStates(1, &constantsBarrier);
		// -----------

		// BARRIER ----
		rawrbox::BarrierUtils::barrier({{target.getTexture(), Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_RENDER_TARGET, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE}});
		// --------

		context->SetRenderTargets(1, &targetRT, nullptr, Diligent::RESOURCE_STATE_TRANSITION_MODE_VERIFY);
		context->SetPipelineState(pipeline);

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