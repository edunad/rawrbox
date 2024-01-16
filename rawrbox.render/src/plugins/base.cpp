
#include <rawrbox/render/plugins/base.hpp>

namespace rawrbox {
	// UTILS --------
	void RenderPlugin::setEnabled(bool enabled) { this->_enabled = enabled; }
	bool RenderPlugin::isEnabled() const { return this->_enabled; }

	const std::string RenderPlugin::getID() const { throw this->_logger->error("Not implemented"); }
	// --------------

	void RenderPlugin::requirements(Diligent::DeviceFeatures& /*features*/) {}
	void RenderPlugin::signatures(std::vector<Diligent::PipelineResourceDesc>& /*sig*/, bool /*comput*/) {}
	void RenderPlugin::bind(Diligent::IPipelineResourceSignature& /*sig*/, bool /*compute*/) {}

	void RenderPlugin::initialize(const rawrbox::Vector2i& /*renderSize*/) {}
	void RenderPlugin::resize(const rawrbox::Vector2i& /*renderSize*/) {}
	void RenderPlugin::upload() {}

	void RenderPlugin::preRender() {}
	void RenderPlugin::postRender(rawrbox::TextureRender& /*renderTarget*/) {}

	void RenderPlugin::update() {}
} // namespace rawrbox
