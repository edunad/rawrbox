
#include <rawrbox/render/post_process/manager.hpp>
#include <rawrbox/render/static.hpp>
#include <rawrbox/render/utils/render.hpp>

#include <fmt/format.h>

#include <stdexcept>

namespace rawrbox {
	PostProcessManager::PostProcessManager(const rawrbox::Vector2i& windowSize) : _windowSize(windowSize) {}
	PostProcessManager::~PostProcessManager() {
		/*for (auto sample : this->_samples)
			RAWRBOX_DESTROY(sample);

		this->_samples.clear();*/
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
		for (auto& effect : this->_postProcesses) {
			effect->upload();
		}
	}

	void PostProcessManager::render(Diligent::ITextureView* renderTexture) {
		if (this->_postProcesses.empty()) return;

		for (size_t pass = 0; pass < this->_postProcesses.size(); pass++) {

			// rawrbox::CURRENT_VIEW_ID = rawrbox::POST_PROCESSING_ID + static_cast<bgfx::ViewId>(pass);
			// rawrbox::RenderUtils::drawQUAD(pass == 0 ? renderTexture : bgfx::getTexture(this->_samples[pass - 1]), this->_windowSize, false);

			this->_postProcesses[pass]->applyEffect();
		}

		/*if (this->_samples.empty()) return;

		bgfx::ViewId prevID = rawrbox::CURRENT_VIEW_ID;
		for (size_t pass = 0; pass < this->_postProcesses.size(); pass++) {
			rawrbox::CURRENT_VIEW_ID = rawrbox::POST_PROCESSING_ID + static_cast<bgfx::ViewId>(pass);
			rawrbox::RenderUtils::drawQUAD(pass == 0 ? renderTexture : bgfx::getTexture(this->_samples[pass - 1]), this->_windowSize, false);
			this->_postProcesses[pass]->applyEffect();
		}

		// Draw final texture
		rawrbox::CURRENT_VIEW_ID = prevID;
		bgfx::setViewTransform(rawrbox::CURRENT_VIEW_ID, nullptr, nullptr);
		rawrbox::RenderUtils::drawQUAD(bgfx::getTexture(this->_samples.back()), this->_windowSize);*/
	}

} // namespace rawrbox
