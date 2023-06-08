#pragma once

#include <rawrbox/math/vector2.hpp>
#include <rawrbox/math/vector3.hpp>
#include <rawrbox/render/postprocess/base.hpp>
#include <rawrbox/render/texture/render.hpp>

#include <bgfx/bgfx.h>

#include <memory>

namespace rawrbox {
	struct PosUVVertexData {
		float x = 0;
		float y = 0;
		float z = 0;

		float u = 0;
		float v = 0;

		PosUVVertexData() = default;
		PosUVVertexData(const rawrbox::Vector3f& pos, const rawrbox::Vector2f& uv) : x(pos.x), y(pos.y), z(pos.z), u(uv.x), v(uv.y) {}
		PosUVVertexData(float _x, float _y, float _z, float _u, float _v) : x(_x), y(_y), z(_z), u(_u), v(_v) {}
	};

	class PostProcessManager {
	protected:
		std::unique_ptr<rawrbox::TextureRender> _render;
		std::vector<std::unique_ptr<rawrbox::PostProcessBase>> _postProcesses;

		rawrbox::Vector2i _windowSize;

		bool _recording = false;

		// Drawing ----
		bgfx::VertexLayout _vLayout;

		bgfx::VertexBufferHandle _vbh = BGFX_INVALID_HANDLE; // Vertices
		bgfx::IndexBufferHandle _ibh = BGFX_INVALID_HANDLE;  // Indices
		bgfx::UniformHandle _texColor = BGFX_INVALID_HANDLE;
		bgfx::ProgramHandle _program = BGFX_INVALID_HANDLE;

		std::vector<rawrbox::PosUVVertexData> _vertices;
		std::vector<uint16_t> _indices;
		// -----

		// POS-PROCESS SAMPLES
		std::vector<bgfx::FrameBufferHandle> _samples;
		// ----

		void pushVertice(rawrbox::Vector2f pos, const rawrbox::Vector2f& uv);
		void pushIndices(uint16_t a, uint16_t b, uint16_t c);

		void buildPRViews();

	public:
		PostProcessManager() = default;
		explicit PostProcessManager(const rawrbox::Vector2i& size);
		virtual ~PostProcessManager();

		PostProcessManager(PostProcessManager&&) = delete;
		PostProcessManager& operator=(PostProcessManager&&) = delete;
		PostProcessManager(const PostProcessManager&) = delete;
		PostProcessManager& operator=(const PostProcessManager&) = delete;

		// Process utils ----
		virtual void add(std::unique_ptr<rawrbox::PostProcessBase> post);
		virtual void remove(size_t indx);
		[[nodiscard]] virtual rawrbox::PostProcessBase& get(size_t indx) const;
		virtual size_t count();
		// ---------

		virtual void upload();

		virtual void begin();
		virtual void end();
	};
} // namespace rawrbox
