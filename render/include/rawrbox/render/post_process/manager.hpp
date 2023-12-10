#pragma once

#include <rawrbox/math/vector2.hpp>
#include <rawrbox/math/vector3.hpp>
#include <rawrbox/render/post_process/base.hpp>
#include <rawrbox/render/texture/render.hpp>

#include <Framebuffer.h>
#include <RenderPass.h>

#include <memory>
#include <unordered_map>

namespace rawrbox {

	class PostProcessManager {
	protected:
		std::vector<std::unique_ptr<rawrbox::PostProcessBase>> _postProcesses = {};
		std::unique_ptr<rawrbox::TextureRender> _rt = {};

		Diligent::RefCntAutoPtr<Diligent::ITexture> _cpRT;
		Diligent::ITextureView* _cpRTView = nullptr;

	public:
		PostProcessManager() = default;
		virtual ~PostProcessManager();

		PostProcessManager(PostProcessManager&&) = delete;
		PostProcessManager& operator=(PostProcessManager&&) = delete;
		PostProcessManager(const PostProcessManager&) = delete;
		PostProcessManager& operator=(const PostProcessManager&) = delete;

		// Process utils ----
		template <class T, typename... CallbackArgs>
		void add(CallbackArgs&&... args) {
			this->_postProcesses.push_back(std::make_unique<T>(std::forward<CallbackArgs>(args)...));
		}

		virtual void remove(size_t indx);
		[[nodiscard]] virtual rawrbox::PostProcessBase& get(size_t indx) const;
		virtual size_t count();
		// ---------

		virtual void upload();
		virtual void render(Diligent::ITextureView* renderTexture);
	};
} // namespace rawrbox
