#pragma once

#include <rawrbox/math/aabb.hpp>
#include <rawrbox/math/color.hpp>
#include <rawrbox/math/pi.hpp>
#include <rawrbox/math/vector2.hpp>
#include <rawrbox/math/vector3.hpp>
#include <rawrbox/render/static.hpp>
#include <rawrbox/render/text/font.hpp>
#include <rawrbox/render/texture/flat.hpp>
#include <rawrbox/render/texture/render.hpp>

#include <bgfx/bgfx.h>

#include <memory>
#include <queue>

namespace rawrbox {
	struct PosUVColorVertexData {
		float x = 0;
		float y = 0;
		float z = 0;

		float u = 0;
		float v = 0;

		uint32_t abgr = 0xFFFFFFFF;

		PosUVColorVertexData() = default;
		PosUVColorVertexData(const rawrbox::Vector3f& pos, const rawrbox::Vector2f& uv, const rawrbox::Color& cl) : x(pos.x), y(pos.y), z(pos.z), u(uv.x), v(uv.y), abgr(cl.pack()) {}
		PosUVColorVertexData(float _x, float _y, float _z, float _u, float _v, uint32_t _abgr) : x(_x), y(_y), z(_z), u(_u), v(_v), abgr(_abgr) {}
	};

	struct PolygonVertice {
		rawrbox::Vector2f pos = {};
		rawrbox::Vector2f uv = {};
		rawrbox::Color col = rawrbox::Colors::White;
	};

	struct Polygon {
		std::vector<PolygonVertice> verts = {};
		std::vector<unsigned int> indices = {};
	};

	struct StencilDraw {
		bgfx::ProgramHandle stencilProgram = BGFX_INVALID_HANDLE;
		bgfx::TextureHandle textureHandle = BGFX_INVALID_HANDLE;

		std::vector<PosUVColorVertexData> vertices = {};
		std::vector<uint16_t> indices = {};

		uint64_t drawMode = 0;
		uint16_t clip = UINT16_MAX;
		bool cull = true;

		void clear() {
			this->cull = true;

			this->drawMode = 0;      // Triangle
			this->clip = UINT16_MAX; // NONE

			this->stencilProgram = BGFX_INVALID_HANDLE;
			this->textureHandle = BGFX_INVALID_HANDLE;

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
	private:
		bgfx::VertexLayout _vLayout;

		bgfx::ProgramHandle _2dprogram = BGFX_INVALID_HANDLE;
		bgfx::ProgramHandle _lineprogram = BGFX_INVALID_HANDLE;
		bgfx::ProgramHandle _textprogram = BGFX_INVALID_HANDLE;

		bgfx::UniformHandle _texColor = BGFX_INVALID_HANDLE;

		std::shared_ptr<rawrbox::TextureFlat> _pixelTexture;
		rawrbox::Vector2i _windowSize;

		// Offset handling ----
		rawrbox::Vector2f _offset;
		rawrbox::Vector2f _oldOffset;
		std::vector<rawrbox::Vector2f> _offsets;
		// ----------

		// Clip handling ----
		std::vector<uint32_t> _clips;
		// ----------

		// Outline handling ----
		std::vector<StencilOutline> _outlines; // thickness, stipple
		StencilOutline _outline;
		// ----------

		// Rotation handling ----
		std::vector<StencilRotation> _rotations;
		StencilRotation _rotation;
		// ----------

		// Scale handling ----
		std::vector<rawrbox::Vector2f> _scales;
		rawrbox::Vector2f _scale;
		// ----------

		// Drawing -----
		rawrbox::StencilDraw _currentDraw = {};
		std::vector<rawrbox::StencilDraw> _drawCalls;
		// ----------

		// ------ UTILS
		void pushVertice(rawrbox::Vector2f pos, const rawrbox::Vector2f& uv, const rawrbox::Color& col);
		void pushIndices(std::vector<uint16_t> ind);

		void applyRotation(rawrbox::Vector2f& vert);
		void applyScale(rawrbox::Vector2f& vert);
		// --------------------

		// ------ RENDERING
		void setupDrawCall(const bgfx::ProgramHandle& program, const bgfx::TextureHandle& texture = BGFX_INVALID_HANDLE, uint64_t drawMode = 0);
		void pushDrawCall();
		void internalDraw();
		// --------------------
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
		virtual void drawPolygon(rawrbox::Polygon poly);
		virtual void drawTriangle(const rawrbox::Vector2f& a, const rawrbox::Vector2f& aUV, const rawrbox::Color& colA, const rawrbox::Vector2f& b, const rawrbox::Vector2f& bUV, const rawrbox::Color& colB, const rawrbox::Vector2f& c, const rawrbox::Vector2f& cUV, const rawrbox::Color& colC);
		virtual void drawBox(const rawrbox::Vector2f& pos, const rawrbox::Vector2f& size, const rawrbox::Color& col = rawrbox::Colors::White);
		virtual void drawTexture(const rawrbox::Vector2f& pos, const rawrbox::Vector2f& size, std::shared_ptr<rawrbox::TextureBase> tex, const rawrbox::Color& col = rawrbox::Colors::White, const rawrbox::Vector2f& uvStart = {0, 0}, const rawrbox::Vector2f& uvEnd = {1, 1});
		virtual void drawCircle(const rawrbox::Vector2f& pos, const rawrbox::Vector2f& size, const rawrbox::Color& col = rawrbox::Colors::White, size_t roundness = 32, float angleStart = 0.F, float angleEnd = 360.F);
		virtual void drawLine(const rawrbox::Vector2& from, const rawrbox::Vector2& to, const rawrbox::Color& col = rawrbox::Colors::White);
		virtual void drawText(const rawrbox::Font& font, const std::string& text, const rawrbox::Vector2f& pos, const rawrbox::Color& col = rawrbox::Colors::White, rawrbox::Alignment alignX = rawrbox::Alignment::Left, rawrbox::Alignment alignY = rawrbox::Alignment::Left);
		// --------------------

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
