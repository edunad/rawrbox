
#include <rawrbox/render/postprocess/manager.h>
#include <rawrbox/render/shader_defines.h>
#include <rawrbox/render/static.h>

#include <bx/bx.h>
#include <fmt/format.h>
#include <generated/shaders/render/all.h>
// NOLINTBEGIN(*)
static const bgfx::EmbeddedShader model_shaders[] = {
    BGFX_EMBEDDED_SHADER(vs_stencil_flat),
    BGFX_EMBEDDED_SHADER(fs_stencil_flat),
    BGFX_EMBEDDED_SHADER_END()};
// NOLINTEND(*)
namespace rawrBox {
	PostProcessManager::PostProcessManager(bgfx::ViewId view, const rawrBox::Vector2i& windowSize) : _view(view), _windowSize(windowSize) {

		this->_pixels.resize(this->_windowSize.x * this->_windowSize.y * 4); // * Channels

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
		RAWRBOX_DESTROY(this->_finalHandle);
		RAWRBOX_DESTROY(this->_copyHandle);

		this->_render = nullptr;

		this->_postProcesses.clear();
		this->_vertices.clear();
		this->_indices.clear();
	}

	// Post utils ----
	void PostProcessManager::registerPostProcess(std::shared_ptr<rawrBox::PostProcessBase> post) {
		this->_postProcesses.push_back(std::move(post));
	}

	void PostProcessManager::removePostProcess(size_t indx) {
		if (this->_postProcesses.empty() || indx >= this->_postProcesses.size()) return;
		this->_postProcesses.erase(this->_postProcesses.begin() + indx);
	}
	// ----

	void PostProcessManager::pushVertice(rawrBox::Vector2f pos, const rawrBox::Vector2f& uv) {
		this->_vertices.emplace_back(
		    // pos
		    (pos.x / this->_windowSize.x * 2 - 1),
		    (pos.y / this->_windowSize.y * 2 - 1) * -1,
		    0.0f,

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
		this->pushVertice(0, 0);
		this->pushVertice({0, this->_windowSize.y}, {0, 1});
		this->pushVertice({this->_windowSize.x, 0}, {1, 0});
		this->pushVertice({this->_windowSize.x, this->_windowSize.y}, 1);

		this->pushIndices(4, 3, 2);
		this->pushIndices(3, 1, 2);

		// Generate buffers ---
		this->_vbh = bgfx::createVertexBuffer(bgfx::makeRef(this->_vertices.data(), static_cast<uint32_t>(this->_vertices.size()) * this->_vLayout.m_stride), this->_vLayout);
		this->_ibh = bgfx::createIndexBuffer(bgfx::makeRef(this->_indices.data(), static_cast<uint32_t>(this->_indices.size()) * sizeof(uint16_t)));

		// Final texture
		this->_copyHandle = bgfx::createTexture2D(static_cast<uint32_t>(this->_windowSize.x), static_cast<uint32_t>(this->_windowSize.y), false, 1, bgfx::TextureFormat::RGBA8, 0 | BGFX_TEXTURE_BLIT_DST | BGFX_TEXTURE_READ_BACK | BGFX_SAMPLER_MIN_POINT | BGFX_SAMPLER_MAG_POINT | BGFX_SAMPLER_MIP_POINT | BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP);
		bgfx::setName(this->_copyHandle, fmt::format("RAWR-BLIT-{}", this->_copyHandle.idx).c_str());

		this->_finalHandle = bgfx::createTexture2D(static_cast<uint32_t>(this->_windowSize.x), static_cast<uint32_t>(this->_windowSize.y), false, 1, bgfx::TextureFormat::RGBA8, 0 | BGFX_SAMPLER_MIN_POINT | BGFX_SAMPLER_MAG_POINT | BGFX_SAMPLER_MIP_POINT | BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP);
		bgfx::setName(this->_finalHandle, fmt::format("RAWR-POSTPROCESS-{}", this->_finalHandle.idx).c_str());

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

	void PostProcessManager::captureData() {
		bgfx::blit(rawrBox::CURRENT_VIEW_ID, this->_copyHandle, 0, 0, this->_render->getHandle()); // Get pixels from RT texture
		bgfx::readTexture(this->_copyHandle, this->_pixels.data());

		bgfx::updateTexture2D(this->_finalHandle, 0, 0, 0, 0, static_cast<uint32_t>(this->_windowSize.x), static_cast<uint32_t>(this->_windowSize.y), bgfx::copy(this->_pixels.data(), static_cast<uint32_t>(this->_pixels.size())));
	}

	void PostProcessManager::end() {
		if (!this->_recording) throw std::runtime_error("[RawrBox-PostProcess] Not drawing, call 'begin()' first");
		if (this->_render == nullptr) throw std::runtime_error("[RawrBox-PostProcess] Render texture is not set");

		this->_recording = false;
		this->_render->stopRecord();
		this->captureData();

		// Apply effects in order
		for (auto effect : this->_postProcesses) {
			// this->_render->startRecord();

			bgfx::setTexture(0, this->_texColor, this->_finalHandle); // Pass our flatten render target
			bgfx::setVertexBuffer(0, this->_vbh);
			bgfx::setIndexBuffer(this->_ibh);
			effect->applyEffect();

			// this->_render->stopRecord();
			// this->captureData();
		}

		// Draw final texture
		bgfx::setTexture(0, this->_texColor, this->_finalHandle); // Pass our flatten render target
		bgfx::setVertexBuffer(0, this->_vbh);
		bgfx::setIndexBuffer(this->_ibh);
		bgfx::submit(0, this->_program);
	}

} // namespace rawrBox
