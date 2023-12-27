#include <rawrbox/render/plugins/post_process.hpp>
#include <rawrbox/render/static.hpp>

#include <fmt/format.h>

namespace rawrbox {
	PostProcessPlugin::~PostProcessPlugin() {
		this->_postProcesses.clear();
	}

	// Post utils ----
	void PostProcessPlugin::remove(size_t indx) {
		if (this->_postProcesses.empty() || indx >= this->_postProcesses.size()) throw std::runtime_error(fmt::format("[RawrBox-PostProcess] Failed to remove {}!", indx));
		this->_postProcesses.erase(this->_postProcesses.begin() + indx);
	}

	rawrbox::PostProcessBase& PostProcessPlugin::get(size_t indx) const {
		if (indx >= this->_postProcesses.size()) throw std::runtime_error(fmt::format("[RawrBox-PostProcess] Failed to get {}!", indx));
		return *this->_postProcesses[indx];
	}

	size_t PostProcessPlugin::count() {
		return this->_postProcesses.size();
	}
	// ----

	const std::string PostProcessPlugin::getID() const { return "PostProcess"; }

	void PostProcessPlugin::initialize(const rawrbox::Vector2i& size) {
		this->_rt = std::make_unique<rawrbox::TextureRender>(size, false);
		this->_rt->upload(Diligent::TEX_FORMAT_RGBA8_UNORM_SRGB);

		for (const auto& _postProcess : this->_postProcesses) {
			_postProcess->upload();
		}
	}

	void PostProcessPlugin::resize(const rawrbox::Vector2i& size) {
		// TODO: RESIZE RENDER TEXTURE?
	}

	void PostProcessPlugin::postRender(rawrbox::TextureRender* renderTexture) {
		if (this->_postProcesses.empty()) return;

		for (const auto& _postProcesse : this->_postProcesses) {
			this->_rt->startRecord(false);
			_postProcesse->applyEffect(renderTexture->getHandle());
			this->_rt->stopRecord();

			// Copy texture over
			Diligent::CopyTextureAttribs CopyAttribs{this->_rt->getTexture(), Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION,
			    renderTexture->getTexture(), Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION};
			rawrbox::RENDERER->context()->CopyTexture(CopyAttribs);
		}
	}

} // namespace rawrbox
