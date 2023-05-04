
#include <rawrbox/render/postprocess/manager.hpp>
#include <rawrbox/render/shader_defines.hpp>
#include <rawrbox/render/static.hpp>

#include <generated/shaders/render/all.hpp>

#include <bx/bx.h>
#include <fmt/format.h>

#include <stdexcept>

// NOLINTBEGIN(*)
static const bgfx::EmbeddedShader model_shaders[] = {
    BGFX_EMBEDDED_SHADER(vs_stencil_flat),
    BGFX_EMBEDDED_SHADER(fs_stencil_flat),
    BGFX_EMBEDDED_SHADER_END()};
// NOLINTEND(*)

constexpr int RENDER_PASS_DOWNSAMPLE_ID = 30;

namespace rawrBox {
	PostProcessManager::PostProcessManager(bgfx::ViewId view, const rawrBox::Vector2i& windowSize) : _view(view), _windowSize(windowSize) {
		// Shader layout
		this->_vLayout.begin()
		    .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
		    .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
		    .end();
	}

	PostProcessManager::~PostProcessManager() {
		RAWRBOX_DESTROY(this->_vbh);
		RAWRBOX_DESTROY(this->_ibh);
		RAWRBOX_DESTROY(this->_texColor);
		RAWRBOX_DESTROY(this->_program);

		for (auto sample : this->_samples)
			RAWRBOX_DESTROY(sample);

		this->_samples.clear();
		this->_render = nullptr;

		this->_postProcesses.clear();
		this->_vertices.clear();
		this->_indices.clear();
	}

	// Post utils ----
	void PostProcessManager::add(std::shared_ptr<rawrBox::PostProcessBase> post) {
		this->_postProcesses.push_back(std::move(post));
		this->buildPRViews();
	}

	void PostProcessManager::remove(size_t indx) {
		if (this->_postProcesses.empty() || indx >= this->_postProcesses.size()) throw std::runtime_error(fmt::format("[RawrBox-PostProcess] Failed to remove {}!", indx));
		this->_postProcesses.erase(this->_postProcesses.begin() + indx);
		this->buildPRViews();
	}

	std::shared_ptr<rawrBox::PostProcessBase> PostProcessManager::get(size_t indx) {
		if (indx >= this->_postProcesses.size()) throw std::runtime_error(fmt::format("[RawrBox-PostProcess] Failed to get {}!", indx));
		return this->_postProcesses[indx];
	}

	size_t PostProcessManager::count() {
		return this->_postProcesses.size();
	}
	// ----
	void PostProcessManager::buildPRViews() {
		if (!rawrBox::BGFX_INITIALIZED) return;

		// Delete old samples
		for (auto sample : this->_samples)
			RAWRBOX_DESTROY(sample);

		// Prepare new samples
		this->_samples.clear();
		for (size_t i = 0; i < this->_postProcesses.size(); i++) {
			bgfx::ViewId id = RENDER_PASS_DOWNSAMPLE_ID + static_cast<bgfx::ViewId>(i);
			this->_samples.push_back(bgfx::createFrameBuffer(this->_windowSize.x, this->_windowSize.y, bgfx::TextureFormat::RGBA8, BGFX_TEXTURE_RT));

			bgfx::touch(id);
			bgfx::setViewRect(id, 0, 0, bgfx::BackbufferRatio::Equal);
			bgfx::setViewClear(id, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 1.0F, 0, 0);
			bgfx::setViewName(id, fmt::format("POST-PROCESSING-SAMPLE-{}", i).c_str());
			bgfx::setViewFrameBuffer(id, this->_samples[i]);
		}
		// ----
	}

	void PostProcessManager::pushVertice(rawrBox::Vector2f pos, const rawrBox::Vector2f& uv) {
		this->_vertices.emplace_back(
		    // pos
		    (pos.x / this->_windowSize.x * 2 - 1),
		    (pos.y / this->_windowSize.y * 2 - 1) * -1,
		    0.0F,

		    // uv
		    uv.x,
		    uv.y);
	}

	void PostProcessManager::pushIndices(uint16_t a, uint16_t b, uint16_t c) {
		auto pos = static_cast<uint16_t>(this->_vertices.size());

		this->_indices.push_back(pos - a);
		this->_indices.push_back(pos - b);
		this->_indices.push_back(pos - c);
	}

	void PostProcessManager::upload() {
		if (this->_render != nullptr) throw std::runtime_error("[RawrBox-PostProcess] Already uploaded");

		this->_render = std::make_shared<rawrBox::TextureRender>(this->_view, this->_windowSize);
		this->_render->upload();

		for (auto effect : this->_postProcesses) {
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
		this->_vbh = bgfx::createVertexBuffer(bgfx::makeRef(this->_vertices.data(), static_cast<uint32_t>(this->_vertices.size()) * this->_vLayout.m_stride), this->_vLayout);
		this->_ibh = bgfx::createIndexBuffer(bgfx::makeRef(this->_indices.data(), static_cast<uint32_t>(this->_indices.size()) * sizeof(uint16_t)));

		// Load Shader --------
		bgfx::RendererType::Enum type = bgfx::getRendererType();
		bgfx::ShaderHandle vsh = bgfx::createEmbeddedShader(model_shaders, type, "vs_stencil_flat");
		bgfx::ShaderHandle fsh = bgfx::createEmbeddedShader(model_shaders, type, "fs_stencil_flat");

		this->_program = bgfx::createProgram(vsh, fsh, true);
		if ((!bgfx::isValid(this->_program))) throw std::runtime_error("[RawrBox-Dither] Failed to initialize shader program");
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

		for (size_t pass = 0; pass < this->_postProcesses.size(); pass++) {
			bgfx::ViewId id = RENDER_PASS_DOWNSAMPLE_ID + static_cast<bgfx::ViewId>(pass);
			rawrBox::CURRENT_VIEW_ID = id;

			bgfx::touch(id);
			bgfx::setTexture(0, this->_texColor, pass == 0 ? this->_render->getHandle() : bgfx::getTexture(this->_samples[pass - 1]));
			bgfx::setVertexBuffer(0, this->_vbh);
			bgfx::setIndexBuffer(this->_ibh);

			bgfx::setState(0 | BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_CULL_CW);
			this->_postProcesses[pass]->applyEffect();
		}

		rawrBox::CURRENT_VIEW_ID = this->_view;

		// Draw final texture
		bgfx::touch(this->_view);
		bgfx::setTexture(0, this->_texColor, bgfx::getTexture(this->_samples.back()));
		bgfx::setVertexBuffer(0, this->_vbh);
		bgfx::setIndexBuffer(this->_ibh);
		bgfx::setState(0 | BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_CULL_CW);
		bgfx::submit(0, this->_program);
	}

} // namespace rawrBox
