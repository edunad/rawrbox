
#include <rawrbox/render/postprocess/manager.hpp>
#include <rawrbox/render/static.hpp>
#include <rawrbox/render/utils/render.hpp>

#include <bx/bx.h>
#include <fmt/format.h>

#include <stdexcept>

// NOLINTBEGIN(*)
static const bgfx::EmbeddedShader quad_shaders[] = {
    BGFX_EMBEDDED_SHADER(vs_quadtex),
    BGFX_EMBEDDED_SHADER(fs_quadtex),
    BGFX_EMBEDDED_SHADER_END()};
// NOLINTEND(*)

namespace rawrbox {
	PostProcessManager::PostProcessManager(const rawrbox::Vector2i& windowSize) : _windowSize(windowSize) {}
	PostProcessManager::~PostProcessManager() {
		RAWRBOX_DESTROY(this->_texColor);
		RAWRBOX_DESTROY(this->_program);

		for (auto sample : this->_samples)
			RAWRBOX_DESTROY(sample);

		this->_samples.clear();
		this->_render.reset();

		this->_postProcesses.clear();
	}

	// Post utils ----
	void PostProcessManager::add(std::unique_ptr<rawrbox::PostProcessBase> post) {
		this->_postProcesses.push_back(std::move(post));
		this->buildPRViews();
	}

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
		this->_samples.clear();
		for (size_t i = 0; i < this->_postProcesses.size(); i++) {
			bgfx::ViewId id = rawrbox::POST_PROCESSING_ID + static_cast<bgfx::ViewId>(i);
			this->_samples.push_back(bgfx::createFrameBuffer(this->_windowSize.x, this->_windowSize.y, bgfx::TextureFormat::RGBA8, BGFX_TEXTURE_RT));

			bgfx::touch(id);
			bgfx::setViewRect(id, 0, 0, bgfx::BackbufferRatio::Equal);
			bgfx::setViewClear(id, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH | BGFX_CLEAR_STENCIL, 0x00000000, 1.0F);
			bgfx::setViewName(id, fmt::format("POST-PROCESSING-SAMPLE-{}", i).c_str());
			bgfx::setViewFrameBuffer(id, this->_samples[i]);
		}
		// ----
	}

	void PostProcessManager::upload() {
		if (this->_render != nullptr) throw std::runtime_error("[RawrBox-PostProcess] Already uploaded");

		this->_render = std::make_unique<rawrbox::TextureRender>(this->_windowSize);
		this->_render->upload();

		for (auto& effect : this->_postProcesses) {
			effect->upload();
		}

		// Load Shader --------
		rawrbox::RenderUtils::buildShader(quad_shaders, this->_program);
		// ------------------

		this->_texColor = bgfx::createUniform("s_texColor", bgfx::UniformType::Sampler);
	}

	void PostProcessManager::begin() {
		if (this->_recording) throw std::runtime_error("[RawrBox-PostProcess] Already drawing, call 'end()' first");
		if (this->_render == nullptr) throw std::runtime_error("[RawrBox-PostProcess] Render texture is not set");

		this->_recording = true;
		this->_render->startRecord();
	}

	void PostProcessManager::end() {
		if (!this->_recording) throw std::runtime_error("[RawrBox-PostProcess] Not drawing, call 'begin()' first");
		if (this->_render == nullptr) throw std::runtime_error("[RawrBox-PostProcess] Render texture is not set");

		this->_recording = false;
		this->_render->stopRecord();

		bgfx::ViewId prevID = rawrbox::CURRENT_VIEW_ID;
		for (size_t pass = 0; pass < this->_postProcesses.size(); pass++) {
			bgfx::ViewId id = rawrbox::POST_PROCESSING_ID + static_cast<bgfx::ViewId>(pass);
			rawrbox::CURRENT_VIEW_ID = id;

			bgfx::touch(id);
			bgfx::setTexture(0, this->_texColor, pass == 0 ? this->_render->getHandle() : bgfx::getTexture(this->_samples[pass - 1]));
			rawrbox::RenderUtils::renderScreenQuad(this->_windowSize);

			bgfx::setState(0 | BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_CULL_CW);
			this->_postProcesses[pass]->applyEffect();
			bgfx::discard();
		}

		rawrbox::CURRENT_VIEW_ID = prevID;

		// Draw final texture
		bgfx::touch(rawrbox::CURRENT_VIEW_ID);
		bgfx::setTexture(0, this->_texColor, bgfx::getTexture(this->_samples.back()));
		rawrbox::RenderUtils::renderScreenQuad(this->_windowSize);
		bgfx::setState(0 | BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_CULL_CW);
		bgfx::submit(0, this->_program);
		bgfx::discard();
	}

} // namespace rawrbox
