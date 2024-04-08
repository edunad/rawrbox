
#include <rawrbox/render/plugins/base.hpp>

namespace rawrbox {
	// UTILS --------
	void RenderPlugin::setEnabled(bool enabled) { this->_enabled = enabled; }
	bool RenderPlugin::isEnabled() const { return this->_enabled; }
	// --------------

	void RenderPlugin::requirements(Diligent::DeviceFeatures& /*features*/) {}

	void RenderPlugin::signatures(std::vector<Diligent::PipelineResourceDesc>& /*sig*/) {}
	void RenderPlugin::bindStatic(Diligent::IPipelineResourceSignature& /*sig*/) {}
	void RenderPlugin::bindMutable(Diligent::IShaderResourceBinding& /*bind*/) {}

	void RenderPlugin::initialize(const rawrbox::Vector2u& /*renderSize*/) {}
	void RenderPlugin::resize(const rawrbox::Vector2u& /*renderSize*/) {}
	void RenderPlugin::upload() {}

	void RenderPlugin::preRender() {}
	void RenderPlugin::postRender(rawrbox::TextureRender& /*renderTarget*/) {}

	void RenderPlugin::update() {}
} // namespace rawrbox
