
#include <rawrbox/render/postprocess/manager.hpp>
#include <rawrbox/render/static.hpp>
#include <rawrbox/render/utils/render.hpp>

#include <bx/bx.h>
#include <fmt/format.h>

#include <stdexcept>

// NOLINTBEGIN(*)
static const bgfx::EmbeddedShader final_postprocess_shaders[] = {
    BGFX_EMBEDDED_SHADER(vs_post_base),
    BGFX_EMBEDDED_SHADER(fs_post_final),
    BGFX_EMBEDDED_SHADER_END()};
// NOLINTEND(*)

#define BGFX_STATE_DEFAULT_POST (0 | BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_CULL_CW)

namespace rawrbox {
	PostProcessManager::PostProcessManager(const rawrbox::Vector2i& windowSize) : _windowSize(windowSize) {}
	PostProcessManager::~PostProcessManager() {
		RAWRBOX_DESTROY(this->_vbh);
		RAWRBOX_DESTROY(this->_ibh);
		RAWRBOX_DESTROY(this->_texColor);
		RAWRBOX_DESTROY(this->_program);

		for (auto sample : this->_samples)
			RAWRBOX_DESTROY(sample);

		this->_samples.clear();
		this->_render.reset();

		this->_postProcesses.clear();
		this->_vertices.clear();
		this->_indices.clear();
	}

	void PostProcessManager::pushVertice(const rawrbox::Vector2f& pos, const rawrbox::Vector2f& uv) {
		this->_vertices.emplace_back(
		    // pos
		    rawrbox::Vector3f((pos.x / this->_windowSize.x * 2 - 1),
			(pos.y / this->_windowSize.y * 2 - 1) * -1,
			0.0F),

		    // uv
		    rawrbox::Vector2f(uv.x,
			uv.y));
	}

	void PostProcessManager::pushIndices(uint16_t a, uint16_t b, uint16_t c) {
		auto pos = static_cast<uint16_t>(this->_vertices.size());

		this->_indices.push_back(pos - a);
		this->_indices.push_back(pos - b);
		this->_indices.push_back(pos - c);
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
		this->_samples.clear();
		for (size_t i = 0; i < this->_postProcesses.size(); i++) {
			bgfx::ViewId id = rawrbox::POST_PROCESSING_ID + static_cast<bgfx::ViewId>(i);
			this->_samples.push_back(bgfx::createFrameBuffer(this->_windowSize.x, this->_windowSize.y, bgfx::TextureFormat::RGBA8, BGFX_TEXTURE_RT));

			bgfx::touch(id);
			bgfx::setViewRect(id, 0, 0, bgfx::BackbufferRatio::Equal);
			bgfx::setViewClear(id, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH | BGFX_CLEAR_STENCIL, 0x00000000, 1.0F, 0);
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

		// Generate vertices -----
		auto screeSize = this->_windowSize.cast<float>();
		this->pushVertice({0, 0}, {0, 0});
		this->pushVertice({0, screeSize.y}, {0, 1});
		this->pushVertice({screeSize.x, 0}, {1, 0});
		this->pushVertice({screeSize.x, screeSize.y}, {1, 1});

		this->pushIndices(4, 3, 2);
		this->pushIndices(3, 1, 2);

		// Generate buffers ---
		this->_vbh = bgfx::createVertexBuffer(bgfx::makeRef(this->_vertices.data(), static_cast<uint32_t>(this->_vertices.size()) * rawrbox::PosUVVertexData::vLayout().getStride()), rawrbox::PosUVVertexData::vLayout());
		this->_ibh = bgfx::createIndexBuffer(bgfx::makeRef(this->_indices.data(), static_cast<uint32_t>(this->_indices.size()) * sizeof(uint16_t)));

		// Load Shader --------
		rawrbox::RenderUtils::buildShader(final_postprocess_shaders, this->_program);
		// ------------------

		this->_texColor = bgfx::createUniform("s_texColor", bgfx::UniformType::Sampler);
	}

	void PostProcessManager::begin() {
		if (this->_recording) throw std::runtime_error("[RawrBox-PostProcess] Already drawing, call 'end()' first");
		if (this->_render == nullptr) throw std::runtime_error("[RawrBox-PostProcess] Render texture is not set");

		this->_recording = true;
		this->_render->startRecord();
		bgfx::setViewTransform(rawrbox::CURRENT_VIEW_ID, rawrbox::MAIN_CAMERA->getViewMtx().data(), rawrbox::MAIN_CAMERA->getProjMtx().data());
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

			bgfx::touch(rawrbox::CURRENT_VIEW_ID);
			bgfx::setViewTransform(rawrbox::CURRENT_VIEW_ID, nullptr, nullptr);
			bgfx::setTexture(0, this->_texColor, pass == 0 ? this->_render->getHandle() : bgfx::getTexture(this->_samples[pass - 1]));
			bgfx::setVertexBuffer(0, this->_vbh);
			bgfx::setIndexBuffer(this->_ibh);

			bgfx::setState(BGFX_STATE_DEFAULT_POST);
			this->_postProcesses[pass]->applyEffect();
		}

		bgfx::discard(BGFX_DISCARD_ALL);

		// Draw final texture
		rawrbox::CURRENT_VIEW_ID = prevID;
		bgfx::touch(rawrbox::CURRENT_VIEW_ID);
		bgfx::setViewTransform(rawrbox::CURRENT_VIEW_ID, nullptr, nullptr);
		bgfx::setTexture(0, this->_texColor, bgfx::getTexture(this->_samples.back()));
		bgfx::setVertexBuffer(0, this->_vbh);
		bgfx::setIndexBuffer(this->_ibh);
		bgfx::setState(BGFX_STATE_DEFAULT_POST);
		bgfx::submit(rawrbox::CURRENT_VIEW_ID, this->_program);
		bgfx::discard();
	}

} // namespace rawrbox
