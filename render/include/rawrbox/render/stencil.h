#pragma once

#include <rawrbox/render/texture/flat.h>
#include <rawrbox/render/texture/render.h>

#include <rawrbox/math/color.hpp>
#include <rawrbox/math/vector2.hpp>
#include <rawrbox/math/vector3.hpp>
#include <rawrbox/math/aabb.hpp>

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
		PosUVColorVertexData(float _x, float _y, float _z, float _u, float _v, uint32_t _abgr): x(_x), y(_y), z(_z), u(_u), v(_v), abgr(_abgr) { }
	};

	class Stencil {
	private:
		bgfx::ViewId _viewId;
		bgfx::VertexLayout _vLayout;

		bgfx::ProgramHandle _stencilProgram = BGFX_INVALID_HANDLE;
		bgfx::ProgramHandle _2dprogram = BGFX_INVALID_HANDLE;
		bgfx::TextureHandle _textureHandle = BGFX_INVALID_HANDLE;
		bgfx::UniformHandle _texColor = BGFX_INVALID_HANDLE;

		std::shared_ptr<rawrBox::TextureFlat> _pixelTexture;
		std::shared_ptr<rawrBox::TextureRender> _renderTexture;

		rawrBox::Vector2 _windowSize;

		// Offset handling ----
		rawrBox::Vector2 _offset;
		rawrBox::Vector2 _oldOffset;
		std::vector<rawrBox::Vector2> _offsets;
		std::vector<rawrBox::AABB> _clips;
		// ----------

		std::vector<PosUVColorVertexData> _vertices;
		std::vector<uint16_t> _indices;

		bool _recording = false;

		// ------ UTILS
		void pushVertice(const rawrBox::Vector2& pos, const rawrBox::Vector2& uv, const rawrBox::Color& col);
		void pushIndices(uint16_t a, uint16_t b, uint16_t c);
		// --------------------

		// ------ RENDERING
		void internalDraw(const bgfx::ViewId& id);
		void drawRecording();
		// --------------------
	public:
		Stencil(bgfx::ViewId& id, const rawrBox::Vector2& size);
		~Stencil();

		void initialize();
		void resize(const rawrBox::Vector2i& size);

		// ------ UTILS
		void drawTriangle(const rawrBox::Vector2& a, const rawrBox::Vector2& aUV, const rawrBox::Color& colA, const rawrBox::Vector2& b, const rawrBox::Vector2& bUV, const rawrBox::Color& colB, const rawrBox::Vector2& c, const rawrBox::Vector2& cUV, const rawrBox::Color& colC);
		void drawBox(const rawrBox::Vector2& pos, const rawrBox::Vector2& size, rawrBox::Color col = rawrBox::Colors::White);
		void drawTexture(const rawrBox::Vector2& pos, const rawrBox::Vector2& size, std::shared_ptr<rawrBox::TextureBase> tex, rawrBox::Color col = rawrBox::Colors::White, rawrBox::Vector2 uvStart = {0, 0}, rawrBox::Vector2 uvEnd = {1, 1}, float rotation = 0.f, rawrBox::Vector2 origin = {std::nanf(""), std::nanf("")});
		// --------------------

		// ------ RENDERING
		void setTexture(const bgfx::TextureHandle& tex);
		void setShaderProgram(const bgfx::ProgramHandle& handle);

		void begin();
		void end();
		// --------------------

		// ------ LOCATION
		void pushOffset(const rawrBox::Vector2& offset);
		void popOffset();
		void pushLocalOffset();
		void popLocalOffset();
		// --------------------

		// ------ CLIPPING
		void pushClipping(const rawrBox::AABB& rect);
		void popClipping();
		// --------------------
	};
}
