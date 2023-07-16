#pragma once

#include <rawrbox/math/vector2.hpp>
#include <rawrbox/math/vector3.hpp>
#include <rawrbox/render/postprocess/base.hpp>
#include <rawrbox/render/texture/render.hpp>

#include <bgfx/bgfx.h>

#include <memory>

namespace rawrbox {

	class PostProcessManager {
	protected:
		std::vector<std::unique_ptr<rawrbox::PostProcessBase>> _postProcesses = {};
		rawrbox::Vector2i _windowSize = {};

		// POS-PROCESS SAMPLES
		std::vector<bgfx::FrameBufferHandle> _samples = {};
		// ----

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
		virtual void render(const bgfx::TextureHandle& renderTexture);
	};
} // namespace rawrbox
