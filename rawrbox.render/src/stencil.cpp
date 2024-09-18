
#include <rawrbox/math/utils/math.hpp>
#include <rawrbox/render/bindless.hpp>
#include <rawrbox/render/static.hpp>
#include <rawrbox/render/stencil.hpp>
#include <rawrbox/utils/time.hpp>

#pragma warning(push)
#pragma warning(disable : 4505)
#include <stb/stb_easy_font.hpp>
#pragma warning(pop)

namespace rawrbox {
	Stencil::Stencil(const rawrbox::Vector2u& size) : _size(size) {
		this->_streamingVB = std::make_unique<rawrbox::StreamingBuffer>("RawrBox::Stencil::VertexBuffer", Diligent::BIND_VERTEX_BUFFER, MaxVertsInStreamingBuffer * static_cast<uint32_t>(sizeof(rawrbox::PosUVColorVertexData)), 1);
		this->_streamingIB = std::make_unique<rawrbox::StreamingBuffer>("RawrBox::Stencil::IndexBuffer", Diligent::BIND_INDEX_BUFFER, MaxVertsInStreamingBuffer * 3 * static_cast<uint32_t>(sizeof(uint32_t)), 1);

		// Only supported on DX12 / Vulkan --
		this->_streamingVB->setPersistent(true);
		this->_streamingIB->setPersistent(true);
		// --------------------------
	}

	Stencil::~Stencil() {
		this->_drawCalls.clear();

		this->_2dPipeline = nullptr;
		this->_linePipeline = nullptr;
		this->_textPipeline = nullptr;

		this->_streamingIB.reset();
		this->_streamingVB.reset();
	}

	void Stencil::upload() {
		if (this->_2dPipeline != nullptr || this->_linePipeline != nullptr) throw this->_logger->error("Upload already called");

		// PIPELINE ----
		rawrbox::PipeSettings settings;
		settings.depth = Diligent::COMPARISON_FUNC_UNKNOWN;
		settings.cull = Diligent::CULL_MODE_NONE;
		settings.pVS = "stencil.vsh";
		settings.pPS = "stencil.psh";
		settings.scissors = true;
		settings.blending = {Diligent::BLEND_FACTOR_SRC_ALPHA, Diligent::BLEND_FACTOR_INV_SRC_ALPHA};
		settings.layout = rawrbox::PosUVColorVertexData::vLayout();
		settings.signatures = {rawrbox::BindlessManager::signature}; // Use bindless

		this->_2dPipeline = rawrbox::PipelineUtils::createPipeline("Stencil::2D", settings);

		settings.topology = Diligent::PRIMITIVE_TOPOLOGY_LINE_LIST;
		this->_linePipeline = rawrbox::PipelineUtils::createPipeline("Stencil::2DLine", settings);

		settings.topology = Diligent::PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		settings.pPS = "stencil_text.psh";
		this->_textPipeline = rawrbox::PipelineUtils::createPipeline("Stencil::2DText", settings);
		// -------------
	}

	void Stencil::resize(const rawrbox::Vector2u& size) {
		this->_size = size;
	}

	void Stencil::pushVertice(const uint32_t& textureID, rawrbox::Vector2f pos, const rawrbox::Vector4f& uv, const rawrbox::Color& col) {
		this->applyScale(pos);
		this->applyRotation(pos);

		this->_currentDraw.vertices.emplace_back(textureID,
		    rawrbox::Vector2f(pos.x + this->_offset.x, pos.y + this->_offset.y),
		    uv,
		    col);
	}

	void Stencil::pushIndices(std::vector<uint32_t> ind) {
		this->_currentDraw.indices.insert(this->_currentDraw.indices.end(), ind.begin(), ind.end());
	}

	void Stencil::applyRotation(rawrbox::Vector2f& vert) const {
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

	rawrbox::Vector2f Stencil::alignPosition(const rawrbox::Vector2f& pos, const rawrbox::Vector2f& size, rawrbox::Alignment alignX, rawrbox::Alignment alignY) const {
		rawrbox::Vector2f startpos = pos;
		if (alignX != rawrbox::Alignment::Left || alignY != rawrbox::Alignment::Left) {
			switch (alignX) {
				case rawrbox::Alignment::Left:
					break;
				case rawrbox::Alignment::Center:
					startpos.x -= size.x / 2;
					break;
				case rawrbox::Alignment::Right:
					startpos.x -= size.x;
					break;
			}

			switch (alignY) {
				case rawrbox::Alignment::Left:
					break;
				case rawrbox::Alignment::Center:
					startpos.y -= size.y / 2;
					break;
				case rawrbox::Alignment::Right:
					startpos.y -= size.y;
					break;
			}
		}

		return startpos.round();
	}
	// --------------------

	// ------UTILS
	void Stencil::drawPolygon(const rawrbox::Polygon& poly) {
		uint32_t textureID = rawrbox::WHITE_TEXTURE->getTextureID();

		// Setup --------
		this->setupDrawCall(this->_2dPipeline);
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
			for (const auto& v : poly.verts)
				this->pushVertice(textureID, v.pos, v.uv, v.col);

			this->_currentDraw.indices.insert(this->_currentDraw.indices.end(), poly.indices.begin(), poly.indices.end());
		}

		// Add to calls
		this->pushDrawCall();
		// ----
	}

	void Stencil::drawTriangle(const rawrbox::Vector2f& a, const rawrbox::Vector2f& aUV, const rawrbox::Color& colA, const rawrbox::Vector2f& b, const rawrbox::Vector2f& bUV, const rawrbox::Color& colB, const rawrbox::Vector2f& c, const rawrbox::Vector2f& cUV, const rawrbox::Color& colC) {
		if (colA.invisible() && colB.invisible() && colC.invisible()) return;
		uint32_t textureID = rawrbox::WHITE_TEXTURE->getTextureID();

		// Setup --------
		this->setupDrawCall(this->_2dPipeline);
		// ----

		if (this->_outline.isSet()) {
			float thick = this->_outline.thickness;

			this->drawLine({a.x, a.y - thick}, {b.x, b.y + thick / 2.F}, colA);
			this->drawLine(b, c, colB);
			this->drawLine({c.x + thick / 2.F, c.y}, a, colC);
		} else {
			this->pushVertice(textureID, a, aUV, colA);
			this->pushVertice(textureID, b, bUV, colB);
			this->pushVertice(textureID, c, cUV, colC);

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
			this->drawTexture(pos, size, *rawrbox::WHITE_TEXTURE, col);
		}
	}

	void Stencil::drawTexture(const rawrbox::Vector2f& pos, const rawrbox::Vector2f& size, const rawrbox::TextureBase& tex, const rawrbox::Color& col, const rawrbox::Vector2f& uvStart, const rawrbox::Vector2f& uvEnd, int slice) {
		if (col.invisible()) return;

		// Setup --------
		this->setupDrawCall(this->_2dPipeline);
		// ----

		auto textureID = tex.getTextureID();
		auto a = slice == -1 ? tex.getSlice() : static_cast<float>(slice);

		this->pushVertice(textureID, {pos.x, pos.y}, {uvStart.x, uvStart.y, a}, col);
		this->pushVertice(textureID, {pos.x, pos.y + size.y}, {uvStart.x, uvEnd.y, a}, col);
		this->pushVertice(textureID, {pos.x + size.x, pos.y}, {uvEnd.x, uvStart.y, a}, col);
		this->pushVertice(textureID, {pos.x + size.x, pos.y + size.y}, {uvEnd.x, uvEnd.y, a}, col);

		this->pushIndices({0, 1, 2,
		    1, 3, 2});

		// Add to calls
		this->pushDrawCall();
		// ----
	}

	void Stencil::drawCircle(const rawrbox::Vector2f& pos, const rawrbox::Vector2f& size, const rawrbox::Color& col, size_t roundness, float angleStart, float angleEnd) {
		if (col.invisible()) return;

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
		if (col.invisible()) return;

		rawrbox::StencilOutline outline = {1.F, 0.F}; // Default line size
		if (this->_outline.isSet()) outline = this->_outline;
		if (outline.thickness <= 0.F) return;

		uint32_t textureID = rawrbox::WHITE_TEXTURE->getTextureID();

		bool usePTLines = outline.thickness == 1.F;
		float enableStipple = outline.stipple > 0.F ? 1.F : 0.F;

		// Setup --------
		this->setupDrawCall(usePTLines ? this->_linePipeline : this->_2dPipeline);
		// ----

		if (usePTLines) {
			this->pushVertice(textureID, from, {0, 0, 0, enableStipple}, col);
			this->pushVertice(textureID, to, {outline.stipple, outline.stipple, 0, enableStipple}, col);
			this->pushIndices({0, 1});
		} else {
			float angle = -from.angle(to);
			float uvEnd = outline.stipple <= 0.F ? 1.F : outline.stipple;

			auto vertA = from + rawrbox::Vector2::cosSin(angle) * outline.thickness;
			auto vertB = from + rawrbox::Vector2::cosSin(angle) * -outline.thickness;
			auto vertC = to + rawrbox::Vector2::cosSin(angle) * outline.thickness;
			auto vertD = to + rawrbox::Vector2::cosSin(angle) * -outline.thickness;

			this->pushVertice(textureID, vertA, {0, 0, 0, enableStipple}, col);
			this->pushVertice(textureID, vertB, {0, uvEnd, 0, enableStipple}, col);
			this->pushVertice(textureID, vertC, {uvEnd, 0, 0, enableStipple}, col);
			this->pushVertice(textureID, vertD, {uvEnd, uvEnd, 0, enableStipple}, col);

			this->pushIndices({0, 1, 2,
			    1, 3, 2});
		}

		// Add to calls
		this->pushDrawCall();
		// ----
	}

	void Stencil::drawText(const std::string& text, const rawrbox::Vector2f& pos, const rawrbox::Color& col, const rawrbox::Color& bgCol, rawrbox::Alignment alignX, rawrbox::Alignment alignY) {
		if (col.invisible() || text.empty()) return;
		uint32_t textureID = rawrbox::WHITE_TEXTURE->getTextureID();

		// NOLINTBEGIN(cppcoreguidelines-pro-type-const-cast)
		auto* textCh = const_cast<char*>(text.c_str());
		// NOLINTEND(cppcoreguidelines-pro-type-const-cast)

		auto fontWidth = static_cast<float>(stb_easy_font_width(textCh));
		auto fontHeight = static_cast<float>(stb_easy_font_height(textCh));

		// Draw background ---
		if (!bgCol.invisible()) {
			this->drawBox({pos.x - 2.F, pos.y - 2.F}, {fontWidth + 3.F, fontHeight - 1.F}, bgCol);
		}
		// ---------------

		// Setup position ---
		rawrbox::Vector2f startpos = this->alignPosition(pos, {fontWidth, fontHeight}, alignX, alignY);
		// ------------------

		// Setup --------
		this->setupDrawCall(this->_2dPipeline);
		// ----

		// Generate vertices and indices for the text
		std::vector<char> vertexBuffer = {};
		vertexBuffer.resize(text.length() * 300);

		int num_quads = stb_easy_font_print(0, 0, textCh, nullptr, vertexBuffer.data(), static_cast<int>(vertexBuffer.size()));

		auto* data = std::bit_cast<float*>(vertexBuffer.data());
		std::vector<uint32_t> indices = {};

		for (int quad = 0, stride = 0; quad < num_quads; ++quad, stride += 16) {
			// Push vertices for the current quad
			this->pushVertice(textureID, rawrbox::Vector2f{data[(0 * 4) + stride], data[(0 * 4) + 1 + stride]} + startpos, {}, col);
			this->pushVertice(textureID, rawrbox::Vector2f{data[(1 * 4) + stride], data[(1 * 4) + 1 + stride]} + startpos, {}, col);
			this->pushVertice(textureID, rawrbox::Vector2f{data[(2 * 4) + stride], data[(2 * 4) + 1 + stride]} + startpos, {}, col);
			this->pushVertice(textureID, rawrbox::Vector2f{data[(3 * 4) + stride], data[(3 * 4) + 1 + stride]} + startpos, {}, col);

			// Generate indices for the current quad
			uint32_t base_index = quad * 4; //  4 vertices per quad
			indices.push_back(base_index);
			indices.push_back(base_index + 1);
			indices.push_back(base_index + 2);
			indices.push_back(base_index + 2);
			indices.push_back(base_index + 3);
			indices.push_back(base_index);
		}

		this->pushIndices(indices);

		// Add to calls
		this->pushDrawCall();
		// ----
	}

	void Stencil::drawText(const rawrbox::Font& font, const std::string& text, const rawrbox::Vector2f& pos, const rawrbox::Color& col, rawrbox::Alignment alignX, rawrbox::Alignment alignY) {
		if (col.invisible() || text.empty()) return;

		rawrbox::Vector2f tsize = font.getStringSize(text);
		rawrbox::Vector2f startpos = this->alignPosition(pos, tsize, alignX, alignY);

		font.render(text, startpos, false, [this, &font, col](rawrbox::Glyph* glyph, float x0, float y0, float x1, float y1) {
			uint32_t textureID = font.getPackTexture(glyph)->getTextureID();

			// Setup --------
			this->setupDrawCall(this->_textPipeline);
			// ----

			this->pushVertice(textureID, {x0, y0}, glyph->textureTopLeft, col);
			this->pushVertice(textureID, {x0, y1}, {glyph->textureTopLeft.x, glyph->textureBottomRight.y}, col);
			this->pushVertice(textureID, {x1, y0}, {glyph->textureBottomRight.x, glyph->textureTopLeft.y}, col);
			this->pushVertice(textureID, {x1, y1}, glyph->textureBottomRight, col);

			this->pushIndices({0, 1, 2,
			    1, 3, 2});

			// Add to calls
			this->pushDrawCall();
			// ----
		});
	}

	void Stencil::drawLoading(const rawrbox::Vector2f& pos, const rawrbox::Vector2f& size, const rawrbox::Color& color) {
		// Setup --------
		this->setupDrawCall(this->_2dPipeline);
		// ----

		const auto textureID = rawrbox::CHECKER_TEXTURE->getTextureID();
		uint64_t LOAD_OFFSET = rawrbox::TimeUtils::time();
		if ((LOAD_OFFSET % static_cast<uint32_t>(size.x * size.y)) == 0U) LOAD_OFFSET = 0U;

		float offset = static_cast<float>(LOAD_OFFSET) * 0.0005F;
		this->pushVertice(textureID, {pos.x, pos.y}, {offset, 1.F + offset, 0.F}, color);
		this->pushVertice(textureID, {pos.x, pos.y + size.y}, {offset, offset, 0.F}, color);
		this->pushVertice(textureID, {pos.x + size.x, pos.y}, {1.F + offset, 1.F + offset, 0.F}, color);
		this->pushVertice(textureID, {pos.x + size.x, pos.y + size.y}, {1.F + offset, offset, 0.F}, color);

		this->pushIndices({0, 1, 2,
		    1, 3, 2});

		// Add to calls
		this->pushDrawCall();
		// ----
	}

	void Stencil::drawVertices(const std::vector<rawrbox::PosUVColorVertexData>& vertices, const std::vector<uint32_t>& indices) {
		if (vertices.empty() || indices.empty()) return;

		// Setup --------
		this->setupDrawCall(this->_2dPipeline);
		// ----

		this->_currentDraw.vertices = vertices;
		this->_currentDraw.indices = indices;

		// Add to calls
		this->pushDrawCall();
		// ----
	}
	// --------------------

	// ------RENDERING
	void Stencil::setupDrawCall(Diligent::IPipelineState* program) {
		this->_currentDraw.clear();

		this->_currentDraw.optimize = this->_optimizations.empty() ? true : this->_optimizations.back();
		this->_currentDraw.stencilProgram = program;
		this->_currentDraw.clip = this->_clips.empty() ? rawrbox::Clip{{0, 0, this->_size.x, this->_size.y}} : this->_clips.back();
	}

	void Stencil::pushDrawCall() {
		if (this->_currentDraw.optimize && !this->_drawCalls.empty()) {
			auto& oldCall = this->_drawCalls.back();

			bool canMerge = oldCall.clip == this->_currentDraw.clip &&
					oldCall.cull == this->_currentDraw.cull &&
					oldCall.stencilProgram == this->_currentDraw.stencilProgram &&
					oldCall.vertices.size() + this->_currentDraw.vertices.size() < MaxVertsInStreamingBuffer;

			if (canMerge) {
				for (auto& ind : this->_currentDraw.indices) {
					oldCall.indices.push_back(static_cast<uint32_t>(oldCall.vertices.size()) + ind);
				}

				oldCall.vertices.insert(oldCall.vertices.end(), this->_currentDraw.vertices.begin(), this->_currentDraw.vertices.end());
				return;
			}
		}

		this->_drawCalls.emplace_back(this->_currentDraw);
	}

	void Stencil::internalDraw() {
		if (this->_drawCalls.empty()) return;

		auto* context = rawrbox::RENDERER->context();
		size_t contextID = 0;

		for (auto& group : this->_drawCalls) {
			if (group.stencilProgram == nullptr) continue;
			if (group.vertices.empty() || group.indices.empty()) continue;

			auto vertSize = static_cast<uint32_t>(group.vertices.size());
			auto indSize = static_cast<uint32_t>(group.indices.size());

			// Allocate data -----
			auto VBOffset = static_cast<uint64_t>(this->_streamingVB->allocate(vertSize * sizeof(rawrbox::PosUVColorVertexData), contextID));
			auto IBOffset = static_cast<uint64_t>(this->_streamingIB->allocate(indSize * sizeof(uint32_t), contextID));

			auto* VertexData = std::bit_cast<rawrbox::PosUVColorVertexData*>(std::bit_cast<uint8_t*>(this->_streamingVB->getCPUAddress(contextID)) + VBOffset);
			auto* IndexData = std::bit_cast<uint32_t*>(std::bit_cast<uint8_t*>(this->_streamingIB->getCPUAddress(contextID)) + IBOffset);

			std::memcpy(VertexData, group.vertices.data(), vertSize * sizeof(rawrbox::PosUVColorVertexData));
			std::memcpy(IndexData, group.indices.data(), indSize * sizeof(uint32_t));

			this->_streamingVB->release(contextID);
			this->_streamingIB->release(contextID);
			//  -------------------

			// Render ------------
			auto* vertBuffer = this->_streamingVB->buffer();

			context->SetVertexBuffers(0, 1, &vertBuffer, &VBOffset, Diligent::RESOURCE_STATE_TRANSITION_MODE_VERIFY, Diligent::SET_VERTEX_BUFFERS_FLAG_RESET);
			context->SetIndexBuffer(this->_streamingIB->buffer(), IBOffset, Diligent::RESOURCE_STATE_TRANSITION_MODE_VERIFY);
			context->SetPipelineState(group.stencilProgram);
			// --------------

			// SCISSOR ---
			Diligent::Rect scissor;
			if (group.clip.worldSpace) {
				scissor.left = std::max<int>(group.clip.bbox.pos.x, 0);
				scissor.top = std::max<int>(group.clip.bbox.pos.y, 0);
				scissor.right = std::min<int>(group.clip.bbox.size.x, this->_size.x);
				scissor.bottom = std::min<int>(group.clip.bbox.size.y, this->_size.y);
			} else {
				scissor.left = std::max<int>(group.clip.bbox.left(), 0);
				scissor.top = std::max<int>(group.clip.bbox.top(), 0);
				scissor.right = std::min<int>(group.clip.bbox.right(), this->_size.x);
				scissor.bottom = std::min<int>(group.clip.bbox.bottom(), this->_size.y);
			}

			if (scissor.IsValid()) {
				context->SetScissorRects(1, &scissor, this->_size.x, this->_size.y);
			}
			// -----------

			Diligent::DrawIndexedAttribs DrawAttrs;
			DrawAttrs.IndexType = Diligent::VT_UINT16;
			DrawAttrs.Flags = Diligent::DRAW_FLAG_VERIFY_ALL | Diligent::DRAW_FLAG_DYNAMIC_RESOURCE_BUFFERS_INTACT;
			DrawAttrs.NumIndices = indSize;
			DrawAttrs.BaseVertex = 0;

			context->DrawIndexed(DrawAttrs);
			// -------------------
		}

		this->_streamingIB->flush(contextID);
		this->_streamingVB->flush(contextID);

		this->_drawCalls.clear();
	}

	void Stencil::render() {
		if (!this->_offsets.empty()) throw this->_logger->error("Missing 'popOffset', cannot draw");
		if (!this->_rotations.empty()) throw this->_logger->error("Missing 'popRotation', cannot draw");
		if (!this->_outlines.empty()) throw this->_logger->error("Missing 'popOutline', cannot draw");
		if (!this->_clips.empty()) throw this->_logger->error("Missing 'popClipping', cannot draw");
		if (!this->_scales.empty()) throw this->_logger->error("Missing 'popScale', cannot draw");
		if (!this->_optimizations.empty()) throw this->_logger->error("Missing 'popOptimize', cannot draw");

		this->internalDraw();
	}

	// --------------------

	// ------ LOCATION
	void Stencil::pushOffset(const rawrbox::Vector2f& offset) {
		this->_offsets.push_back(offset);
		this->_offset += offset;
	}

	void Stencil::popOffset() {
		if (this->_offsets.empty()) throw this->_logger->error("Offset is empty, failed to pop");

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
	void Stencil::pushRotation(const rawrbox::StencilRotation& rot) {
		this->_rotations.push_back(rot);
		this->_rotation += rot;
	}

	void Stencil::popRotation() {
		if (this->_rotations.empty()) throw this->_logger->error("Rotations is empty, failed to pop");

		this->_rotation -= this->_rotations.back();
		this->_rotations.pop_back();
	}
	// --------------------

	// ------ OUTLINE
	void Stencil::pushOutline(const rawrbox::StencilOutline& outline) {
		this->_outlines.push_back(outline);
		this->_outline += outline;
	}

	void Stencil::popOutline() {
		if (this->_outlines.empty()) throw this->_logger->error("Outline is empty, failed to pop");

		this->_outline -= this->_outlines.back();
		this->_outlines.pop_back();
	}
	// --------------------

	// ------ CLIPPING
	void Stencil::pushClipping(const rawrbox::Clip& clip) {
		rawrbox::Clip fixedClip = clip;
		fixedClip.bbox.pos = clip.bbox.pos + this->_offset.cast<uint32_t>();

		this->_clips.emplace_back(fixedClip);
	}

	void Stencil::popClipping() {
		if (this->_clips.empty()) throw this->_logger->error("Clips is empty, failed to pop");
		this->_clips.pop_back();
	}
	// --------------------

	// ------ SCALE
	void Stencil::pushScale(const rawrbox::Vector2f& scale) {
		this->_scales.push_back(scale);
		this->_scale += scale;
	}

	void Stencil::popScale() {
		if (this->_scales.empty()) throw this->_logger->error("Scale is empty, failed to pop");

		this->_scale -= this->_scales.back();
		this->_scales.pop_back();
	}
	// --------------------

	// ------ OPTIMIZATION
	void Stencil::pushOptimize(bool optimize) {
		this->_optimizations.push_back(optimize);
	}

	void Stencil::popOptimize() {
		if (this->_optimizations.empty()) throw this->_logger->error("Optimize is empty, failed to pop");
		this->_optimizations.pop_back();
	}
	// -------------------

	// ------ OTHER
	const rawrbox::Vector2u& Stencil::getSize() const { return this->_size; }
	std::vector<rawrbox::StencilDraw> Stencil::getDrawCalls() const { return this->_drawCalls; }

	void Stencil::clear() { this->_drawCalls.clear(); }
	// --------------------
} // namespace rawrbox
