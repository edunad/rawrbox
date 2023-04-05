#pragma once

#include <rawrbox/render/texture.h>

#include <rawrbox/math/color.hpp>
#include <rawrbox/math/vector2.hpp>

#include<bgfx/bgfx.h>

namespace rawrBOX {
	struct PosUVColorVertexData {
		float x = 0;
		float y = 0;
		float z = 0;

		int16_t u = 0;
		int16_t v = 0;

		uint32_t abgr = 0xFFFFFFFF;

		PosUVColorVertexData() = default;
		PosUVColorVertexData(float _x, float _y, float _z, int16_t _u, int16_t _v, uint32_t _abgr): x(_x), y(_y), z(_z), u(_u), v(_v), abgr(_abgr) { }
	};

	class Stencil {
	private:
		bgfx::ViewId _viewId;
		bgfx::VertexLayout _vLayout;

		bgfx::ProgramHandle _stencilProgram = BGFX_INVALID_HANDLE;
		bgfx::ProgramHandle _2dprogram = BGFX_INVALID_HANDLE;

		bgfx::VertexBufferHandle _vbh = BGFX_INVALID_HANDLE;
		bgfx::IndexBufferHandle _ibh = BGFX_INVALID_HANDLE;

		rawrBOX::Vector2 _windowSize;

		std::vector<PosUVColorVertexData> _vertices;
		std::vector<unsigned int> _indices;

#pragma region RENDERING
		void pushVertice(const rawrBOX::Vector2& pos, const rawrBOX::Vector2& uv, const rawrBOX::Color& col);
		void pushIndices(unsigned int a, unsigned int b, unsigned int c);
#pragma endregion

	public:
		Stencil(bgfx::ViewId, const rawrBOX::Vector2& size);
		~Stencil();

		void initialize();

#pragma region UTILS
		void drawTriangle(const rawrBOX::Vector2& a, const rawrBOX::Vector2& aUV, const rawrBOX::Color& colA, const rawrBOX::Vector2& b, const rawrBOX::Vector2& bUV, const rawrBOX::Color& colB, const rawrBOX::Vector2& c, const rawrBOX::Vector2& cUV, const rawrBOX::Color& colC);
		void drawBox(const rawrBOX::Vector2& pos, const rawrBOX::Vector2& size);
		void drawTexture(rawrBOX::Vector2 pos, rawrBOX::Vector2 size, std::shared_ptr<rawrBOX::Texture> tex, rawrBOX::Color col = rawrBOX::Colors::White, rawrBOX::Vector2 uvStart = {0, 0}, rawrBOX::Vector2 uvEnd = {1, 1}, float rotation = 0.f, const rawrBOX::Vector2& origin = {0, 0});
#pragma endregion

#pragma region RENDERING
		void setShaderProgram(bgfx::ProgramHandle handle);
		void draw();
#pragma endregion
	};
}
