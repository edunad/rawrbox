#include <rawrbox/render/materials/base.hpp>

namespace rawrbox {
	void MaterialBase::createPipelines(const std::string& /*id*/, const std::vector<Diligent::LayoutElement>& /*layout*/, const Diligent::ShaderMacroHelper& /*helper*/) {}
	void MaterialBase::setupPipelines(const std::string& id) {
		if (this->base == nullptr) this->base = rawrbox::PipelineUtils::getPipeline(id);
		if (this->base_alpha == nullptr) this->base_alpha = rawrbox::PipelineUtils::getPipeline(id + "::Alpha");

		if (this->line == nullptr) this->line = rawrbox::PipelineUtils::getPipeline(id + "::Line");

		if (this->cullback == nullptr) this->cullback = rawrbox::PipelineUtils::getPipeline(id + "::CullBack");
		if (this->cullback_alpha == nullptr) this->cullback_alpha = rawrbox::PipelineUtils::getPipeline(id + "::CullBack::Alpha");

		if (this->wireframe == nullptr) this->wireframe = rawrbox::PipelineUtils::getPipeline(id + "::Wireframe");

		if (this->cullnone == nullptr) this->cullnone = rawrbox::PipelineUtils::getPipeline(id + "::CullNone");
		if (this->cullnone_alpha == nullptr) this->cullnone_alpha = rawrbox::PipelineUtils::getPipeline(id + "::CullNone::Alpha");
	}

	void MaterialBase::resetUniformBinds() {
		this->_lastPixelBuffer.reset();
		this->_lastVertexBuffer.reset();
		this->_lastSkinnedVertexBuffer.reset();
	}
} // namespace rawrbox
