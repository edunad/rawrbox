
#include <rawrbox/math/matrix4x4.hpp>
#include <rawrbox/math/utils/math.hpp>
#include <rawrbox/math/vector4.hpp>
#include <rawrbox/render/renderers/base.hpp>
#include <rawrbox/render/static.hpp>
#include <rawrbox/render/stencil.hpp>

// #include <rawrbox/render_temp/utils/render.hpp>

#include <fmt/format.h>
#include <utf8.h>

namespace rawrbox {
	Stencil::Stencil(const rawrbox::Vector2i& size) : _windowSize(size) {
		this->_streamingVB = std::make_unique<rawrbox::StreamingBuffer>("RawrBox::Stencil::VertexBuffer", Diligent::BIND_VERTEX_BUFFER, MaxVertsInStreamingBuffer * static_cast<uint32_t>(sizeof(rawrbox::PosUVColorVertexData)), 1);
		this->_streamingIB = std::make_unique<rawrbox::StreamingBuffer>("RawrBox::Stencil::IndexBuffer", Diligent::BIND_INDEX_BUFFER, MaxVertsInStreamingBuffer * 3 * static_cast<uint32_t>(sizeof(uint32_t)), 1);
	}

	Stencil::~Stencil() {
		// CLEANUP BUFFERS & PROGRAMS
		RAWRBOX_DESTROY(this->_2dPipeline);
		RAWRBOX_DESTROY(this->_linePipeline);
		RAWRBOX_DESTROY(this->_textPipeline);

		this->_drawCalls.clear();
	}

	void Stencil::createPipelines(const std::string& name, const std::string& vsh, const std::string& psh, Diligent::PRIMITIVE_TOPOLOGY topology, Diligent::IPipelineState** pipe) {
		// Create pipe info ----
		Diligent::GraphicsPipelineStateCreateInfo info;
		info.PSODesc.Name = fmt::format("RawrBox::Stencil::{}", name).c_str();
		info.PSODesc.PipelineType = Diligent::PIPELINE_TYPE_GRAPHICS;
		info.GraphicsPipeline.NumRenderTargets = 1;

		auto desc = rawrbox::RENDERER->swapChain->GetDesc();
		info.GraphicsPipeline.RTVFormats[0] = desc.ColorBufferFormat;
		info.GraphicsPipeline.DSVFormat = desc.DepthBufferFormat;

		info.GraphicsPipeline.PrimitiveTopology = topology;
		info.GraphicsPipeline.RasterizerDesc.CullMode = Diligent::CULL_MODE_NONE;
		info.GraphicsPipeline.DepthStencilDesc.DepthEnable = false; // No depth test
		info.GraphicsPipeline.RasterizerDesc.ScissorEnable = true;

		Diligent::BlendStateDesc BlendState;
		BlendState.RenderTargets[0].BlendEnable = true;
		BlendState.RenderTargets[0].SrcBlend = Diligent::BLEND_FACTOR_SRC_ALPHA;
		BlendState.RenderTargets[0].DestBlend = Diligent::BLEND_FACTOR_INV_SRC_ALPHA;
		info.GraphicsPipeline.BlendDesc = BlendState;
		// -----

		// Create the shaders (move this to a loader) ----
		Diligent::ShaderCreateInfo ShaderCI;
		ShaderCI.SourceLanguage = Diligent::SHADER_SOURCE_LANGUAGE_HLSL;
		ShaderCI.Desc.UseCombinedTextureSamplers = true; // (g_Texture + g_Texture_sampler combination)
		ShaderCI.pShaderSourceStreamFactory = rawrbox::SHADER_FACTORY;

		Diligent::RefCntAutoPtr<Diligent::IShader> pVS;
		{
			ShaderCI.Desc.ShaderType = Diligent::SHADER_TYPE_VERTEX;
			ShaderCI.EntryPoint = "main";
			ShaderCI.Desc.Name = fmt::format("RawrBox::Stencil::{}::VS", name).c_str();
			ShaderCI.FilePath = vsh.c_str();

			rawrbox::RENDERER->device->CreateShader(ShaderCI, &pVS);
		}

		// Create a pixel shader
		Diligent::RefCntAutoPtr<Diligent::IShader> pPS;
		{
			ShaderCI.Desc.ShaderType = Diligent::SHADER_TYPE_PIXEL;
			ShaderCI.EntryPoint = "main";
			ShaderCI.Desc.Name = fmt::format("RawrBox::Stencil::{}::PS", name).c_str();
			ShaderCI.FilePath = psh.c_str();

			rawrbox::RENDERER->device->CreateShader(ShaderCI, &pPS);
		}
		// ----------------------

		info.pVS = pVS;
		info.pPS = pPS;

		// Layout
		auto layout = PosUVColorVertexData::vLayout();
		info.GraphicsPipeline.InputLayout.LayoutElements = layout.data();
		info.GraphicsPipeline.InputLayout.NumElements = static_cast<uint32_t>(layout.size());
		// ----

		// Resources ----
		std::array<Diligent::ShaderResourceVariableDesc, 1> vars =
		    {
			Diligent::ShaderResourceVariableDesc{Diligent::SHADER_TYPE_PIXEL, "g_Texture", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC}};

		info.PSODesc.ResourceLayout.Variables = vars.data();
		info.PSODesc.ResourceLayout.NumVariables = static_cast<uint32_t>(vars.size());

		Diligent::SamplerDesc SamLinearClampDesc{
		    Diligent::FILTER_TYPE_POINT, Diligent::FILTER_TYPE_POINT, Diligent::FILTER_TYPE_POINT,
		    Diligent::TEXTURE_ADDRESS_WRAP, Diligent::TEXTURE_ADDRESS_WRAP, Diligent::TEXTURE_ADDRESS_WRAP};

		std::array<Diligent::ImmutableSamplerDesc, 1> samplers =
		    {
			Diligent::ImmutableSamplerDesc{Diligent::SHADER_TYPE_PIXEL, "g_Texture", SamLinearClampDesc}};

		info.PSODesc.ResourceLayout.ImmutableSamplers = samplers.data();
		info.PSODesc.ResourceLayout.NumImmutableSamplers = static_cast<uint32_t>(samplers.size());
		info.PSODesc.ResourceLayout.DefaultVariableType = Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC;
		// ---------------------

		rawrbox::RENDERER->device->CreateGraphicsPipelineState(info, pipe);
		(*pipe)->CreateShaderResourceBinding(&this->_SRB, true);
		// ----------------
	}

	void Stencil::upload() {
		if (this->_2dPipeline != nullptr || this->_linePipeline != nullptr) throw std::runtime_error("[RawrBox-Stencil] Upload already called");

		this->createPipelines("2D", "stencil.vsh", "stencil.psh", Diligent::PRIMITIVE_TOPOLOGY::PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, &this->_2dPipeline);
		this->createPipelines("2DLine", "stencil.vsh", "stencil.psh", Diligent::PRIMITIVE_TOPOLOGY::PRIMITIVE_TOPOLOGY_LINE_LIST, &this->_linePipeline);
	}

	void Stencil::resize(const rawrbox::Vector2i& size) {
		this->_windowSize = size;
	}

	void Stencil::pushVertice(rawrbox::Vector2f pos, const rawrbox::Vector4f& uv, const rawrbox::Color& col) {
		auto wSize = this->_windowSize.cast<float>();

		this->applyScale(pos);
		this->applyRotation(pos);

		this->_currentDraw.vertices.emplace_back(
		    rawrbox::Vector2f(((pos.x + this->_offset.x) / wSize.x * 2 - 1), ((pos.y + this->_offset.y) / wSize.y * 2 - 1) * -1),
		    uv,
		    col);
	}

	void Stencil::pushIndices(std::vector<uint32_t> ind) {
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
		this->setupDrawCall(
		    this->_2dPipeline,
		    rawrbox::WHITE_TEXTURE->getHandle());
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
		this->setupDrawCall(
		    this->_2dPipeline,
		    rawrbox::WHITE_TEXTURE->getHandle());
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

	void Stencil::drawTexture(const rawrbox::Vector2f& pos, const rawrbox::Vector2f& size, Diligent::ITextureView* tex, const rawrbox::Color& col, const rawrbox::Vector2f& uvStart, const rawrbox::Vector2f& uvEnd, uint32_t atlas) {
		if (col.isTransparent()) return;

		// Setup --------
		this->setupDrawCall(
		    this->_2dPipeline,
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
		float enableStipple = outline.stipple > 0.F ? 1.F : 0.F;

		// Setup --------
		this->setupDrawCall(
		    usePTLines ? this->_linePipeline : this->_2dPipeline,
		    rawrbox::WHITE_TEXTURE->getHandle());

		// ----

		if (usePTLines) {
			this->pushVertice(from, {0, 0, 0, enableStipple}, col);
			this->pushVertice(to, {outline.stipple, outline.stipple, 0, enableStipple}, col);
			this->pushIndices({0, 1});
		} else {
			float angle = -from.angle(to);
			float uvEnd = outline.stipple <= 0.F ? 1.F : outline.stipple;

			auto vertA = from + rawrbox::Vector2::cosSin(angle) * outline.thickness;
			auto vertB = from + rawrbox::Vector2::cosSin(angle) * -outline.thickness;
			auto vertC = to + rawrbox::Vector2::cosSin(angle) * outline.thickness;
			auto vertD = to + rawrbox::Vector2::cosSin(angle) * -outline.thickness;

			this->pushVertice(vertA, {0, 0, 0, enableStipple}, col);
			this->pushVertice(vertB, {0, uvEnd, 0, enableStipple}, col);
			this->pushVertice(vertC, {uvEnd, 0, 0, enableStipple}, col);
			this->pushVertice(vertD, {uvEnd, uvEnd, 0, enableStipple}, col);

			this->pushIndices({0, 1, 2,
			    1, 3, 2});
		}

		// Add to calls
		this->pushDrawCall();
		// ----
	}

	/*void Stencil::drawText(const rawrbox::Font& font, const std::string& text, const rawrbox::Vector2f& pos, const rawrbox::Color& col, rawrbox::Alignment alignX, rawrbox::Alignment alignY) {
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
	}*/
	// --------------------

	// ------RENDERING
	void Stencil::setupDrawCall(Diligent::IPipelineState* program, Diligent::ITextureView* texture) {
		this->_currentDraw.clear();

		this->_currentDraw.stencilProgram = program;
		this->_currentDraw.textureHandle = texture;
		this->_currentDraw.clip = this->_clips.empty() ? rawrbox::AABBf(0, 0, static_cast<float>(this->_windowSize.x), static_cast<float>(this->_windowSize.y)) : this->_clips.back();
	}

	void Stencil::pushDrawCall() {
		if (!this->_drawCalls.empty()) {
			auto& oldCall = this->_drawCalls.back();
			bool canMerge = oldCall.clip == this->_currentDraw.clip &&
					oldCall.cull == this->_currentDraw.cull &&
					oldCall.stencilProgram == this->_currentDraw.stencilProgram &&
					oldCall.textureHandle == this->_currentDraw.textureHandle;

			if (canMerge) {
				for (auto& ind : this->_currentDraw.indices) {
					oldCall.indices.push_back(static_cast<uint32_t>(oldCall.vertices.size()) + ind);
				}

				oldCall.vertices.insert(oldCall.vertices.end(), this->_currentDraw.vertices.begin(), this->_currentDraw.vertices.end());
				return;
			}
		}

		this->_drawCalls.push_back(this->_currentDraw);
	}

	void Stencil::internalDraw() {
		if (this->_drawCalls.empty()) return;

		auto context = rawrbox::RENDERER->context;
		size_t contextID = 0;

		for (auto& group : this->_drawCalls) {
			if (group.stencilProgram == nullptr || group.textureHandle == nullptr) continue;
			if (group.vertices.empty() || group.indices.empty()) continue;

			auto vertSize = static_cast<uint32_t>(group.vertices.size());
			auto indSize = static_cast<uint32_t>(group.indices.size());

			// Allocate data -----
			auto VBOffset = this->_streamingVB->allocate(vertSize * sizeof(rawrbox::PosUVColorVertexData), contextID);
			auto IBOffset = this->_streamingIB->allocate(indSize * sizeof(uint32_t), contextID);

			auto* VertexData = std::bit_cast<rawrbox::PosUVColorVertexData*>(std::bit_cast<uint8_t*>(this->_streamingVB->getCPUAddress(contextID)) + VBOffset);
			auto* IndexData = std::bit_cast<uint32_t*>(std::bit_cast<uint8_t*>(this->_streamingIB->getCPUAddress(contextID)) + IBOffset);

			std::memcpy(VertexData, group.vertices.data(), vertSize * sizeof(rawrbox::PosUVColorVertexData));
			std::memcpy(IndexData, group.indices.data(), indSize * sizeof(uint32_t));

			this->_streamingVB->release(contextID);
			this->_streamingIB->release(contextID);
			// -------------------

			// Render ------------
			const std::array<uint64_t, 1> offsets = {VBOffset};
			std::array<Diligent::IBuffer*, 1> pBuffs = {this->_streamingVB->buffer()};

			this->_SRB->GetVariableByName(Diligent::SHADER_TYPE_PIXEL, "g_Texture")->Set(group.textureHandle);

			context->SetVertexBuffers(0, 1, pBuffs.data(), offsets.data(), Diligent::RESOURCE_STATE_TRANSITION_MODE_VERIFY, Diligent::SET_VERTEX_BUFFERS_FLAG_RESET);
			context->SetIndexBuffer(this->_streamingIB->buffer(), IBOffset, Diligent::RESOURCE_STATE_TRANSITION_MODE_VERIFY);

			context->SetPipelineState(group.stencilProgram);
			context->CommitShaderResources(this->_SRB, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

			// SCISSOR ---
			Diligent::Rect scissor;
			scissor.left = group.clip.left();
			scissor.right = group.clip.right();
			scissor.top = group.clip.top();
			scissor.bottom = group.clip.bottom();

			context->SetScissorRects(1, &scissor, 0, 0);
			// -----------

			Diligent::DrawIndexedAttribs DrawAttrs;
			DrawAttrs.IndexType = Diligent::VT_UINT32;
			DrawAttrs.Flags = Diligent::DRAW_FLAG_VERIFY_ALL;
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
		this->_clips.emplace_back(rect.pos.x + this->_offset.x, rect.pos.y + this->_offset.y, rect.size.x, rect.size.y);
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

	// ------ OTHER
	const std::vector<rawrbox::StencilDraw> Stencil::getDrawCalls() const { return this->_drawCalls; }
	void Stencil::clear() { this->_drawCalls.clear(); }
	// --------------------
} // namespace rawrbox
