#pragma once

#include <rawrbox/math/aabb.hpp>
#include <rawrbox/math/color.hpp>
#include <rawrbox/math/pi.hpp>
#include <rawrbox/math/vector2.hpp>
#include <rawrbox/math/vector3.hpp>
#include <rawrbox/render/text/font.h>
#include <rawrbox/render/texture/flat.h>
#include <rawrbox/render/texture/render.h>

#include <bgfx/bgfx.h>

#include <memory>

namespace rawrBox {
	struct PosUVColorVertexData {
		float x = 0;
		float y = 0;
		float z = 0;

		float u = 0;
		float v = 0;

		uint32_t abgr = 0xFFFFFFFF;

		PosUVColorVertexData() = default;
		PosUVColorVertexData(const rawrBox::Vector3f& pos, const rawrBox::Vector2f& uv, const rawrBox::Color& cl) : x(pos.x), y(pos.y), z(pos.z), u(uv.x), v(uv.y), abgr(rawrBox::Color::pack(cl)) {}
		PosUVColorVertexData(float _x, float _y, float _z, float _u, float _v, uint32_t _abgr) : x(_x), y(_y), z(_z), u(_u), v(_v), abgr(_abgr) {}
	};

	struct StencilRotation {
		float rotation = 0.f;
		rawrBox::Vector2f origin = {};

		StencilRotation() = default;
		StencilRotation(float _rotation, const rawrBox::Vector2f& _origin) : rotation(_rotation), origin(_origin) {}

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
		float thickness = 0.f;
		float stipple = 0.f;

		StencilOutline() = default;
		StencilOutline(float _thickness, float _stipple = 0.f) : thickness(_thickness), stipple(_stipple) {}

		bool isSet() { return thickness > 0.f || stipple > 0.f; }

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

	enum class TextAlignment {
		Left,
		Center,
		Right
	};

	class Stencil {
	private:
		bgfx::ViewId _viewId;
		bgfx::VertexLayout _vLayout;

		bgfx::ProgramHandle _stencilProgram = BGFX_INVALID_HANDLE;

		bgfx::ProgramHandle _2dprogram = BGFX_INVALID_HANDLE;
		bgfx::ProgramHandle _lineprogram = BGFX_INVALID_HANDLE;
		bgfx::ProgramHandle _textprogram = BGFX_INVALID_HANDLE;

		bgfx::TextureHandle _textureHandle = BGFX_INVALID_HANDLE;
		bgfx::UniformHandle _texColor = BGFX_INVALID_HANDLE;

		std::shared_ptr<rawrBox::TextureFlat> _pixelTexture;
		std::shared_ptr<rawrBox::TextureRender> _renderTexture;
		uint64_t _drawMode = 0;

		rawrBox::Vector2i _windowSize;

		// Offset handling ----
		rawrBox::Vector2f _offset;
		rawrBox::Vector2f _oldOffset;
		std::vector<rawrBox::Vector2f> _offsets;
		// ----------

		// Clip handling ----
		std::vector<rawrBox::AABB> _clips;
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
		std::vector<rawrBox::Vector2f> _scales;
		rawrBox::Vector2f _scale;
		// ----------

		std::vector<PosUVColorVertexData> _vertices;
		std::vector<uint16_t> _indices;

		bool _recording = false;

		// ------ UTILS
		void pushVertice(rawrBox::Vector2f pos, const rawrBox::Vector2f& uv, const rawrBox::Color& col);
		void pushIndices(uint16_t a, uint16_t b, uint16_t c);

		void applyRotation(rawrBox::Vector2f& vert);
		void applyScale(rawrBox::Vector2f& vert);
		// --------------------

		// ------ RENDERING
		void internalDraw();
		void drawRecording();
		// --------------------
	public:
		Stencil(bgfx::ViewId id, const rawrBox::Vector2i& size);
		~Stencil();

		void initialize();
		void resize(const rawrBox::Vector2i& size);

		// ------ UTILS
		void drawTriangle(const rawrBox::Vector2f& a, const rawrBox::Vector2f& aUV, const rawrBox::Color& colA, const rawrBox::Vector2f& b, const rawrBox::Vector2f& bUV, const rawrBox::Color& colB, const rawrBox::Vector2f& c, const rawrBox::Vector2f& cUV, const rawrBox::Color& colC);
		void drawBox(const rawrBox::Vector2f& pos, const rawrBox::Vector2f& size, const rawrBox::Color& col = rawrBox::Colors::White);
		void drawTexture(const rawrBox::Vector2f& pos, const rawrBox::Vector2f& size, std::shared_ptr<rawrBox::TextureBase> tex, const rawrBox::Color& col = rawrBox::Colors::White, const rawrBox::Vector2f& uvStart = {0, 0}, const rawrBox::Vector2f& uvEnd = {1, 1});
		void drawCircle(const rawrBox::Vector2f& pos, const rawrBox::Vector2f& size, const rawrBox::Color& col = rawrBox::Colors::White, size_t roundness = 32, float angleStart = 0.f, float angleEnd = 360.f);
		void drawLine(const rawrBox::Vector2& from, const rawrBox::Vector2& to, const rawrBox::Color& col = rawrBox::Colors::White);
		void drawText(rawrBox::Font* font, const std::string& text, const rawrBox::Vector2f& pos, const rawrBox::Color& col = rawrBox::Colors::White, rawrBox::TextAlignment alignX = rawrBox::TextAlignment::Left, rawrBox::TextAlignment alignY = rawrBox::TextAlignment::Left);
		// --------------------

		// ------ RENDERING
		void setTexture(const bgfx::TextureHandle& tex);
		void setShaderProgram(const bgfx::ProgramHandle& handle);
		void setDrawMode(uint64_t mode = 0);

		void begin();
		void end();
		// --------------------

		// ------ LOCATION
		void pushOffset(const rawrBox::Vector2f& offset);
		void popOffset();
		void pushLocalOffset();
		void popLocalOffset();
		// --------------------

		// ------ ROTATION
		void pushRotation(const StencilRotation& rot);
		void popRotation();
		// --------------------

		// ------ OUTLINE
		void pushOutline(const StencilOutline& outline);
		void popOutline();
		// --------------------

		// ------ CLIPPING
		void pushClipping(const rawrBox::AABB& rect);
		void popClipping();
		// --------------------

		// ------ SCALE
		void pushScale(const rawrBox::Vector2f& scale);
		void popScale();
		// --------------------
	};
} // namespace rawrBox
