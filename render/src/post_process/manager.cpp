
#include <rawrbox/render/post_process/manager.hpp>
#include <rawrbox/render/static.hpp>
#include <rawrbox/render/utils/render.hpp>

#include <fmt/format.h>

#include <stdexcept>

namespace rawrbox {

	PostProcessManager::~PostProcessManager() {
		this->_postProcesses.clear();
	}

	// Post utils ----
	void PostProcessManager::remove(size_t indx) {
		if (this->_postProcesses.empty() || indx >= this->_postProcesses.size()) throw std::runtime_error(fmt::format("[RawrBox-PostProcess] Failed to remove {}!", indx));
		this->_postProcesses.erase(this->_postProcesses.begin() + indx);
	}

	rawrbox::PostProcessBase& PostProcessManager::get(size_t indx) const {
		if (indx >= this->_postProcesses.size()) throw std::runtime_error(fmt::format("[RawrBox-PostProcess] Failed to get {}!", indx));
		return *this->_postProcesses[indx];
	}

	size_t PostProcessManager::count() {
		return this->_postProcesses.size();
	}
	// ----

	void PostProcessManager::upload() {
		this->_rt = std::make_unique<rawrbox::TextureRender>(rawrbox::render::RENDERER->getSize(), false);
		this->_rt->upload(Diligent::TEX_FORMAT_RGBA8_UNORM_SRGB);

		for (const auto& _postProcess : this->_postProcesses) {
			_postProcess->upload();
		}
	}

	void PostProcessManager::render(Diligent::ITextureView* renderTexture) {
		if (this->_postProcesses.empty()) {
			rawrbox::RenderUtils::renderQUAD(renderTexture);
			return;
		}

		for (size_t i = 0; i < this->_postProcesses.size(); i++) {
			auto handle = i == 0 ? renderTexture : this->_rt->getHandle();

			this->_rt->startRecord(false);
			this->_postProcesses[i]->applyEffect(handle);
			this->_rt->stopRecord();
		}

		rawrbox::RenderUtils::renderQUAD(this->_rt->getHandle());
	}

} // namespace rawrbox
