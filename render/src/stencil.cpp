
#include <rawrbox/math/matrix4x4.hpp>
#include <rawrbox/math/vector4.hpp>
#include <rawrbox/render/shader_defines.hpp>
#include <rawrbox/render/static.hpp>
#include <rawrbox/render/stencil.hpp>

// Compiled shaders
#include <generated/shaders/render/all.hpp>

#include <bx/math.h>
#include <utf8.h>

#include <array>

#define BGFX_STATE_DEFAULT_2D (0 | BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA))

// NOLINTBEGIN(*)
static const bgfx::EmbeddedShader stencil_shaders[] = {
    BGFX_EMBEDDED_SHADER(vs_stencil),
    BGFX_EMBEDDED_SHADER(fs_stencil),
    BGFX_EMBEDDED_SHADER(fs_stencil_line_stipple),
    BGFX_EMBEDDED_SHADER(vs_stencil_line_stipple),
    BGFX_EMBEDDED_SHADER(fs_stencil_text),
    BGFX_EMBEDDED_SHADER(vs_stencil_text),
    BGFX_EMBEDDED_SHADER_END()};
// NOLINTEND(*)

namespace rawrbox {
	Stencil::Stencil(const rawrbox::Vector2i& size) : _windowSize(size) {
		// Shader layout
		this->_vLayout.begin()
		    .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
		    .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
		    .add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true)
		    .end();
	}

	Stencil::~Stencil() {
		// CLEANUP BUFFERS & PROGRAMS
		RAWRBOX_DESTROY(this->_2dprogram);
		RAWRBOX_DESTROY(this->_lineprogram);
		RAWRBOX_DESTROY(this->_textprogram);
		RAWRBOX_DESTROY(this->_stencilProgram);

		RAWRBOX_DESTROY(this->_textureHandle);
		RAWRBOX_DESTROY(this->_texColor);

		this->_pixelTexture = nullptr;
		this->_renderTexture = nullptr;

		this->_indices.clear();
		this->_vertices.clear();
	}

	void Stencil::upload() {
		if (this->_renderTexture != nullptr || this->_pixelTexture != nullptr) throw std::runtime_error("[RawrBox-Stencil] Upload already called");
		bgfx::RendererType::Enum type = bgfx::getRendererType();

		// Load 2D --------
		bgfx::ShaderHandle vsh = bgfx::createEmbeddedShader(stencil_shaders, type, "vs_stencil");
		bgfx::ShaderHandle fsh = bgfx::createEmbeddedShader(stencil_shaders, type, "fs_stencil");

		this->_2dprogram = bgfx::createProgram(vsh, fsh, true);
		if (!bgfx::isValid(this->_2dprogram)) throw std::runtime_error("[RawrBox-Stencil] Failed to upload '2d' shader program");
		// ------------------

		// Load Line ---------
		vsh = bgfx::createEmbeddedShader(stencil_shaders, type, "vs_stencil_line_stipple");
		fsh = bgfx::createEmbeddedShader(stencil_shaders, type, "fs_stencil_line_stipple");

		this->_lineprogram = bgfx::createProgram(vsh, fsh, true);
		if (!bgfx::isValid(this->_lineprogram)) throw std::runtime_error("[RawrBox-Stencil] Failed to upload 'line' shader program");
		// --------------------

		// Load Text ---------
		vsh = bgfx::createEmbeddedShader(stencil_shaders, type, "vs_stencil_text");
		fsh = bgfx::createEmbeddedShader(stencil_shaders, type, "fs_stencil_text");

		this->_textprogram = bgfx::createProgram(vsh, fsh, true);
		if (!bgfx::isValid(this->_textprogram)) throw std::runtime_error("[RawrBox-Stencil] Failed to upload 'text' shader program");
		// --------------------

		this->_texColor = bgfx::createUniform("s_texColor", bgfx::UniformType::Sampler);

		this->_renderTexture = std::make_shared<rawrbox::TextureRender>(this->_windowSize);
		this->_pixelTexture = std::make_shared<rawrbox::TextureFlat>(rawrbox::Vector2i(1, 1), Colors::White);
		this->_renderTexture->upload();
		this->_pixelTexture->upload();
	}

	void Stencil::resize(const rawrbox::Vector2i& size) {
		this->_windowSize = size;
	}

	void Stencil::pushVertice(rawrbox::Vector2f pos, const rawrbox::Vector2f& uv, const rawrbox::Color& col) {
		this->applyScale(pos);
		this->applyRotation(pos);

		this->_vertices.emplace_back(
		    // pos
		    ((pos.x + this->_offset.x) / _windowSize.x * 2 - 1),
		    ((pos.y + this->_offset.y) / _windowSize.y * 2 - 1) * -1,
		    0.0F,

		    // uv
		    uv.x,
		    uv.y,

		    // color
		    col.pack());
	}

	void Stencil::pushIndices(uint16_t a, uint16_t b, uint16_t c) {
		auto pos = static_cast<uint16_t>(this->_vertices.size());

		this->_indices.push_back(pos - a);
		this->_indices.push_back(pos - b);
		this->_indices.push_back(pos - c);
	}

	void Stencil::applyRotation(rawrbox::Vector2f& vert) {
		if (this->_rotation.rotation == 0) return;

		rawrbox::Matrix4x4 translationMatrix = {};
		translationMatrix.translate({-_rotation.origin.x, -_rotation.origin.y, 0});

		rawrbox::Matrix4x4 rotationMatrix = {};
		rotationMatrix.rotateZ(bx::toRad(_rotation.rotation));

		rawrbox::Matrix4x4 reverseTranslationMatrix = {};
		reverseTranslationMatrix.translate({_rotation.origin.x, _rotation.origin.y, 0});

		rawrbox::Matrix4x4 mul = translationMatrix * rotationMatrix * reverseTranslationMatrix;

		rawrbox::Vector4f v = {vert.x, vert.y, 0, -1.0F};
		auto res = mul.mulVec(v);

		vert.x = res.x;
		vert.y = res.y;
	}

	void Stencil::applyScale(rawrbox::Vector2f& vert) {
		if (this->_scale == 0) return;

		rawrbox::Matrix4x4 scaleMtx = {};
		scaleMtx.scale({this->_scale.x, this->_scale.y, 1.F});

		rawrbox::Vector4f v = {vert.x, vert.y, 0, -1.0F};
		auto res = scaleMtx.mulVec(v);

		vert.x = res.x;
		vert.y = res.y;
	}
	// --------------------

	// ------UTILS
	void Stencil::drawTriangle(const rawrbox::Vector2f& a, const rawrbox::Vector2f& aUV, const rawrbox::Color& colA, const rawrbox::Vector2f& b, const rawrbox::Vector2f& bUV, const rawrbox::Color& colB, const rawrbox::Vector2f& c, const rawrbox::Vector2f& cUV, const rawrbox::Color& colC) {
		if (colA.isTransparent() && colB.isTransparent() && colC.isTransparent()) return;

		if (this->_outline.isSet()) {
			float thick = this->_outline.thickness;

			this->drawLine({a.x, a.y - thick}, {b.x, b.y + thick / 2.F}, colA);
			this->drawLine(b, c, colB);
			this->drawLine({c.x + thick / 2.F, c.y}, a, colC);
		} else {
			// Setup --------
			if (this->_pixelTexture != nullptr) this->setTexture(this->_pixelTexture->getHandle());
			this->setShaderProgram(this->_2dprogram);
			this->setDrawMode();
			// ----

			this->pushVertice(a, aUV, colA);
			this->pushVertice(b, bUV, colB);
			this->pushVertice(c, cUV, colC);

			this->pushIndices(3, 2, 1);
		}
	}

	void Stencil::drawBox(const rawrbox::Vector2f& pos, const rawrbox::Vector2f& size, const rawrbox::Color& col) {
		if (this->_outline.isSet()) {
			float thick = this->_outline.thickness;

			this->drawLine({pos.x - thick, pos.y}, {pos.x + size.x, pos.y}, col);
			this->drawLine({pos.x + size.x, pos.y - thick}, {pos.x + size.x, pos.y + size.y}, col);
			this->drawLine({pos.x + size.x + (thick > 1.F ? thick : 0.F), pos.y + size.y}, {pos.x - thick, pos.y + size.y}, col);
			this->drawLine({pos.x, pos.y + size.y}, {pos.x, pos.y}, col);
		} else {
			this->drawTexture(pos, size, this->_pixelTexture, col);
		}
	}

	void Stencil::drawTexture(const rawrbox::Vector2f& pos, const rawrbox::Vector2f& size, std::shared_ptr<rawrbox::TextureBase> tex, const rawrbox::Color& col, const rawrbox::Vector2f& uvStart, const rawrbox::Vector2f& uvEnd) {
		if (col.isTransparent()) return;

		// TextureImage setup -----
		if (tex != nullptr) this->setTexture(tex->getHandle());
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

	void Stencil::drawCircle(const rawrbox::Vector2f& pos, const rawrbox::Vector2f& size, const rawrbox::Color& col, size_t roundness, float angleStart, float angleEnd) {
		if (col.isTransparent()) return;

		// Setup --------
		if (!this->_outline.isSet()) {
			if (this->_pixelTexture != nullptr) this->setTexture(this->_pixelTexture->getHandle());
			this->setShaderProgram(this->_2dprogram);
			this->setDrawMode();
		}
		// ----

		auto radius = size / 2;
		auto targetPos = pos + radius;

		float angStartRad = bx::toRad(angleStart);
		float angEndRad = bx::toRad(angleEnd);

		float space = rawrbox::pi<float> / roundness * 2;

		for (size_t i = 0; i < roundness;) {
			auto ang = space * i++ + angStartRad;
			if (ang + space > angEndRad) break;

			rawrbox::Vector2 b = targetPos + rawrbox::Vector2::sinCos(ang) * radius;
			rawrbox::Vector2 c = targetPos + rawrbox::Vector2::sinCos(ang + space) * radius;

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

	void Stencil::drawLine(const rawrbox::Vector2& from, const rawrbox::Vector2& to, const rawrbox::Color& col) {
		if (col.isTransparent()) return;

		rawrbox::StencilOutline outline = {1.F, 0.F};
		if (this->_outline.isSet()) outline = this->_outline;
		if (outline.thickness <= 0.F) return;

		bool usePTLines = outline.thickness == 1.F;

		// Setup --------
		if (this->_pixelTexture != nullptr) this->setTexture(this->_pixelTexture->getHandle());
		this->setShaderProgram((usePTLines || outline.stipple > 0.F) ? this->_lineprogram : this->_2dprogram);
		this->setDrawMode(usePTLines ? BGFX_STATE_PT_LINES : 0); // Reset
		// ----

		if (usePTLines) {
			this->pushVertice(from, {0, 0}, col);
			this->pushVertice(to, {outline.stipple, outline.stipple}, col);

			auto pos = static_cast<uint16_t>(this->_vertices.size());
			this->_indices.push_back(pos - 1);
			this->_indices.push_back(pos - 2);
		} else {
			float angle = from.angle(to);
			float uvEnd = outline.stipple <= 0.F ? 1.F : outline.stipple;

			auto vertA = from + rawrbox::Vector2::sinCos(angle) * outline.thickness;
			auto vertB = from + rawrbox::Vector2::sinCos(angle) * -outline.thickness;
			auto vertC = to + rawrbox::Vector2::sinCos(angle) * outline.thickness;
			auto vertD = to + rawrbox::Vector2::sinCos(angle) * -outline.thickness;

			this->pushVertice(vertA, {0, 0}, col);
			this->pushVertice(vertC, {uvEnd, 0}, col);
			this->pushVertice(vertB, {0, uvEnd}, col);
			this->pushVertice(vertD, {uvEnd, uvEnd}, col);

			this->pushIndices(4, 3, 2);
			this->pushIndices(3, 1, 2);
		}
	}

	void Stencil::drawText(std::weak_ptr<rawrbox::Font> font, const std::string& text, const rawrbox::Vector2f& pos, const rawrbox::Color& col, rawrbox::Alignment alignX, rawrbox::Alignment alignY) {
		if (font.expired() || col.isTransparent() || text.empty()) return;
		auto f = font.lock();

		// Setup --------
		this->setShaderProgram(this->_textprogram);
		this->setDrawMode(); // Reset
		// ----

		rawrbox::Vector2f startpos = pos;
		rawrbox::Vector2f tsize = f->getStringSize(text);
		if (alignX != Alignment::Left || alignY != Alignment::Left) {
			switch (alignX) {
				case Alignment::Left:
					break;
				case Alignment::Center:
					startpos.x -= tsize.x / 2;
					break;
				case Alignment::Right:
					startpos.x -= tsize.x;
					break;
			}

			switch (alignY) {
				case Alignment::Left:
					break;
				case Alignment::Center:
					startpos.y -= tsize.y / 2;
					break;
				case Alignment::Right:
					startpos.y -= tsize.y;
					break;
			}
		}

		startpos.x = std::roundf(startpos.x);
		startpos.y = std::roundf(startpos.y);

		float lineheight = f->getLineHeight();
		startpos.y += lineheight + static_cast<float>(f->face->size->metrics.descender >> 6);

		rawrbox::Vector2 curpos = startpos;
		const rawrbox::Glyph* prevGlyph = nullptr;

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

			auto& glyph = f->getGlyph(point);
			if (prevGlyph != nullptr) {
				curpos.x += f->getKerning(glyph, *prevGlyph);
			}

			rawrbox::Vector2 p = {curpos.x + glyph.bearing.x, curpos.y - glyph.bearing.y};
			rawrbox::Vector2 s = {static_cast<float>(glyph.size.x), static_cast<float>(glyph.size.y)};

			this->setTexture(f->getAtlasTexture(glyph)->getHandle());

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
		if (this->_textureHandle.idx != tex.idx) this->internalDraw();
		this->_textureHandle = tex;
	}

	void Stencil::setShaderProgram(const bgfx::ProgramHandle& handle) {
		if (!bgfx::isValid(handle)) return;

		if (this->_stencilProgram.idx != handle.idx) this->internalDraw();
		this->_stencilProgram = handle;
	}

	void Stencil::setDrawMode(uint64_t mode) {
		if (this->_drawMode != mode) this->internalDraw();
		this->_drawMode = mode;
	}

	void Stencil::internalDraw() {
		if (this->_vertices.empty() || this->_indices.empty()) return;
		bgfx::setTexture(0, this->_texColor, this->_textureHandle);

		auto vertSize = static_cast<uint32_t>(this->_vertices.size());
		auto indSize = static_cast<uint32_t>(this->_indices.size());

		bgfx::TransientVertexBuffer tvb = {};
		bgfx::TransientIndexBuffer tib = {};

		if (!bgfx::allocTransientBuffers(&tvb, this->_vLayout, vertSize, &tib, indSize)) return;

		bx::memCopy(tvb.data, this->_vertices.data(), vertSize * this->_vLayout.m_stride);
		bx::memCopy(tib.data, this->_indices.data(), indSize * sizeof(uint16_t));

		bgfx::setVertexBuffer(0, &tvb);
		bgfx::setIndexBuffer(&tib);

		uint64_t flags = BGFX_STATE_DEFAULT_2D;
		if (this->_cull) flags |= BGFX_STATE_CULL_CW;
		if (this->_drawMode != 0) flags |= this->_drawMode;

		bgfx::setState(flags, 0);

		if (!this->_clips.empty()) {
			auto& clip = this->_clips.front();
			bgfx::setScissor(static_cast<uint16_t>(clip.pos.x), static_cast<uint16_t>(clip.pos.y), static_cast<uint16_t>(clip.size.x), static_cast<uint16_t>(clip.size.y));
		} else {
			bgfx::setScissor(); // Clear scissor
		}

		bgfx::submit(rawrbox::CURRENT_VIEW_ID, this->_stencilProgram);

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

		rawrbox::Vector2f size = this->_windowSize.cast<float>();

		this->pushVertice({0, 0}, {0, 0}, rawrbox::Colors::White);
		this->pushVertice({0, size.y}, {0, 1}, rawrbox::Colors::White);
		this->pushVertice({size.x, 0}, {1, 0}, rawrbox::Colors::White);
		this->pushVertice({size.x, size.y}, {1, 1}, rawrbox::Colors::White);

		this->pushIndices(4, 3, 2);
		this->pushIndices(3, 1, 2);

		this->internalDraw(); // Draw on main window
	}

	void Stencil::begin() {
		if (this->_recording) throw std::runtime_error("[RawrBox-Stencil] Already drawing, call 'end()' first");

		this->_renderTexture->startRecord();
		this->_recording = true;

		bgfx::setViewTransform(rawrbox::CURRENT_VIEW_ID, nullptr, nullptr);
	}

	void Stencil::end() {
		if (!this->_recording) throw std::runtime_error("[RawrBox-Stencil] Not drawing, call 'begin()' first");

		if (!this->_offsets.empty()) throw std::runtime_error("[RawrBox-Stencil] Missing 'popOffset', cannot draw");
		if (!this->_rotations.empty()) throw std::runtime_error("[RawrBox-Stencil] Missing 'popRotation', cannot draw");
		if (!this->_outlines.empty()) throw std::runtime_error("[RawrBox-Stencil] Missing 'popOutline', cannot draw");
		if (!this->_clips.empty()) throw std::runtime_error("[RawrBox-Stencil] Missing 'popClipping', cannot draw");
		if (!this->_scales.empty()) throw std::runtime_error("[RawrBox-Stencil] Missing 'popScale', cannot draw");

		this->internalDraw(); // Draw remaining primitives

		this->_renderTexture->stopRecord();
		this->_recording = false;

		// Draw render texture with clipping --
		this->drawRecording();
	}
	// --------------------

	// ------ LOCATION
	void Stencil::pushOffset(const rawrbox::Vector2f& offset) {
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
	void Stencil::pushClipping(const rawrbox::AABB& rect) {
		this->_clips.emplace_back(rect.pos.x + this->_offset.x, rect.pos.y + this->_offset.y, rect.size.x, rect.size.y);
	}

	void Stencil::popClipping() {
		if (this->_clips.empty()) throw std::runtime_error("[RawrBox-Stencil] Clips is empty, failed to pop");

		this->internalDraw();
		this->_clips.pop_back();
	}
	// --------------------

	// ------ SCALE
	void Stencil::pushScale(const rawrbox::Vector2f& scale) {
		this->_scales.push_back(scale);
		this->_scale += scale;

		this->_cull = this->_scale.x > 0 && this->_scale.y > 0;
	}

	void Stencil::popScale() {
		if (this->_scales.empty()) throw std::runtime_error("[RawrBox-Stencil] Scale is empty, failed to pop");

		this->_scale -= this->_scales.back();
		this->_scales.pop_back();
	}
	// --------------------

	// ------ OTHER
	const std::vector<PosUVColorVertexData>& Stencil::getVertices() const { return this->_vertices; }
	const std::vector<uint16_t>& Stencil::getIndices() const { return this->_indices; }
	void Stencil::clear() { this->_vertices.clear(); }
	// --------------------
} // namespace rawrbox
