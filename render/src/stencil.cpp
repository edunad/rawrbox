
#include <rawrbox/render/shader_defines.h>
#include <rawrbox/render/stencil.h>

// Compiled shaders
#include <bx/math.h>
#include <generated/shaders/render/all.h>

#include <utf8.h>

#define BGFX_STATE_DEFAULT_2D (0 | BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA) | BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_CULL_CW)

static const bgfx::EmbeddedShader shaders[] = {
    BGFX_EMBEDDED_SHADER(vs_stencil),
    BGFX_EMBEDDED_SHADER(fs_stencil),
    BGFX_EMBEDDED_SHADER(fs_stencil_line_stipple),
    BGFX_EMBEDDED_SHADER(vs_stencil_line_stipple),
    BGFX_EMBEDDED_SHADER(fs_stencil_text),
    BGFX_EMBEDDED_SHADER(vs_stencil_text),
    BGFX_EMBEDDED_SHADER_END()};

namespace rawrBox {
	Stencil::Stencil(bgfx::ViewId id, const rawrBox::Vector2i& size) {
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
		bgfx::destroy(this->_lineprogram);
		bgfx::destroy(this->_textprogram);

		bgfx::destroy(this->_textureHandle);
		bgfx::destroy(this->_texColor);

		this->_pixelTexture = nullptr;
		this->_renderTexture = nullptr;
	}

	void Stencil::initialize() {
		bgfx::RendererType::Enum type = bgfx::getRendererType();

		// Load 2D --------
		bgfx::ShaderHandle vsh = bgfx::createEmbeddedShader(shaders, type, "vs_stencil");
		bgfx::ShaderHandle fsh = bgfx::createEmbeddedShader(shaders, type, "fs_stencil");

		this->_2dprogram = bgfx::createProgram(vsh, fsh, true);
		if (this->_2dprogram.idx == 0) throw std::runtime_error("[RawrBox-Stencil] Failed to initialize shader program");
		// ------------------

		// Load Line ---------
		vsh = bgfx::createEmbeddedShader(shaders, type, "vs_stencil_line_stipple");
		fsh = bgfx::createEmbeddedShader(shaders, type, "fs_stencil_line_stipple");

		this->_lineprogram = bgfx::createProgram(vsh, fsh, true);
		if (this->_lineprogram.idx == 0) throw std::runtime_error("[RawrBox-Stencil] Failed to initialize shader program");
		// --------------------

		// Load Text ---------
		vsh = bgfx::createEmbeddedShader(shaders, type, "vs_stencil_text");
		fsh = bgfx::createEmbeddedShader(shaders, type, "fs_stencil_text");

		this->_textprogram = bgfx::createProgram(vsh, fsh, true);
		if (this->_textprogram.idx == 0) throw std::runtime_error("[RawrBox-Stencil] Failed to initialize shader program");
		// --------------------

		this->_pixelTexture = std::make_shared<rawrBox::TextureFlat>(rawrBox::Vector2i(1, 1), Colors::White);
		this->_pixelTexture->upload();

		this->_texColor = bgfx::createUniform("s_texColor", bgfx::UniformType::Sampler);
		this->_renderTexture = std::make_shared<rawrBox::TextureRender>(this->_viewId, this->_windowSize);
	}

	void Stencil::resize(const rawrBox::Vector2i& size) {
		this->_windowSize = size;
	}

	void Stencil::pushVertice(rawrBox::Vector2f pos, const rawrBox::Vector2f& uv, const rawrBox::Color& col) {
		this->applyScale(pos);
		this->applyRotation(pos);

		this->_vertices.emplace_back(
		    // pos
		    ((pos.x + this->_offset.x) / _windowSize.x * 2 - 1),
		    ((pos.y + this->_offset.y) / _windowSize.y * 2 - 1) * -1,
		    0.0f,

		    // uv
		    uv.x,
		    uv.y,

		    // color
		    rawrBox::Color::pack(col));
	}

	void Stencil::pushIndices(uint16_t a, uint16_t b, uint16_t c) {
		auto pos = static_cast<uint16_t>(this->_vertices.size());

		this->_indices.push_back(pos - a);
		this->_indices.push_back(pos - b);
		this->_indices.push_back(pos - c);
	}

	void Stencil::applyRotation(rawrBox::Vector2f& vert) {
		if (this->_rotation.rotation == 0) return;

		float translationMatrix[16];
		bx::mtxIdentity(translationMatrix);
		bx::mtxTranslate(translationMatrix, -_rotation.origin.x, -_rotation.origin.y, 0);

		float rotationMatrix[16];
		bx::mtxIdentity(rotationMatrix);
		bx::mtxRotateZ(rotationMatrix, bx::toRad(_rotation.rotation));

		float reverseTranslationMatrix[16];
		bx::mtxIdentity(reverseTranslationMatrix);
		bx::mtxTranslate(reverseTranslationMatrix, _rotation.origin.x, _rotation.origin.y, 0);

		float mul[16];
		bx::mtxMul(mul, reverseTranslationMatrix, rotationMatrix);
		bx::mtxMul(mul, mul, translationMatrix);

		float vv[4] = {vert.x, vert.y, 0, -1.0f};
		float v[4];
		bx::vec4MulMtx(v, vv, mul);

		vert.x = v[0];
		vert.y = v[1];
	}

	void Stencil::applyScale(rawrBox::Vector2f& vert) {
		if (this->_scale == 0) return;

		float translationMatrix[16];
		bx::mtxIdentity(translationMatrix);
		bx::mtxTranslate(translationMatrix, -_rotation.origin.x, -_rotation.origin.y, 0);

		float rotationMatrix[16];
		bx::mtxIdentity(rotationMatrix);
		bx::mtxScale(rotationMatrix, this->_scale.x, this->_scale.y, 1.f);

		float reverseTranslationMatrix[16];
		bx::mtxIdentity(reverseTranslationMatrix);
		bx::mtxTranslate(reverseTranslationMatrix, _rotation.origin.x, _rotation.origin.y, 0);

		float mul[16];
		bx::mtxMul(mul, reverseTranslationMatrix, rotationMatrix);
		bx::mtxMul(mul, mul, translationMatrix);

		float vv[4] = {vert.x, vert.y, 0, -1.0f};
		float v[4];
		bx::vec4MulMtx(v, vv, mul);

		vert.x = v[0];
		vert.y = v[1];
	}
	// --------------------

	// ------UTILS
	void Stencil::drawTriangle(const rawrBox::Vector2f& a, const rawrBox::Vector2f& aUV, const rawrBox::Color& colA, const rawrBox::Vector2f& b, const rawrBox::Vector2f& bUV, const rawrBox::Color& colB, const rawrBox::Vector2f& c, const rawrBox::Vector2f& cUV, const rawrBox::Color& colC) {
		if (colA.isTransparent() && colB.isTransparent() && colC.isTransparent()) return;

		if (this->_outline.isSet()) {
			float thick = this->_outline.thickness;

			this->drawLine({a.x, a.y - thick}, {b.x, b.y + thick / 2.f}, colA);
			this->drawLine(b, c, colB);
			this->drawLine({c.x + thick / 2.f, c.y}, a, colC);
		} else {
			// Setup --------
			this->setTexture(this->_pixelTexture->getHandle());
			this->setShaderProgram(this->_2dprogram);
			this->setDrawMode();
			// ----

			this->pushVertice(a, aUV, colA);
			this->pushVertice(b, bUV, colB);
			this->pushVertice(c, cUV, colC);

			this->pushIndices(3, 2, 1);
		}
	}

	void Stencil::drawBox(const rawrBox::Vector2f& pos, const rawrBox::Vector2f& size, const rawrBox::Color& col) {
		if (this->_outline.isSet()) {
			float thick = this->_outline.thickness;

			this->drawLine({pos.x - thick, pos.y}, {pos.x + size.x, pos.y}, col);
			this->drawLine({pos.x + size.x, pos.y - thick}, {pos.x + size.x, pos.y + size.y}, col);
			this->drawLine({pos.x + size.x + (thick > 1.f ? thick : 0.f), pos.y + size.y}, {pos.x - thick, pos.y + size.y}, col);
			this->drawLine({pos.x, pos.y + size.y}, {pos.x, pos.y}, col);
		} else {
			this->drawTexture(pos, size, this->_pixelTexture, col);
		}
	}

	void Stencil::drawTexture(const rawrBox::Vector2f& pos, const rawrBox::Vector2f& size, std::shared_ptr<rawrBox::TextureBase> tex, const rawrBox::Color& col, const rawrBox::Vector2f& uvStart, const rawrBox::Vector2f& uvEnd) {
		if (col.isTransparent()) return;

		// TextureImage setup -----
		if (tex == nullptr) throw std::runtime_error("[RawrBox-Stencil] Invalid texture, cannot draw");

		// TextureImage setup -----
		this->setTexture(tex->getHandle());
		this->setShaderProgram(this->_2dprogram);
		this->setDrawMode();
		// ------

		this->pushVertice({pos.x, pos.y}, uvStart, col);
		this->pushVertice({pos.x, pos.y + size.y}, {uvStart.x, uvEnd.y}, col);
		this->pushVertice({pos.x + size.x, pos.y}, {uvEnd.x, uvStart.y}, col);
		this->pushVertice({pos.x + size.x, pos.y + size.y}, uvEnd, col);

		this->pushIndices(4, 3, 2);
		this->pushIndices(3, 1, 2);
	}

	void Stencil::drawCircle(const rawrBox::Vector2f& pos, const rawrBox::Vector2f& size, const rawrBox::Color& col, size_t roundness, float angleStart, float angleEnd) {
		if (col.isTransparent()) return;

		// Setup --------
		if (!this->_outline.isSet()) {
			this->setTexture(this->_pixelTexture->getHandle());
			this->setShaderProgram(this->_2dprogram);
			this->setDrawMode();
		}
		// ----

		auto radius = size / 2;
		auto targetPos = pos + radius;

		float angStartRad = bx::toRad(angleStart);
		float angEndRad = bx::toRad(angleEnd);

		float space = rawrBox::pi<float> / roundness * 2;

		for (size_t i = 0; i < roundness;) {
			auto ang = space * i++ + angStartRad;
			if (ang + space > angEndRad) break;

			rawrBox::Vector2 b = targetPos + rawrBox::Vector2::cosSin(ang) * radius;
			rawrBox::Vector2 c = targetPos + rawrBox::Vector2::cosSin(ang + space) * radius;

			if (this->_outline.isSet()) {
				this->drawLine(b, c, col);
			} else {
				this->drawTriangle(
				    b, {}, col,
				    targetPos, {}, col,
				    c, {}, col);
			}
		}
	}

	void Stencil::drawLine(const rawrBox::Vector2& from, const rawrBox::Vector2& to, const rawrBox::Color& col) {
		if (col.isTransparent()) return;

		rawrBox::StencilOutline outline = {1.f, 0.f};
		if (this->_outline.isSet()) outline = this->_outline;
		if (outline.thickness <= 0.f) return;

		bool usePTLines = outline.thickness == 1.f;

		// Setup --------
		this->setShaderProgram((usePTLines || outline.stipple > 0.f) ? this->_lineprogram : this->_2dprogram);
		this->setTexture(this->_pixelTexture->getHandle());
		this->setDrawMode(usePTLines ? BGFX_STATE_PT_LINES : 0); // Reset
		// ----

		if (usePTLines) {
			this->pushVertice(from, 0, col);
			this->pushVertice(to, outline.stipple, col);

			auto pos = static_cast<uint16_t>(this->_vertices.size());
			this->_indices.push_back(pos - 1);
			this->_indices.push_back(pos - 2);
		} else {
			float angle = from.angle(to);
			float uvEnd = outline.stipple <= 0.f ? 1.f : outline.stipple;

			auto vertA = from + rawrBox::Vector2::cosSin(angle) * outline.thickness;
			auto vertB = from + rawrBox::Vector2::cosSin(angle) * -outline.thickness;
			auto vertC = to + rawrBox::Vector2::cosSin(angle) * outline.thickness;
			auto vertD = to + rawrBox::Vector2::cosSin(angle) * -outline.thickness;

			this->pushVertice(vertA, 0, col);
			this->pushVertice(vertC, {uvEnd, 0}, col);
			this->pushVertice(vertB, {0, uvEnd}, col);
			this->pushVertice(vertD, uvEnd, col);

			this->pushIndices(4, 3, 2);
			this->pushIndices(3, 1, 2);
		}
	}

	void Stencil::drawText(rawrBox::Font* font, const std::string& text, const rawrBox::Vector2f& pos, const rawrBox::Color& col, rawrBox::TextAlignment alignX, rawrBox::TextAlignment alignY) {
		if (font == nullptr || col.isTransparent() || text.empty()) return;

		// Setup --------
		this->setShaderProgram(this->_textprogram);
		this->setDrawMode(); // Reset
		// ----

		rawrBox::Vector2f startpos = pos;
		rawrBox::Vector2f tsize = font->getStringSize(text);
		if (alignX != TextAlignment::Left || alignY != TextAlignment::Left) {
			switch (alignX) {
			case TextAlignment::Left:
				break;
			case TextAlignment::Center:
				startpos.x -= tsize.x / 2;
				break;
			case TextAlignment::Right:
				startpos.x -= tsize.x;
				break;
			}

			switch (alignY) {
			case TextAlignment::Left:
				break;
			case TextAlignment::Center:
				startpos.y -= tsize.y / 2;
				break;
			case TextAlignment::Right:
				startpos.y -= tsize.y;
				break;
			}
		}

		startpos.x = std::roundf(startpos.x);
		startpos.y = std::roundf(startpos.y);

		float lineheight = font->getLineHeight();
		startpos.y += lineheight + static_cast<float>(font->face->size->metrics.descender >> 6);

		rawrBox::Vector2 curpos = startpos;
		const rawrBox::Glyph* prevGlyph = nullptr;

		auto beginIter = text.begin();
		auto endIter = utf8::find_invalid(text.begin(), text.end());

		while (beginIter != endIter) {
			uint32_t point = utf8::next(beginIter, endIter);

			if (point == '\n') {
				curpos.y += lineheight;
				curpos.x = startpos.x;

				prevGlyph = nullptr;
				continue;
			}

			auto& glyph = font->getGlyph(point);
			if (prevGlyph != nullptr) {
				curpos.x += font->getKerning(glyph, *prevGlyph);
			}

			rawrBox::Vector2 p = {curpos.x + glyph.bearing.x, curpos.y - glyph.bearing.y};
			rawrBox::Vector2 s = {static_cast<float>(glyph.size.x), static_cast<float>(glyph.size.y)};

			this->setTexture(font->getHandle(glyph));

			this->pushVertice({p.x, p.y}, glyph.textureTopLeft, col);
			this->pushVertice({p.x, p.y + s.y}, {glyph.textureTopLeft.x, glyph.textureBottomRight.y}, col);
			this->pushVertice({p.x + s.x, p.y}, {glyph.textureBottomRight.x, glyph.textureTopLeft.y}, col);
			this->pushVertice({p.x + s.x, p.y + s.y}, glyph.textureBottomRight, col);

			this->pushIndices(4, 3, 2);
			this->pushIndices(3, 1, 2);

			curpos.x += glyph.advance.x;
			curpos.y += glyph.advance.y;

			prevGlyph = &glyph;
		}
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

	void Stencil::setDrawMode(uint64_t mode) {
		if (this->_drawMode != mode) this->internalDraw(this->_renderTexture->id());
		this->_drawMode = mode;
	}

	void Stencil::internalDraw(bgfx::ViewId id) {
		if (this->_vertices.empty() || this->_indices.empty()) return;
		if ((this->_drawMode & BGFX_STATE_PT_LINES) == 0) bgfx::setTexture(0, this->_texColor, this->_textureHandle);

		/*
		Setup buffer (Transient are destroyed every frame, made for things that change a lot) ----

		TLDR: It's special buffer type for index/vertex buffers that are changing every frame. It's there as convenience as fire&forget, so you don't have to manually manage buffers that are changing all the time. Also, as we know what's life-time of these buffers, it allows some internal optimizations.
		- Transient - changes every frame
		- Dynamic - changes sometimes (if you don't pass data on creation, buffer/texture is dynamic)
		- Static - never changes (if you pass data to any creation function, it's assumed that buffer/texture is immutable)
		*/
		bgfx::TransientVertexBuffer vbh;
		bgfx::TransientIndexBuffer ibh;

		bgfx::allocTransientVertexBuffer(&vbh, static_cast<uint32_t>(this->_vertices.size()), this->_vLayout);
		bx::memCopy(vbh.data, this->_vertices.data(), this->_vertices.size() * this->_vLayout.m_stride);

		bgfx::allocTransientIndexBuffer(&ibh, static_cast<uint32_t>(this->_indices.size()));
		bx::memCopy(ibh.data, this->_indices.data(), this->_indices.size() * sizeof(uint16_t));

		bgfx::setVertexBuffer(0, &vbh, 0, static_cast<uint32_t>(this->_vertices.size()));
		bgfx::setIndexBuffer(&ibh, 0, static_cast<uint32_t>(this->_indices.size()));
		// ----------------------

		uint64_t flags = BGFX_STATE_DEFAULT_2D;
		if (this->_drawMode != 0) flags |= this->_drawMode;

		bgfx::setState(flags, 0);

		if (!this->_clips.empty()) {
			auto& clip = this->_clips.front();
			bgfx::setScissor(static_cast<uint16_t>(clip.pos.x), static_cast<uint16_t>(clip.pos.y), static_cast<uint16_t>(clip.size.x), static_cast<uint16_t>(clip.size.y));
		} else {
			bgfx::setScissor(); // Clear scissor
		}

		bgfx::submit(id, this->_stencilProgram);

		this->_vertices.clear();
		this->_indices.clear();
	}

	void Stencil::drawRecording() {
		if (this->_renderTexture == nullptr || !bgfx::isValid(this->_renderTexture->getHandle())) return;

		// TextureImage setup -----
		this->_textureHandle = _renderTexture->getHandle();
		this->_stencilProgram = this->_2dprogram;
		this->_drawMode = 0;
		// -----

		rawrBox::Vector2i size = this->_windowSize;

		this->pushVertice(0, 0, rawrBox::Colors::White);
		this->pushVertice({0, size.y}, {0, 1}, rawrBox::Colors::White);
		this->pushVertice({size.x, 0}, {1, 0}, rawrBox::Colors::White);
		this->pushVertice({size.x, size.y}, 1, rawrBox::Colors::White);

		this->pushIndices(4, 3, 2);
		this->pushIndices(3, 1, 2);

		this->internalDraw(this->_viewId); // Draw on main window
	}

	void Stencil::begin() {
		if (this->_recording) throw std::runtime_error("[RawrBox-Stencil] Already drawing, call 'end()' first");

		this->_renderTexture->startRecord();
		this->_recording = true;
	}

	void Stencil::end() {
		if (!this->_recording) throw std::runtime_error("[RawrBox-Stencil] Not drawing, call 'begin()' first");

		if (!this->_offsets.empty()) throw std::runtime_error("[RawrBox-Stencil] Missing 'popOffset', cannot draw");
		if (!this->_rotations.empty()) throw std::runtime_error("[RawrBox-Stencil] Missing 'popRotation', cannot draw");
		if (!this->_outlines.empty()) throw std::runtime_error("[RawrBox-Stencil] Missing 'popOutline', cannot draw");
		if (!this->_clips.empty()) throw std::runtime_error("[RawrBox-Stencil] Missing 'popClipping', cannot draw");
		if (!this->_scales.empty()) throw std::runtime_error("[RawrBox-Stencil] Missing 'popScale', cannot draw");

		this->internalDraw(this->_renderTexture->id()); // Draw remaining primitives

		this->_renderTexture->stopRecord();
		this->_recording = false;

		// Draw render texture with clipping --
		this->drawRecording();
	}
	// --------------------

	// ------ LOCATION
	void Stencil::pushOffset(const rawrBox::Vector2f& offset) {
		this->_offsets.push_back(offset);
		this->_offset += offset;
	}

	void Stencil::popOffset() {
		if (this->_offsets.empty()) throw std::runtime_error("[RawrBox-Stencil] Offset is empty, failed to pop");

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

	// ------ ROTATION
	void Stencil::pushRotation(const StencilRotation& rot) {
		this->_rotations.push_back(rot);
		this->_rotation += rot;
	}

	void Stencil::popRotation() {
		if (this->_rotations.empty()) throw std::runtime_error("[RawrBox-Stencil] Rotations is empty, failed to pop");

		this->_rotation -= this->_rotations.back();
		this->_rotations.pop_back();
	}
	// --------------------

	// ------ OUTLINE
	void Stencil::pushOutline(const StencilOutline& outline) {
		this->_outlines.push_back(outline);
		this->_outline += outline;
	}

	void Stencil::popOutline() {
		if (this->_outlines.empty()) throw std::runtime_error("[RawrBox-Stencil] Outline is empty, failed to pop");

		this->_outline -= this->_outlines.back();
		this->_outlines.pop_back();
	}
	// --------------------

	// ------ CLIPPING
	void Stencil::pushClipping(const rawrBox::AABB& rect) {
		this->_clips.push_back({rect.pos.x + this->_offset.x, rect.pos.y + this->_offset.y, rect.size.x, rect.size.y});
	}

	void Stencil::popClipping() {
		if (this->_clips.empty()) throw std::runtime_error("[RawrBox-Stencil] Clips is empty, failed to pop");

		this->internalDraw(this->_renderTexture->id());
		this->_clips.pop_back();
	}
	// --------------------

	// ------ SCALE
	void Stencil::pushScale(const rawrBox::Vector2f& scale) {
		this->_scales.push_back(scale);
		this->_scale += scale;
	}

	void Stencil::popScale() {
		if (this->_scales.empty()) throw std::runtime_error("[RawrBox-Stencil] Scale is empty, failed to pop");

		this->_scale -= this->_scales.back();
		this->_scales.pop_back();
	}
	// --------------------
} // namespace rawrBox
