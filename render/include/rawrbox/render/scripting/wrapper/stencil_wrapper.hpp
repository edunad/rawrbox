#pragma once

#include <rawrbox/render/stencil.hpp>

#include <sol/sol.hpp>

namespace rawrbox {
	class StencilWrapper {
	protected:
		rawrbox::Stencil* _stencil = nullptr;

	public:
		StencilWrapper(rawrbox::Stencil* stencil);
		StencilWrapper(const StencilWrapper&) = default;
		StencilWrapper(StencilWrapper&&) = default;
		StencilWrapper& operator=(const StencilWrapper&) = default;
		StencilWrapper& operator=(StencilWrapper&&) = default;
		virtual ~StencilWrapper() = default;

		virtual void drawBox(const rawrbox::Vector2f& pos, const rawrbox::Vector2f& size, const rawrbox::Colori& col = rawrbox::Colorsi::White());

		static void registerLua(sol::state& lua);
	};
} // namespace rawrbox
