#include <rawrbox/render/materials/base.hpp>

namespace rawrbox {
	void MaterialBase::createPipelines(const std::string& /*id*/, const std::vector<Diligent::LayoutElement>& /*layout*/, Diligent::IBuffer* /*uniforms*/, Diligent::IBuffer* /*pixelUniforms*/, Diligent::ShaderMacroHelper /*helper*/) {}
	void MaterialBase::setupPipelines(const std::string& id) {
		if (this->_base == nullptr) this->_base = rawrbox::PipelineUtils::getPipeline(id);
		if (this->_base_alpha == nullptr) this->_base_alpha = rawrbox::PipelineUtils::getPipeline(id + "::Alpha");

		if (this->_line == nullptr) this->_line = rawrbox::PipelineUtils::getPipeline(id + "::Line");

		if (this->_cullback == nullptr) this->_cullback = rawrbox::PipelineUtils::getPipeline(id + "::CullBack");
		if (this->_cullback_alpha == nullptr) this->_cullback_alpha = rawrbox::PipelineUtils::getPipeline(id + "::CullBack::Alpha");

		if (this->_wireframe == nullptr) this->_wireframe = rawrbox::PipelineUtils::getPipeline(id + "::Wireframe");

		if (this->_cullnone == nullptr) this->_cullnone = rawrbox::PipelineUtils::getPipeline(id + "::CullNone");
		if (this->_cullnone_alpha == nullptr) this->_cullnone_alpha = rawrbox::PipelineUtils::getPipeline(id + "::CullNone::Alpha");

		if (this->_bind == nullptr) {
			this->_bind = rawrbox::PipelineUtils::getBind(id);

			auto textureVar = this->_bind->GetVariableByName(Diligent::SHADER_TYPE_PIXEL, "g_Textures");
			if (textureVar == nullptr) throw std::runtime_error("[RawrBox-MaterialBase] Variable 'g_Textures' not found on PIXEL_SHADER!");

			textureVar->SetArray(rawrbox::TextureManager::getHandles().data(), 0, static_cast<uint32_t>(rawrbox::TextureManager::total()), Diligent::SET_SHADER_RESOURCE_FLAG_ALLOW_OVERWRITE);
			rawrbox::TextureManager::onUpdate += [textureVar]() {
				textureVar->SetArray(rawrbox::TextureManager::getHandles().data(), 0, static_cast<uint32_t>(rawrbox::TextureManager::total()), Diligent::SET_SHADER_RESOURCE_FLAG_ALLOW_OVERWRITE);
			};
		}
	}

	void MaterialBase::bindShaderResources() const {
		auto context = rawrbox::RENDERER->context();

		if (this->_bind == nullptr) throw std::runtime_error("[RawrBox-MaterialBase] Bind not set!");
		context->CommitShaderResources(this->_bind, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
	}
} // namespace rawrbox
