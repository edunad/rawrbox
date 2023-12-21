
#include <rawrbox/render/passes/base.hpp>

#include <stdexcept>

namespace rawrbox {
	// UTILS --------
	void RenderPass::setEnabled(bool enabled) { this->_enabled = enabled; }
	bool RenderPass::isEnabled() const { return this->_enabled; }

	// const rawrbox::RenderPassType RenderPass::getType() const { throw std::runtime_error("[RenderPass] Not implemented"); }
	const std::string RenderPass::getID() const { throw std::runtime_error("[RenderPass] Not implemented"); }
	// --------------

	// PASS ---
	const std::vector<rawrbox::RenderPassAttachments>& RenderPass::getPasses() const { return this->_passes; }
	void RenderPass::addPass(const rawrbox::RenderPassAttachments& attach) {
		this->_passes.push_back(attach);
	}

	const std::vector<std::string>& RenderPass::getDependencies() const { return this->_dependencies; }
	void RenderPass::addDependency(const std::string& id) {
		this->_dependencies.push_back(id);
	}
	// --------

	void RenderPass::initialize(const rawrbox::Vector2i& renderSize) {}
	void RenderPass::resize(const rawrbox::Vector2i& renderSize) {}

	void RenderPass::render(const rawrbox::DrawPass& pass) {}
	void RenderPass::update() {}
} // namespace rawrbox
