#pragma once

#include <rawrbox/math/vector2.hpp>
#include <rawrbox/math/vector3.hpp>
#include <rawrbox/render/post_process/base.hpp>
#include <rawrbox/render/texture/render.hpp>

#include <Graphics\GraphicsEngine\interface\Framebuffer.h>
#include <Graphics\GraphicsEngine\interface\RenderPass.h>

#include <memory>
#include <unordered_map>

namespace rawrbox {

	class PostProcessManager {
	protected:
		std::vector<std::unique_ptr<rawrbox::PostProcessBase>> _postProcesses = {};
		Diligent::RefCntAutoPtr<Diligent::IRenderPass> _renderPass;

		// TEST ----
		// Diligent::RefCntAutoPtr<Diligent::ITexture> _aaa;
		// Diligent::ITextureView* _pDstRenderTarget = nullptr;
		// Diligent::RefCntAutoPtr<Diligent::IFramebuffer> _frameBuffer;

		std::unordered_map<Diligent::ITextureView*, Diligent::RefCntAutoPtr<Diligent::IFramebuffer>> m_FramebufferCache;
		Diligent::RefCntAutoPtr<Diligent::ITexture> pOpenGLOffsreenColorBuffer;
		Diligent::RefCntAutoPtr<Diligent::ITexture> pColorBuffer;
		// --------------

		void buildRenderPass();
		Diligent::RefCntAutoPtr<Diligent::IFramebuffer> createFramebuffer(Diligent::ITextureView* renderTarget);
		Diligent::IFramebuffer* getCurrentFramebuffer();

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
