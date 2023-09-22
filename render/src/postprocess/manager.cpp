
#include <rawrbox/render/postprocess/manager.hpp>
#include <rawrbox/render/static.hpp>
#include <rawrbox/render/utils/render.hpp>

#include <bx/bx.h>
#include <fmt/format.h>

#include <stdexcept>

namespace rawrbox {
	PostProcessManager::PostProcessManager(const rawrbox::Vector2i& windowSize) : _windowSize(windowSize) {}
	PostProcessManager::~PostProcessManager() {
		for (auto sample : this->_samples)
			RAWRBOX_DESTROY(sample);

		this->_samples.clear();
		this->_postProcesses.clear();
	}

	// Post utils ----
	void PostProcessManager::remove(size_t indx) {
		if (this->_postProcesses.empty() || indx >= this->_postProcesses.size()) throw std::runtime_error(fmt::format("[RawrBox-PostProcess] Failed to remove {}!", indx));
		this->_postProcesses.erase(this->_postProcesses.begin() + indx);
		this->buildPRViews();
	}

	rawrbox::PostProcessBase& PostProcessManager::get(size_t indx) const {
		if (indx >= this->_postProcesses.size()) throw std::runtime_error(fmt::format("[RawrBox-PostProcess] Failed to get {}!", indx));
		return *this->_postProcesses[indx];
	}

	size_t PostProcessManager::count() {
		return this->_postProcesses.size();
	}
	// ----
	void PostProcessManager::buildPRViews() {
		if (!rawrbox::BGFX_INITIALIZED) return;

		// Delete old samples
		for (auto sample : this->_samples)
			RAWRBOX_DESTROY(sample);

		// Prepare new samples
		auto w = static_cast<uint16_t>(this->_windowSize.x);
		auto h = static_cast<uint16_t>(this->_windowSize.y);

		this->_samples.clear();
		for (size_t i = 0; i < this->_postProcesses.size(); i++) {
			bgfx::ViewId id = rawrbox::POST_PROCESSING_ID + static_cast<bgfx::ViewId>(i);
			this->_samples.push_back(bgfx::createFrameBuffer(w, h, bgfx::TextureFormat::RGBA8, BGFX_TEXTURE_RT));

			bgfx::touch(id);
			bgfx::setViewRect(id, 0, 0, bgfx::BackbufferRatio::Equal);
			bgfx::setViewClear(id, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH | BGFX_CLEAR_STENCIL, 0x00000000, 1.0F, 0);
			bgfx::setViewName(id, fmt::format("POST-PROCESSING-SAMPLE-{}", i).c_str());
			bgfx::setViewFrameBuffer(id, this->_samples[i]);
		}
		// ----
	}

	void PostProcessManager::upload() {
		for (auto& effect : this->_postProcesses) {
			effect->upload();
		}
	}

	void PostProcessManager::render(const bgfx::TextureHandle& renderTexture) {
		if (this->_samples.empty()) return;

		bgfx::ViewId prevID = rawrbox::CURRENT_VIEW_ID;
		for (size_t pass = 0; pass < this->_postProcesses.size(); pass++) {
			rawrbox::CURRENT_VIEW_ID = rawrbox::POST_PROCESSING_ID + static_cast<bgfx::ViewId>(pass);
			rawrbox::RenderUtils::drawQUAD(pass == 0 ? renderTexture : bgfx::getTexture(this->_samples[pass - 1]), this->_windowSize, false);
			this->_postProcesses[pass]->applyEffect();
		}

		// Draw final texture
		rawrbox::CURRENT_VIEW_ID = prevID;
		bgfx::setViewTransform(rawrbox::CURRENT_VIEW_ID, nullptr, nullptr);
		rawrbox::RenderUtils::drawQUAD(bgfx::getTexture(this->_samples.back()), this->_windowSize);
	}

} // namespace rawrbox
