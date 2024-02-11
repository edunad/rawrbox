#pragma once

#include <rawrbox/render/scripting/wrappers/text/font.hpp>
#include <rawrbox/render/scripting/wrappers/textures/base.hpp>
#include <rawrbox/render/stencil.hpp>
#include <rawrbox/scripting/utils/lua.hpp>

namespace rawrbox {
	class StencilWrapper {

	public:
		// Draw ----
		// virtual void drawPolygon(rawrbox::Polygon poly);
		static void drawTriangle(const rawrbox::Vector2f& a, const rawrbox::Vector2f& aUV, const rawrbox::Colori& colA, const rawrbox::Vector2f& b, const rawrbox::Vector2f& bUV, const rawrbox::Colori& colB, const rawrbox::Vector2f& c, const rawrbox::Vector2f& cUV, const rawrbox::Colori& colC);
		static void drawBox(const rawrbox::Vector2f& pos, const rawrbox::Vector2f& size, const std::optional<rawrbox::Colori> col);
		static void drawTexture(const rawrbox::Vector2f& pos, const rawrbox::Vector2f& size, const rawrbox::TextureWrapper& tex, const std::optional<rawrbox::Colori> col, const std::optional<rawrbox::Vector2f> uvStart, const std::optional<rawrbox::Vector2f> uvEnd);
		static void drawCircle(const rawrbox::Vector2f& pos, const rawrbox::Vector2f& size, const std::optional<rawrbox::Colori> col, const std::optional<size_t> roundness, const std::optional<float> angleStart, const std::optional<float> angleEnd);
		static void drawLine(const rawrbox::Vector2& from, const rawrbox::Vector2& to, const std::optional<rawrbox::Colori> col);
		static void drawText(const rawrbox::FontWrapper& font, const std::string& text, const rawrbox::Vector2f& pos, const std::optional<rawrbox::Colori>& col, const std::optional<rawrbox::Alignment> alignX, const std::optional<rawrbox::Alignment> alignY);
		//  ------

		// Pop & Push ----
		static void pushOffset(const rawrbox::Vector2f& offset);
		static void popOffset();

		static void pushLocalOffset();
		static void popLocalOffset();

		static void pushRotation(float _rotation, const rawrbox::Vector2f& _origin);
		static void popRotation();

		static void pushOutline(float thickness, float stipple = 0.F);
		static void popOutline();

		static void pushClipping(const rawrbox::AABB& rect);
		static void popClipping();

		static void pushScale(const rawrbox::Vector2f& scale);
		static void popScale();
		//  ------

		static void registerLua(lua_State* L);
	};
} // namespace rawrbox
