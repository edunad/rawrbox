#pragma once
#include <rawrbox/math/vector2.hpp>
#include <rawrbox/render/texture/render.hpp>

#include <bgfx/bgfx.h>

#include <array>
#include <functional>
#include <string>

namespace rawrbox {
	class RendererBase {
	protected:
		std::unique_ptr<rawrbox::TextureRender> _render = nullptr;
		std::unique_ptr<rawrbox::TextureRender> _decals = nullptr;

		rawrbox::Vector2i _size = {};

		virtual void frame();

	public:
		std::function<void()> worldRender = nullptr;
		std::function<void()> overlayRender = nullptr;

		std::function<void()> postRender = nullptr;

		RendererBase() = default;
		RendererBase(const RendererBase&) = delete;
		RendererBase(RendererBase&&) = delete;
		RendererBase& operator=(const RendererBase&) = delete;
		RendererBase& operator=(RendererBase&&) = delete;
		virtual ~RendererBase();

		virtual void init(const rawrbox::Vector2i& size);
		virtual void resize(const rawrbox::Vector2i& size);

		virtual void setWorldRender(std::function<void()> render);
		virtual void setOverlayRender(std::function<void()> render);
		virtual void overridePostWorld(std::function<void()> post);

		virtual void render();
		virtual void finalRender();
		virtual void bindRenderUniforms();

		// Utils ----
		[[nodiscard]] virtual const bgfx::TextureHandle getDepth() const;
		[[nodiscard]] virtual const bgfx::TextureHandle getColor() const;
		[[nodiscard]] virtual const bgfx::TextureHandle getMask() const;
		// ------

		static bool supported();
	};
} // namespace rawrbox
