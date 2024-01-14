#include <rawrbox/render/plugins/post_process.hpp>
#include <rawrbox/render/static.hpp>

namespace rawrbox {
	PostProcessPlugin::~PostProcessPlugin() {
		this->_postProcesses.clear();
	}

	// Post utils ----
	void PostProcessPlugin::remove(size_t indx) {
		if (this->_postProcesses.empty() || indx >= this->_postProcesses.size()) throw this->_logger->error("Failed to remove {}!", indx);
		this->_postProcesses.erase(this->_postProcesses.begin() + indx);
	}

	rawrbox::PostProcessBase& PostProcessPlugin::get(size_t indx) const {
		if (indx >= this->_postProcesses.size()) throw this->_logger->error("Failed to get {}!", indx);
		return *this->_postProcesses[indx];
	}

	size_t PostProcessPlugin::count() {
		return this->_postProcesses.size();
	}
	// ----

	const std::string PostProcessPlugin::getID() const { return "PostProcess"; }

	void PostProcessPlugin::initialize(const rawrbox::Vector2i& /*size*/) {
		for (const auto& _postProcess : this->_postProcesses) {
			_postProcess->init();
		}
	}

	void PostProcessPlugin::resize(const rawrbox::Vector2i& /*size*/) {
		// TODO: RESIZE RENDER TEXTURE?
	}

	void PostProcessPlugin::postRender(rawrbox::TextureRender& renderTexture) {
		for (const auto& process : this->_postProcesses) {
			renderTexture.startRecord(false);
			process->applyEffect(renderTexture);
			renderTexture.stopRecord();
		}
	}

} // namespace rawrbox
