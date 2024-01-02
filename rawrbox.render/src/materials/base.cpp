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

		if (this->_bind == nullptr) this->_bind = rawrbox::PipelineUtils::getBind(id);
	}

	void MaterialBase::bindShaderResources() {
		auto context = rawrbox::RENDERER->context();

		if (this->_bind == nullptr) throw std::runtime_error("[RawrBox-MaterialBase] Bind not set!");
		context->CommitShaderResources(this->_bind, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
	}

} // namespace rawrbox
