
#include <rawrbox/render/stencil.h>
#include <bgfx/embedded_shader.h>

// Compiled shaders
#include <generated/shaders/render/all.h>

#define BGFX_STATE_DEFAULT_2D (0 \
		| BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA) \
		| BGFX_STATE_WRITE_RGB \
		| BGFX_STATE_WRITE_A \
		| BGFX_STATE_CULL_CW \
		)

static const bgfx::EmbeddedShader shaders[] =
{
	BGFX_EMBEDDED_SHADER(vs_stencil),
	BGFX_EMBEDDED_SHADER(fs_stencil),
	BGFX_EMBEDDED_SHADER_END()
};

namespace rawrBox {
	Stencil::Stencil(bgfx::ViewId id, const rawrBox::Vector2& size) {
		this->_windowSize = size;
		this->_viewId = id;

		// Shader layout
		this->_vLayout.begin()
			.add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
			.add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
			.add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true)
		.end();
	}

	Stencil::~Stencil() {
		// CLEANUP BUFFERS & PROGRAMS
		bgfx::destroy(this->_2dprogram);
		bgfx::destroy(this->_textureHandle);
		bgfx::destroy(this->_texColor);

		this->_pixelTexture = nullptr;
	}

	void Stencil::initialize() {
		bgfx::RendererType::Enum type = bgfx::getRendererType();
		bgfx::ShaderHandle vsh = bgfx::createEmbeddedShader(shaders, type, "vs_stencil");
		bgfx::ShaderHandle fsh = bgfx::createEmbeddedShader(shaders, type, "fs_stencil");

		this->_2dprogram = bgfx::createProgram(vsh, fsh, true);
		if(this->_2dprogram.idx == 0) throw std::runtime_error("[RawrBOX-Stencil] Failed to initialize shader program");

		this->_pixelTexture = std::make_shared<rawrBox::TextureFlat>(rawrBox::Vector2i(1, 1), Colors::White);
		this->_pixelTexture->upload();

		this->_texColor = bgfx::createUniform("s_texColor", bgfx::UniformType::Sampler);
		this->_renderTexture = std::make_shared<rawrBox::TextureRender>(this->_viewId, this->_windowSize);
	}

	void Stencil::resize(const rawrBox::Vector2i& size) {
		this->_windowSize = size;
	}

	// ------RENDERING
	void Stencil::pushVertice(const rawrBox::Vector2& pos, const rawrBox::Vector2& uv, const rawrBox::Color& col) {
		this->_vertices.emplace_back(
			// pos
			(pos.x / _windowSize.x * 2 - 1),
			(pos.y / _windowSize.y * 2 - 1) * -1,
			0.0f,

			// uv
			uv.x,
			uv.y,

			// color
			rawrBox::Color::pack(col)
		);
	}

	void Stencil::pushIndices(uint16_t a, uint16_t b, uint16_t c) {
		this->_indices.push_back(a);
		this->_indices.push_back(b);
		this->_indices.push_back(c);
	}
	// --------------------

	// ------UTILS
	void Stencil::drawTriangle(const rawrBox::Vector2& a, const rawrBox::Vector2& aUV, const rawrBox::Color& colA, const rawrBox::Vector2& b, const rawrBox::Vector2& bUV, const rawrBox::Color& colB, const rawrBox::Vector2& c, const rawrBox::Vector2& cUV, const rawrBox::Color& colC) {
		if (colA.isTransparent() && colB.isTransparent() && colC.isTransparent()) return;

		// Setup --------
		this->setTexture(this->_pixelTexture->getHandle());
		this->setShaderProgram(this->_2dprogram);
		// ----

		this->pushVertice(a + this->_offset, aUV, colA);
		this->pushVertice(b + this->_offset, bUV, colB);
		this->pushVertice(c + this->_offset, cUV, colC);
		this->pushIndices(3, 2, 1);
	}

	void Stencil::drawBox(const rawrBox::Vector2& pos, const rawrBox::Vector2& size, rawrBox::Color col) {
		this->drawTexture(pos, size, this->_pixelTexture, col);
	}

	void Stencil::drawTexture(const rawrBox::Vector2& pos, const rawrBox::Vector2& size, std::shared_ptr<rawrBox::TextureBase> tex, rawrBox::Color col, rawrBox::Vector2 uvStart, rawrBox::Vector2 uvEnd, float rotation, rawrBox::Vector2 origin) {
		// TextureImage setup -----
		if(tex == nullptr) throw std::runtime_error("[RawrBOX-Stencil] Invalid texture, cannot draw");

		// TextureImage setup -----
		this->setTexture(tex->getHandle());
		this->setShaderProgram(this->_2dprogram);
		// ------

		this->pushVertice(pos + rawrBox::Vector2(-size.x, -size.y), uvStart, col);
		this->pushVertice(pos + rawrBox::Vector2(-size.x,  size.y), {uvStart.x, uvEnd.y}, col);
		this->pushVertice(pos + rawrBox::Vector2(size.x, -size.y), {uvEnd.x, uvStart.y}, col);
		this->pushVertice(pos + rawrBox::Vector2(size.x,  size.y), uvEnd, col);

		this->pushIndices(0, 1, 2);
		this->pushIndices(1, 3, 2);
	}
	// --------------------

	// ------RENDERING
	void Stencil::setTexture(const bgfx::TextureHandle& tex) {
		if (this->_textureHandle.idx != tex.idx) this->internalDraw(this->_renderTexture->id());
		this->_textureHandle = tex;
	}

	void Stencil::setShaderProgram(const bgfx::ProgramHandle& handle) {
		if (this->_stencilProgram.idx != handle.idx) this->internalDraw(this->_renderTexture->id());
		this->_stencilProgram = handle;
	}

	void Stencil::internalDraw(bgfx::ViewId id) {
		if (this->_vertices.empty() || this->_indices.empty()) return;
		bgfx::setTexture(0, this->_texColor, this->_textureHandle);

		// Setup buffer (Transient are destroyed every frame, made for things that change a lot) ----
		/*
		It's special buffer type for index/vertex buffers that are changing every frame. It's there as convenience as fire&forget, so you don't have to manually manage buffers that are changing all the time. Also, as we know what's life-time of these buffers, it allows some internal optimizations.

		Transient - changes every frame
		Dynamic - changes sometimes (if you don't pass data on creation, buffer/texture is dynamic)
		Static - never changes (if you pass data to any creation function, it's assumed that buffer/texture is immutable)
		*/
		bgfx::TransientVertexBuffer vbh;
		bgfx::TransientIndexBuffer ibh;

		bgfx::allocTransientVertexBuffer(&vbh, static_cast<uint32_t>(this->_vertices.size()), this->_vLayout);
		bx::memCopy(vbh.data, this->_vertices.data(), this->_vertices.size() * this->_vLayout.m_stride);

		bgfx::allocTransientIndexBuffer(&ibh, static_cast<uint32_t>(this->_indices.size()));
		bx::memCopy(ibh.data, this->_indices.data(),  this->_indices.size() * sizeof(uint16_t));

		bgfx::setVertexBuffer(0, &vbh, 0, static_cast<uint32_t>(this->_vertices.size()));
		bgfx::setIndexBuffer(&ibh, 0, static_cast<uint32_t>(this->_indices.size()));
		// ----------------------

		bgfx::setState(BGFX_STATE_DEFAULT_2D, 0);
		bgfx::submit(id, this->_stencilProgram);

		this->_vertices.clear();
		this->_indices.clear();
	}

	void Stencil::drawRecording() {
		if(this->_renderTexture == nullptr || !bgfx::isValid(this->_renderTexture->getHandle())) return;

		// TextureImage setup -----
		this->_textureHandle = _renderTexture->getHandle();
		this->_stencilProgram = this->_2dprogram;
		// -----

		rawrBox::Vector2 pos = _windowSize / 2;
		rawrBox::Vector2 size = _windowSize / 2;

		this->pushVertice(pos + rawrBox::Vector2(-size.x, -size.y), 0, rawrBox::Colors::White);
		this->pushVertice(pos + rawrBox::Vector2(-size.x,  size.y), {0, 1}, rawrBox::Colors::White);
		this->pushVertice(pos + rawrBox::Vector2(size.x, -size.y), {1, 0}, rawrBox::Colors::White);
		this->pushVertice(pos + rawrBox::Vector2(size.x,  size.y), 1, rawrBox::Colors::White);

		this->pushIndices(0, 1, 2);
		this->pushIndices(1, 3, 2);

		this->internalDraw(this->_viewId); // Draw on main window
	}

	void Stencil::begin() {
		if(this->_recording) throw std::runtime_error("[RawrBOX-Stencil] Already drawing");

		this->_renderTexture->startRecord();
		this->_recording = true;
	}

	void Stencil::end() {
		if(!this->_recording) throw std::runtime_error("[RawrBOX-Stencil] Not drawing");
		this->internalDraw(this->_renderTexture->id()); // Draw remaining primitives

		this->_renderTexture->stopRecord();
		this->_recording = false;

		// Draw render texture with clipping --
		this->drawRecording();
	}
	// --------------------

	// ------ LOCATION
	void Stencil::pushOffset(const rawrBox::Vector2& offset) {
		this->_offsets.push_back(_offset);
		this->_offset += _offset;
	}

	void Stencil::popOffset() {
		this->_offset -= this->_offsets.back();
		this->_offsets.pop_back();
	}

	void Stencil::pushLocalOffset() {
		this->_oldOffset = this->_offset;
		this->_offset = {};
	}

	void Stencil::popLocalOffset() {
		this->_offset = this->_oldOffset;
	}
	// --------------------

	// ------ CLIPPING
	void Stencil::pushClipping(const rawrBox::AABB& rect) {
		rawrBox::AABB newrect = {rect.pos.x + this->_offset.x, rect.pos.y + this->_offset.y, rect.size.x, rect.size.y};
		if(!this->_clips.empty()) newrect = newrect.clamp(this->_clips.back());

		this->_clips.push_back(newrect);
	}

	void Stencil::popClipping() {
		if (this->_clips.empty()) return;
		this->_clips.pop_back();
	}
	// --------------------
}
