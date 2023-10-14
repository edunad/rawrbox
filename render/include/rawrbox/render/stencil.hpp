#pragma once

#include <rawrbox/math/aabb.hpp>
#include <rawrbox/math/color.hpp>
#include <rawrbox/math/pi.hpp>
#include <rawrbox/math/vector4.hpp>
#include <rawrbox/render/buffer/streaming.hpp>

// #include <rawrbox/render_temp/static.hpp>
// #include <rawrbox/render_temp/text/font.hpp>
// #include <rawrbox/render_temp/texture/flat.hpp>
// #include <rawrbox/render_temp/texture/render.hpp>

// #include <bgfx/bgfx.h>

#include <Common/interface/RefCntAutoPtr.hpp>

#include <Graphics/GraphicsEngine/interface/PipelineState.h>
#include <Graphics/GraphicsEngine/interface/Texture.h>

#include <memory>
#include <vector>

namespace rawrbox {
	struct PosUVColorVertexData {
		rawrbox::Vector2f pos = {};
		rawrbox::Vector4f uv = {};
		rawrbox::Colorf color = {};

		PosUVColorVertexData() = default;
		PosUVColorVertexData(const rawrbox::Vector2f& _pos, const rawrbox::Vector4f& _uv, const rawrbox::Color& _cl) : pos(_pos), uv(_uv), color(_cl) {}

		static std::array<Diligent::LayoutElement, 3> vLayout() {
			return {
			    // Attribute 0 - Position
			    Diligent::LayoutElement{0, 0, 2, Diligent::VT_FLOAT32, false, Diligent::LAYOUT_ELEMENT_AUTO_OFFSET, Diligent::LAYOUT_ELEMENT_AUTO_STRIDE, Diligent::INPUT_ELEMENT_FREQUENCY_PER_VERTEX},
			    // Attribute 1 - UV
			    Diligent::LayoutElement{1, 0, 4, Diligent::VT_FLOAT32, false, Diligent::LAYOUT_ELEMENT_AUTO_OFFSET, Diligent::LAYOUT_ELEMENT_AUTO_STRIDE, Diligent::INPUT_ELEMENT_FREQUENCY_PER_VERTEX},
			    // Attribute 2 - Color
			    Diligent::LayoutElement{2, 0, 4, Diligent::VT_FLOAT32, false, Diligent::LAYOUT_ELEMENT_AUTO_OFFSET, Diligent::LAYOUT_ELEMENT_AUTO_STRIDE, Diligent::INPUT_ELEMENT_FREQUENCY_PER_VERTEX}};
		}
	};

	struct PolygonVertice {
		rawrbox::Vector2f pos = {};
		rawrbox::Vector2f uv = {};
		rawrbox::Color col = rawrbox::Colors::White();
	};

	struct Polygon {
		std::vector<rawrbox::PolygonVertice> verts = {};
		std::vector<unsigned int> indices = {};
	};

	struct StencilDraw {
		Diligent::IPipelineState* stencilProgram = nullptr;
		Diligent::ITexture* textureHandle = nullptr;

		std::vector<rawrbox::PosUVColorVertexData> vertices = {};
		std::vector<uint32_t> indices = {};

		rawrbox::AABB clip = {};
		bool cull = true;

		void clear() {
			this->cull = true;
			this->clip = {}; // NONE

			this->stencilProgram = nullptr;
			this->textureHandle = nullptr;

			this->indices.clear();
			this->vertices.clear();
		}

		StencilDraw() = default;
	};

	struct StencilRotation {
		float rotation = 0.F;
		rawrbox::Vector2f origin = {};

		StencilRotation() = default;
		StencilRotation(float _rotation, const rawrbox::Vector2f& _origin) : rotation(_rotation), origin(_origin) {}

		StencilRotation operator-(const StencilRotation& other) const {
			return {this->rotation - other.rotation, this->origin - other.origin};
		}

		StencilRotation& operator-=(const StencilRotation& other) {
			this->rotation -= other.rotation;
			this->origin -= other.origin;

			return *this;
		}

		StencilRotation operator+(const StencilRotation& other) const {
			return {this->rotation + other.rotation, this->origin + other.origin};
		}

		StencilRotation& operator+=(const StencilRotation& other) {
			this->rotation += other.rotation;
			this->origin += other.origin;

			return *this;
		}
	};

	struct StencilOutline {
		float thickness = 0.F;
		float stipple = 0.F;

		StencilOutline() = default;

		// NOLINTBEGIN(hicpp-explicit-conversions)
		StencilOutline(float _thickness) : thickness(_thickness) {}
		// NOLINTEND(hicpp-explicit-conversions)
		StencilOutline(float _thickness, float _stipple) : thickness(_thickness), stipple(_stipple) {}

		bool isSet() { return thickness > 0.F || stipple > 0.F; }

		StencilOutline operator-(const StencilOutline& other) const {
			return {this->thickness - other.thickness, this->stipple - other.stipple};
		}

		StencilOutline& operator-=(const StencilOutline& other) {
			this->thickness -= other.thickness;
			this->stipple -= other.stipple;

			return *this;
		}

		StencilOutline operator+(const StencilOutline& other) const {
			return {this->thickness + other.thickness, this->stipple + other.stipple};
		}

		StencilOutline& operator+=(const StencilOutline& other) {
			this->thickness += other.thickness;
			this->stipple += other.stipple;

			return *this;
		}
	};

	class Stencil {
		// static bgfx::ViewId renderID;

	private:
		// bgfx::VertexLayout _vLayout;
		//
		// bgfx::ProgramHandle _2dprogram = BGFX_INVALID_HANDLE;
		// bgfx::ProgramHandle _lineprogram = BGFX_INVALID_HANDLE;
		// bgfx::ProgramHandle _textprogram = BGFX_INVALID_HANDLE;
		//
		// bgfx::UniformHandle _texColor = BGFX_INVALID_HANDLE;

		// HANDLES ----
		Diligent::RefCntAutoPtr<Diligent::IPipelineState> _2dPipeline;
		Diligent::RefCntAutoPtr<Diligent::IPipelineState> _linePipeline;
		Diligent::RefCntAutoPtr<Diligent::IPipelineState> _textPipeline;

		static constexpr const int MaxVertsInStreamingBuffer = 4096;
		std::unique_ptr<rawrbox::StreamingBuffer> _streamingVB = nullptr;
		std::unique_ptr<rawrbox::StreamingBuffer> _streamingIB = nullptr;
		// ------------

		// WINDOW ----
		rawrbox::Vector2i _windowSize = {};
		// bgfx::ViewId _prevViewId = 0;
		// bgfx::ViewId _renderId = 0;
		//  ----------

		// Offset handling ----
		rawrbox::Vector2f _offset = {};
		rawrbox::Vector2f _oldOffset = {};
		std::vector<rawrbox::Vector2f> _offsets = {};
		// ----------

		// Clip handling ----
		std::vector<rawrbox::AABB> _clips = {};
		// ----------

		// Outline handling ----
		std::vector<rawrbox::StencilOutline> _outlines = {}; // thickness, stipple
		StencilOutline _outline = {};
		// ----------

		// Rotation handling ----
		std::vector<rawrbox::StencilRotation> _rotations = {};
		StencilRotation _rotation = {};
		// ----------

		// Scale handling ----
		std::vector<rawrbox::Vector2f> _scales = {};
		rawrbox::Vector2f _scale = {};
		// ----------

		// Drawing -----
		rawrbox::StencilDraw _currentDraw = {};
		std::vector<rawrbox::StencilDraw> _drawCalls = {};
		// ----------

		// ------ UTILS
		void pushVertice(rawrbox::Vector2f pos, const rawrbox::Vector4f& uv, const rawrbox::Color& col);
		void pushIndices(std::vector<uint32_t> ind);

		void applyRotation(rawrbox::Vector2f& vert);
		void applyScale(rawrbox::Vector2f& vert);
		// --------------------

		// ------ RENDERING
		void setupDrawCall(Diligent::IPipelineState* program, Diligent::ITexture* texture = nullptr);
		void pushDrawCall();
		void internalDraw();
		// --------------------

		void createPipelines(const std::string& name, const std::string& vsh, const std::string& psh, Diligent::PRIMITIVE_TOPOLOGY topology, Diligent::IPipelineState** pipe);

	public:
		Stencil() = default;
		explicit Stencil(const rawrbox::Vector2i& size);

		Stencil(Stencil&&) = delete;
		Stencil& operator=(Stencil&&) = delete;
		Stencil(const Stencil&) = delete;
		Stencil& operator=(const Stencil&) = delete;

		virtual ~Stencil();

		virtual void upload();
		virtual void resize(const rawrbox::Vector2i& size);

		// ------ UTILS
		virtual void drawPolygon(const rawrbox::Polygon& poly);
		virtual void drawTriangle(const rawrbox::Vector2f& a, const rawrbox::Vector2f& aUV, const rawrbox::Color& colA, const rawrbox::Vector2f& b, const rawrbox::Vector2f& bUV, const rawrbox::Color& colB, const rawrbox::Vector2f& c, const rawrbox::Vector2f& cUV, const rawrbox::Color& colC);
		virtual void drawBox(const rawrbox::Vector2f& pos, const rawrbox::Vector2f& size, const rawrbox::Color& col = rawrbox::Colors::White());
		virtual void drawTexture(const rawrbox::Vector2f& pos, const rawrbox::Vector2f& size, const Diligent::ITexture* tex, const rawrbox::Color& col = rawrbox::Colors::White(), const rawrbox::Vector2f& uvStart = {0, 0}, const rawrbox::Vector2f& uvEnd = {1, 1}, uint32_t atlasId = 0);
		// virtual void drawTexture(const rawrbox::Vector2f& pos, const rawrbox::Vector2f& size, const rawrbox::TextureBase& tex, const rawrbox::Color& col = rawrbox::Colors::White(), const rawrbox::Vector2f& uvStart = {0, 0}, const rawrbox::Vector2f& uvEnd = {1, 1}, uint32_t atlasId = 0);
		virtual void drawCircle(const rawrbox::Vector2f& pos, const rawrbox::Vector2f& size, const rawrbox::Color& col = rawrbox::Colors::White(), size_t roundness = 32, float angleStart = 0.F, float angleEnd = 360.F);
		virtual void drawLine(const rawrbox::Vector2& from, const rawrbox::Vector2& to, const rawrbox::Color& col = rawrbox::Colors::White());
		// virtual void drawText(const rawrbox::Font& font, const std::string& text, const rawrbox::Vector2f& pos, const rawrbox::Color& col = rawrbox::Colors::White(), rawrbox::Alignment alignX = rawrbox::Alignment::Left, rawrbox::Alignment alignY = rawrbox::Alignment::Left);
		//  --------------------

		// ------ RENDERING
		virtual void render();
		// --------------------

		// ------ LOCATION
		virtual void pushOffset(const rawrbox::Vector2f& offset);
		virtual void popOffset();
		virtual void pushLocalOffset();
		virtual void popLocalOffset();
		// --------------------

		// ------ ROTATION
		virtual void pushRotation(const StencilRotation& rot);
		virtual void popRotation();
		// --------------------

		// ------ OUTLINE
		virtual void pushOutline(const StencilOutline& outline);
		virtual void popOutline();
		// --------------------

		// ------ CLIPPING
		virtual void pushClipping(const rawrbox::AABB& rect);
		virtual void popClipping();
		// --------------------

		// ------ SCALE
		virtual void pushScale(const rawrbox::Vector2f& scale);
		virtual void popScale();
		// --------------------

		// ------ OTHER
		[[nodiscard]] virtual const std::vector<rawrbox::StencilDraw> getDrawCalls() const;
		virtual void clear();
		// --------------------
	};
} // namespace rawrbox
