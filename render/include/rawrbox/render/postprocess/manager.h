#pragma once

#include <rawrbox/math/vector2.hpp>
#include <rawrbox/math/vector3.hpp>
#include <rawrbox/render/postprocess/base.hpp>
#include <rawrbox/render/texture/render.h>

#include <bgfx/bgfx.h>

#include <memory>

namespace rawrBox {
	struct PosUVVertexData {
		float x = 0;
		float y = 0;
		float z = 0;

		float u = 0;
		float v = 0;

		PosUVVertexData() = default;
		PosUVVertexData(const rawrBox::Vector3f& pos, const rawrBox::Vector2f& uv) : x(pos.x), y(pos.y), z(pos.z), u(uv.x), v(uv.y) {}
		PosUVVertexData(float _x, float _y, float _z, float _u, float _v) : x(_x), y(_y), z(_z), u(_u), v(_v) {}
	};

	class PostProcessManager {
		std::shared_ptr<rawrBox::TextureRender> _render;
		std::vector<std::shared_ptr<rawrBox::PostProcessBase>> _postProcesses;

		bgfx::ViewId _view;
		rawrBox::Vector2i _windowSize;

		bool _recording = false;

		// Drawing ----
		bgfx::VertexLayout _vLayout;

		bgfx::VertexBufferHandle _vbh = BGFX_INVALID_HANDLE; // Vertices
		bgfx::IndexBufferHandle _ibh = BGFX_INVALID_HANDLE;  // Indices
		bgfx::UniformHandle _texColor = BGFX_INVALID_HANDLE;
		bgfx::ProgramHandle _program = BGFX_INVALID_HANDLE;
		bgfx::TextureHandle _copyHandle = BGFX_INVALID_HANDLE;
		bgfx::TextureHandle _finalHandle = BGFX_INVALID_HANDLE;

		std::vector<uint8_t> _pixels;
		std::vector<PosUVVertexData> _vertices;
		std::vector<uint16_t> _indices;
		// -----

		void pushVertice(rawrBox::Vector2f pos, const rawrBox::Vector2f& uv);
		void pushIndices(uint16_t a, uint16_t b, uint16_t c);
		void captureData();

	public:
		PostProcessManager(bgfx::ViewId view, const rawrBox::Vector2i& size);
		~PostProcessManager();

		// Process utils ----
		void registerPostProcess(const std::shared_ptr<rawrBox::PostProcessBase>& post);
		void removePostProcess(size_t indx);
		// ---------

		void upload();

		void begin();
		void end();
	};
} // namespace rawrBox
