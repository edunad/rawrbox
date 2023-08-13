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

		// Draw ----
		// virtual void drawPolygon(rawrbox::Polygon poly);
		virtual void drawTriangle(const rawrbox::Vector2f& a, const rawrbox::Vector2f& aUV, const rawrbox::Colori& colA, const rawrbox::Vector2f& b, const rawrbox::Vector2f& bUV, const rawrbox::Colori& colB, const rawrbox::Vector2f& c, const rawrbox::Vector2f& cUV, const rawrbox::Colori& colC);
		virtual void drawBox(const rawrbox::Vector2f& pos, const rawrbox::Vector2f& size, const sol::optional<rawrbox::Colori> col);
		// virtual void drawTexture(const rawrbox::Vector2f& pos, const rawrbox::Vector2f& size, const rawrbox::TextureBase& tex, const rawrbox::Colori& col = rawrbox::Colorsi::White(), const rawrbox::Vector2f& uvStart = {0, 0}, const rawrbox::Vector2f& uvEnd = {1, 1});
		virtual void drawCircle(const rawrbox::Vector2f& pos, const rawrbox::Vector2f& size, const sol::optional<rawrbox::Colori> col, const sol::optional<size_t> roundness, const sol::optional<float> angleStart, const sol::optional<float> angleEnd);
		virtual void drawLine(const rawrbox::Vector2& from, const rawrbox::Vector2& to, const sol::optional<rawrbox::Colori> col);
		//  ------

		// Pop & Push ----
		virtual void pushOffset(const rawrbox::Vector2f& offset);
		virtual void popOffset();

		virtual void pushLocalOffset();
		virtual void popLocalOffset();

		virtual void pushRotation(float _rotation, const rawrbox::Vector2f& _origin);
		virtual void popRotation();

		virtual void pushOutline(float thickness, float stipple = 0.F);
		virtual void popOutline();

		virtual void pushClipping(const rawrbox::AABB& rect);
		virtual void popClipping();

		virtual void pushScale(const rawrbox::Vector2f& scale);
		virtual void popScale();

		virtual void pushDisableCulling();
		virtual void popDisableCulling();
		// ------

		static void registerLua(sol::state& lua);
	};
} // namespace rawrbox
