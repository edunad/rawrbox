#include <rawrbox/render/materials/base.hpp>

namespace rawrbox {
	void MaterialBase::createPipelines(const std::string& /*id*/, const std::vector<Diligent::LayoutElement>& /*layout*/, Diligent::ShaderMacroHelper /*helper*/) {}
	void MaterialBase::setupPipelines(const std::string& id) {
		if (this->_base == nullptr) this->_base = rawrbox::PipelineUtils::getPipeline(id);
		if (this->_base_alpha == nullptr) this->_base_alpha = rawrbox::PipelineUtils::getPipeline(id + "::Alpha");

		if (this->_line == nullptr) this->_line = rawrbox::PipelineUtils::getPipeline(id + "::Line");

		if (this->_cullback == nullptr) this->_cullback = rawrbox::PipelineUtils::getPipeline(id + "::CullBack");
		if (this->_cullback_alpha == nullptr) this->_cullback_alpha = rawrbox::PipelineUtils::getPipeline(id + "::CullBack::Alpha");

		if (this->_wireframe == nullptr) this->_wireframe = rawrbox::PipelineUtils::getPipeline(id + "::Wireframe");

		if (this->_cullnone == nullptr) this->_cullnone = rawrbox::PipelineUtils::getPipeline(id + "::CullNone");
		if (this->_cullnone_alpha == nullptr) this->_cullnone_alpha = rawrbox::PipelineUtils::getPipeline(id + "::CullNone::Alpha");
	}

	void MaterialBase::bindShaderResources() const {
		if (rawrbox::PipelineUtils::signatureBind == nullptr) throw std::runtime_error("[RawrBox-MaterialBase] Signature bind not initialized!");

		auto context = rawrbox::RENDERER->context();
		context->CommitShaderResources(rawrbox::PipelineUtils::signatureBind, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
	}
} // namespace rawrbox
