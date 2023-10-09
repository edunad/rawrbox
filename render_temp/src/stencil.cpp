
#include <rawrbox/math/matrix4x4.hpp>
#include <rawrbox/math/utils/math.hpp>
#include <rawrbox/math/vector4.hpp>
#include <rawrbox/render_temp/static.hpp>
#include <rawrbox/render_temp/stencil.hpp>
#include <rawrbox/render_temp/utils/render.hpp>

#include <fmt/format.h>
#include <utf8.h>

#define BGFX_STATE_DEFAULT_2D (0 | BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_Z | BGFX_STATE_WRITE_A | BGFX_STATE_DEPTH_TEST_LEQUAL | BGFX_STATE_BLEND_ALPHA)
#define BGFX_DEFAULT_CLEAR    (0 | BGFX_CLEAR_STENCIL | BGFX_CLEAR_DEPTH)

// NOLINTBEGIN(*)
static const bgfx::EmbeddedShader stencil_shaders[] = {
    BGFX_EMBEDDED_SHADER(vs_stencil),
    BGFX_EMBEDDED_SHADER(fs_stencil),
    BGFX_EMBEDDED_SHADER_END()};

static const bgfx::EmbeddedShader stencil_line_shaders[] = {
    BGFX_EMBEDDED_SHADER(vs_stencil_line_stipple),
    BGFX_EMBEDDED_SHADER(fs_stencil_line_stipple),
    BGFX_EMBEDDED_SHADER_END()};

static const bgfx::EmbeddedShader stencil_text_shaders[] = {
    BGFX_EMBEDDED_SHADER(vs_stencil_text),
    BGFX_EMBEDDED_SHADER(fs_stencil_text),
    BGFX_EMBEDDED_SHADER_END()};
// NOLINTEND(*)

namespace rawrbox {
	bgfx::ViewId Stencil::renderID = 0;

	Stencil::Stencil(const rawrbox::Vector2i& size) : _windowSize(size), _renderId(rawrbox::STENCIL_VIEW_ID + ++Stencil::renderID) {
		// Shader layout
		this->_vLayout.begin()
		    .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
		    .add(bgfx::Attrib::TexCoord0, 3, bgfx::AttribType::Float)
		    .add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true)
		    .end();

		if (rawrbox::BGFX_INITIALIZED) {
			// SETUP STENCIL ---
			bgfx::setViewRect(this->_renderId, 0, 0, static_cast<uint16_t>(this->_windowSize.x), static_cast<uint16_t>(this->_windowSize.y));
			bgfx::setViewMode(this->_renderId, bgfx::ViewMode::Sequential);
			bgfx::setViewName(this->_renderId, fmt::format("RawrBox-RENDERER-STENCIL-{}", this->_renderId).c_str());
			bgfx::setViewClear(this->_renderId, BGFX_DEFAULT_CLEAR, 0x00000000, 1.0F, 0);
			// ---
		}
	}

	Stencil::~Stencil() {
		// CLEANUP BUFFERS & PROGRAMS
		RAWRBOX_DESTROY(this->_2dprogram);
		RAWRBOX_DESTROY(this->_lineprogram);
		RAWRBOX_DESTROY(this->_textprogram);
		RAWRBOX_DESTROY(this->_texColor);

		this->_drawCalls.clear();
	}

	void Stencil::upload() {
		if (bgfx::isValid(this->_2dprogram)) throw std::runtime_error("[RawrBox-Stencil] Upload already called");

		// Load Shaders --------
		rawrbox::RenderUtils::buildShader(stencil_shaders, this->_2dprogram);
		rawrbox::RenderUtils::buildShader(stencil_line_shaders, this->_lineprogram);
		rawrbox::RenderUtils::buildShader(stencil_text_shaders, this->_textprogram);
		// ------------------

		this->_texColor = bgfx::createUniform("s_texColor", bgfx::UniformType::Sampler);
	}

	void Stencil::resize(const rawrbox::Vector2i& size) {
		this->_windowSize = size;

		bgfx::setViewRect(this->_renderId, 0, 0, static_cast<uint16_t>(size.x), static_cast<uint16_t>(size.y));
		bgfx::setViewClear(this->_renderId, BGFX_DEFAULT_CLEAR, 0x00000000, 1.0F, 0);
	}

	void Stencil::pushVertice(rawrbox::Vector2f pos, const rawrbox::Vector3f& uv, const rawrbox::Color& col) {
		auto wSize = this->_windowSize.cast<float>();

		this->applyScale(pos);
		this->applyRotation(pos);

		this->_currentDraw.vertices.emplace_back(
		    // pos
		    ((pos.x + this->_offset.x) / wSize.x * 2 - 1),
		    ((pos.y + this->_offset.y) / wSize.y * 2 - 1) * -1,
		    1.0F,

		    // uv
		    uv.x,
		    uv.y,
		    uv.z,

		    // color
		    col.pack());
	}

	void Stencil::pushIndices(std::vector<uint16_t> ind) {
		this->_currentDraw.indices.insert(this->_currentDraw.indices.end(), ind.begin(), ind.end());
	}

	void Stencil::applyRotation(rawrbox::Vector2f& vert) {
		if (this->_rotation.rotation == 0) return;

		rawrbox::Matrix4x4 translationMatrix = {};
		translationMatrix.translate({-_rotation.origin.x, -_rotation.origin.y, 0});

		rawrbox::Matrix4x4 rotationMatrix = {};
		rotationMatrix.rotateZ(-rawrbox::MathUtils::toRad(_rotation.rotation));

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
	void Stencil::drawPolygon(const rawrbox::Polygon& poly) {
		// Setup --------
		bgfx::TextureHandle handl = BGFX_INVALID_HANDLE;
		if (rawrbox::WHITE_TEXTURE != nullptr) handl = rawrbox::WHITE_TEXTURE->getHandle();

		this->setupDrawCall(
		    this->_2dprogram,
		    handl);
		// ----

		if (this->_outline.isSet()) {
			for (auto ind = poly.indices.begin(); ind != poly.indices.end();) {
				auto nextInd = std::next(ind);
				if (nextInd == poly.indices.end()) break;

				// TODO: IGNORE MIDDLE LINES
				this->drawLine(poly.verts[(*ind)].pos, poly.verts[(*nextInd)].pos, poly.verts[(*ind)].col);
				ind++;
			}

			// Close loop
			this->drawLine(poly.verts[poly.indices.back()].pos, poly.verts[poly.indices.front()].pos, poly.verts[poly.indices.back()].col);

		} else {
			for (auto& v : poly.verts)
				this->pushVertice(v.pos, v.uv, v.col);

			this->_currentDraw.indices.insert(this->_currentDraw.indices.end(), poly.indices.begin(), poly.indices.end());
		}

		// Add to calls
		this->pushDrawCall();
		// ----
	}

	void Stencil::drawTriangle(const rawrbox::Vector2f& a, const rawrbox::Vector2f& aUV, const rawrbox::Color& colA, const rawrbox::Vector2f& b, const rawrbox::Vector2f& bUV, const rawrbox::Color& colB, const rawrbox::Vector2f& c, const rawrbox::Vector2f& cUV, const rawrbox::Color& colC) {
		if (colA.isTransparent() && colB.isTransparent() && colC.isTransparent()) return;

		// Setup --------
		bgfx::TextureHandle handl = BGFX_INVALID_HANDLE;
		if (rawrbox::WHITE_TEXTURE != nullptr) handl = rawrbox::WHITE_TEXTURE->getHandle();

		this->setupDrawCall(
		    this->_2dprogram,
		    handl);
		// ----

		if (this->_outline.isSet()) {
			float thick = this->_outline.thickness;

			this->drawLine({a.x, a.y - thick}, {b.x, b.y + thick / 2.F}, colA);
			this->drawLine(b, c, colB);
			this->drawLine({c.x + thick / 2.F, c.y}, a, colC);
		} else {
			this->pushVertice(a, aUV, colA);
			this->pushVertice(b, bUV, colB);
			this->pushVertice(c, cUV, colC);

			this->pushIndices({0, 1, 2});
		}

		// Add to calls
		this->pushDrawCall();
		// ----
	}

	void Stencil::drawBox(const rawrbox::Vector2f& pos, const rawrbox::Vector2f& size, const rawrbox::Color& col) {
		if (this->_outline.isSet()) {
			float thick = this->_outline.thickness;

			this->drawLine({pos.x - thick, pos.y}, {pos.x + size.x, pos.y}, col);
			this->drawLine({pos.x, pos.y + size.y}, {pos.x, pos.y}, col);
			this->drawLine({pos.x + size.x, pos.y - thick}, {pos.x + size.x, pos.y + size.y}, col);
			this->drawLine({pos.x + size.x + (thick > 1.F ? thick : 0.F), pos.y + size.y}, {pos.x - thick, pos.y + size.y}, col);
		} else {
			if (rawrbox::WHITE_TEXTURE == nullptr) return;
			this->drawTexture(pos, size, *rawrbox::WHITE_TEXTURE, col);
		}
	}

	void Stencil::drawTexture(const rawrbox::Vector2f& pos, const rawrbox::Vector2f& size, const bgfx::TextureHandle& tex, const rawrbox::Color& col, const rawrbox::Vector2f& uvStart, const rawrbox::Vector2f& uvEnd, uint32_t atlas) {
		if (col.isTransparent()) return;

		// Setup --------
		this->setupDrawCall(
		    this->_2dprogram,
		    tex);
		// ----

		auto a = static_cast<float>(atlas);

		this->pushVertice({pos.x, pos.y}, {uvStart.x, uvStart.y, a}, col);
		this->pushVertice({pos.x, pos.y + size.y}, {uvStart.x, uvEnd.y, a}, col);
		this->pushVertice({pos.x + size.x, pos.y}, {uvEnd.x, uvStart.y, a}, col);
		this->pushVertice({pos.x + size.x, pos.y + size.y}, {uvEnd.x, uvEnd.y, a}, col);

		this->pushIndices({0, 1, 2,
		    1, 3, 2});

		// Add to calls
		this->pushDrawCall();
		// ----
	}

	void Stencil::drawTexture(const rawrbox::Vector2f& pos, const rawrbox::Vector2f& size, const rawrbox::TextureBase& tex, const rawrbox::Color& col, const rawrbox::Vector2f& uvStart, const rawrbox::Vector2f& uvEnd, uint32_t atlas) {
		this->drawTexture(pos, size, tex.getHandle(), col, uvStart, uvEnd, atlas);
	}

	void Stencil::drawCircle(const rawrbox::Vector2f& pos, const rawrbox::Vector2f& size, const rawrbox::Color& col, size_t roundness, float angleStart, float angleEnd) {
		if (col.isTransparent()) return;

		auto radius = size / 2;
		auto targetPos = pos + radius;

		float angStartRad = rawrbox::MathUtils::toRad(angleStart);
		float angEndRad = rawrbox::MathUtils::toRad(angleEnd);

		float space = rawrbox::pi<float> / roundness * 2;

		for (size_t i = 0; i < roundness;) {
			auto ang = space * i++ + angStartRad;
			if (ang + space > angEndRad) break;

			rawrbox::Vector2 b = targetPos + rawrbox::Vector2::cosSin(ang) * radius;
			rawrbox::Vector2 c = targetPos + rawrbox::Vector2::cosSin(ang + space) * radius;

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

		rawrbox::StencilOutline outline = {1.F, 0.F}; // Default line size
		if (this->_outline.isSet()) outline = this->_outline;
		if (outline.thickness <= 0.F) return;

		bool usePTLines = outline.thickness == 1.F;

		// Setup --------
		bgfx::TextureHandle handl = BGFX_INVALID_HANDLE;
		if (rawrbox::WHITE_TEXTURE != nullptr) handl = rawrbox::WHITE_TEXTURE->getHandle();

		this->setupDrawCall(
		    (usePTLines || outline.stipple > 0.F) ? this->_lineprogram : this->_2dprogram,
		    handl,
		    usePTLines ? BGFX_STATE_PT_LINES : 0);

		// ----

		if (usePTLines) {
			this->pushVertice(from, {0, 0}, col);
			this->pushVertice(to, {outline.stipple, outline.stipple}, col);
			this->pushIndices({0, 1});
		} else {
			float angle = -from.angle(to);
			float uvEnd = outline.stipple <= 0.F ? 1.F : outline.stipple;

			auto vertA = from + rawrbox::Vector2::cosSin(angle) * outline.thickness;
			auto vertB = from + rawrbox::Vector2::cosSin(angle) * -outline.thickness;
			auto vertC = to + rawrbox::Vector2::cosSin(angle) * outline.thickness;
			auto vertD = to + rawrbox::Vector2::cosSin(angle) * -outline.thickness;

			this->pushVertice(vertA, {0, 0}, col);
			this->pushVertice(vertB, {0, uvEnd}, col);
			this->pushVertice(vertC, {uvEnd, 0}, col);
			this->pushVertice(vertD, {uvEnd, uvEnd}, col);

			this->pushIndices({0, 1, 2,
			    1, 3, 2});
		}

		// Add to calls
		this->pushDrawCall();
		// ----
	}

	void Stencil::drawText(const rawrbox::Font& font, const std::string& text, const rawrbox::Vector2f& pos, const rawrbox::Color& col, rawrbox::Alignment alignX, rawrbox::Alignment alignY) {
		if (col.isTransparent() || text.empty()) return;

		rawrbox::Vector2f startpos = pos;
		rawrbox::Vector2f tsize = font.getStringSize(text);

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

		font.render(text, startpos, false, [this, &font, col](rawrbox::Glyph* glyph, float x0, float y0, float x1, float y1) {
			// Setup --------
			this->setupDrawCall(
			    this->_textprogram,
			    font.getPackTexture(glyph)->getHandle());
			// ----

			this->pushVertice({x0, y0}, glyph->textureTopLeft, col);
			this->pushVertice({x0, y1}, {glyph->textureTopLeft.x, glyph->textureBottomRight.y}, col);
			this->pushVertice({x1, y0}, {glyph->textureBottomRight.x, glyph->textureTopLeft.y}, col);
			this->pushVertice({x1, y1}, glyph->textureBottomRight, col);

			this->pushIndices({0, 1, 2,
			    1, 3, 2});

			// Add to calls
			this->pushDrawCall();
			// ----
		});
	}
	// --------------------

	// ------RENDERING
	void Stencil::setupDrawCall(const bgfx::ProgramHandle& program, const bgfx::TextureHandle& texture, uint64_t drawMode) {
		this->_currentDraw.clear();

		this->_currentDraw.stencilProgram = program;
		this->_currentDraw.textureHandle = texture;
		this->_currentDraw.drawMode = drawMode;
		this->_currentDraw.clip = this->_clips.empty() ? UINT16_MAX : this->_clips.back();
		this->_currentDraw.cull = this->_culling && this->_scale.x >= 0.F && this->_scale.y >= 0.F;
	}

	void Stencil::pushDrawCall() {
		if (!this->_drawCalls.empty()) {
			auto& oldCall = this->_drawCalls.back();
			bool canMerge = oldCall.clip == this->_currentDraw.clip &&
					oldCall.cull == this->_currentDraw.cull &&
					oldCall.drawMode == this->_currentDraw.drawMode &&
					oldCall.stencilProgram.idx == this->_currentDraw.stencilProgram.idx &&
					oldCall.textureHandle.idx == this->_currentDraw.textureHandle.idx;

			if (canMerge) {
				for (auto& ind : this->_currentDraw.indices) {
					oldCall.indices.push_back(static_cast<uint16_t>(oldCall.vertices.size()) + ind);
				}

				oldCall.vertices.insert(oldCall.vertices.end(), this->_currentDraw.vertices.begin(), this->_currentDraw.vertices.end());
				return;
			}
		}

		this->_drawCalls.push_back(this->_currentDraw);
	}

	void Stencil::internalDraw() {
		if (this->_drawCalls.empty()) return;

		this->_prevViewId = rawrbox::CURRENT_VIEW_ID;
		rawrbox::CURRENT_VIEW_ID = this->_renderId;

		bgfx::touch(rawrbox::CURRENT_VIEW_ID);                              // Make sure we draw on the view
		bgfx::setViewTransform(rawrbox::CURRENT_VIEW_ID, nullptr, nullptr); // Clear view

		for (auto& group : this->_drawCalls) {
			if (!bgfx::isValid(group.stencilProgram) || !bgfx::isValid(group.textureHandle)) continue;
			if (group.vertices.empty() || group.indices.empty()) continue;

			auto vertSize = static_cast<uint32_t>(group.vertices.size());
			auto indSize = static_cast<uint32_t>(group.indices.size());

			bgfx::TransientVertexBuffer tvb = {};
			bgfx::TransientIndexBuffer tib = {};

			if (!bgfx::allocTransientBuffers(&tvb, this->_vLayout, vertSize, &tib, indSize)) continue;

			std::memcpy(tvb.data, group.vertices.data(), vertSize * this->_vLayout.getStride());
			std::memcpy(tib.data, group.indices.data(), indSize * sizeof(uint16_t));

			bgfx::setTexture(0, this->_texColor, group.textureHandle);
			bgfx::setVertexBuffer(0, &tvb);
			bgfx::setIndexBuffer(&tib);

			uint64_t flags = BGFX_STATE_DEFAULT_2D;
			if (group.cull) flags |= BGFX_STATE_CULL_CW;
			if (group.drawMode != 0) flags |= group.drawMode;

			bgfx::setState(flags, 0);

			bgfx::setScissor(group.clip);
			bgfx::submit(rawrbox::CURRENT_VIEW_ID, group.stencilProgram);
		}

		bgfx::discard(BGFX_DISCARD_ALL);
		this->_drawCalls.clear();

		rawrbox::CURRENT_VIEW_ID = this->_prevViewId;
	}

	void Stencil::render() {
		if (!this->_offsets.empty()) throw std::runtime_error("[RawrBox-Stencil] Missing 'popOffset', cannot draw");
		if (!this->_rotations.empty()) throw std::runtime_error("[RawrBox-Stencil] Missing 'popRotation', cannot draw");
		if (!this->_outlines.empty()) throw std::runtime_error("[RawrBox-Stencil] Missing 'popOutline', cannot draw");
		if (!this->_clips.empty()) throw std::runtime_error("[RawrBox-Stencil] Missing 'popClipping', cannot draw");
		if (!this->_scales.empty()) throw std::runtime_error("[RawrBox-Stencil] Missing 'popScale', cannot draw");

		this->internalDraw();
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
		rawrbox::AABB fixedRect = rawrbox::AABB{rect.pos.x + this->_offset.x, rect.pos.y + this->_offset.y, rect.size.x, rect.size.y};
		this->_clips.push_back(bgfx::setScissor(static_cast<uint16_t>(fixedRect.pos.x), static_cast<uint16_t>(fixedRect.pos.y), static_cast<uint16_t>(fixedRect.size.x), static_cast<uint16_t>(fixedRect.size.y)));
	}

	void Stencil::popClipping() {
		if (this->_clips.empty()) throw std::runtime_error("[RawrBox-Stencil] Clips is empty, failed to pop");
		this->_clips.pop_back();
	}
	// --------------------

	// ------ SCALE
	void Stencil::pushScale(const rawrbox::Vector2f& scale) {
		this->_scales.push_back(scale);
		this->_scale += scale;
	}

	void Stencil::popScale() {
		if (this->_scales.empty()) throw std::runtime_error("[RawrBox-Stencil] Scale is empty, failed to pop");

		this->_scale -= this->_scales.back();
		this->_scales.pop_back();
	}
	// --------------------

	// ------ CULLING
	void Stencil::pushDisableCulling() {
		this->_culling = false;
	}

	void Stencil::popDisableCulling() {
		this->_culling = true;
	}
	// --------------------

	// ------ OTHER
	const std::vector<rawrbox::StencilDraw> Stencil::getDrawCalls() const { return this->_drawCalls; }
	void Stencil::clear() { this->_drawCalls.clear(); }
	// --------------------
} // namespace rawrbox
