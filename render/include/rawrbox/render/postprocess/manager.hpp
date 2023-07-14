#pragma once

#include <rawrbox/math/vector2.hpp>
#include <rawrbox/math/vector3.hpp>
#include <rawrbox/render/postprocess/base.hpp>
#include <rawrbox/render/texture/render.hpp>

#include <bgfx/bgfx.h>

#include <memory>

namespace rawrbox {

	struct PosUVVertexData {
		rawrbox::Vector3f pos = {};
		rawrbox::Vector2f uv = {};

		PosUVVertexData() = default;
		PosUVVertexData(const rawrbox::Vector3f& _pos, const rawrbox::Vector2f& _uv) : pos(_pos), uv(_uv) {}

		static bgfx::VertexLayout vLayout() {
			static bgfx::VertexLayout layout;
			layout
			    .begin()
			    .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
			    .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
			    .end();
			return layout;
		}
	};

	class PostProcessManager {
	protected:
		std::unique_ptr<rawrbox::TextureRender> _render = nullptr;
		std::vector<std::unique_ptr<rawrbox::PostProcessBase>> _postProcesses = {};

		rawrbox::Vector2i _windowSize = {};

		bool _recording = false;

		// Drawing ----
		bgfx::UniformHandle _texColor = BGFX_INVALID_HANDLE;
		bgfx::ProgramHandle _program = BGFX_INVALID_HANDLE;
		bgfx::VertexBufferHandle _vbh = BGFX_INVALID_HANDLE; // Vertices
		bgfx::IndexBufferHandle _ibh = BGFX_INVALID_HANDLE;  // Indices
		// -----

		std::vector<rawrbox::PosUVVertexData> _vertices = {};
		std::vector<uint16_t> _indices = {};

		// POS-PROCESS SAMPLES
		std::vector<bgfx::FrameBufferHandle> _samples = {};
		// ----

		void pushVertice(const rawrbox::Vector2f& pos, const rawrbox::Vector2f& uv);
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
		template <class T, typename... CallbackArgs>
		void add(CallbackArgs&&... args) {
			this->_postProcesses.push_back(std::make_unique<T>(std::forward<CallbackArgs>(args)...));
			this->buildPRViews();
		}

		virtual void remove(size_t indx);
		[[nodiscard]] virtual rawrbox::PostProcessBase& get(size_t indx) const;
		virtual size_t count();
		// ---------

		virtual void upload();

		virtual void begin();
		virtual void end();
	};
} // namespace rawrbox
