#include <rawrbox/render/scripting/global/stencil.hpp>
#include <rawrbox/render/static.hpp>

namespace rawrbox {
	// Draw ----
	void StencilGlobal::drawTriangle(const rawrbox::Vector2f& a, const rawrbox::Vector2f& aUV, const rawrbox::Color& colA, const rawrbox::Vector2f& b, const rawrbox::Vector2f& bUV, const rawrbox::Color& colB, const rawrbox::Vector2f& c, const rawrbox::Vector2f& cUV, const rawrbox::Color& colC) {
		if (rawrbox::RENDERER == nullptr) throw std::runtime_error("Invalid stencil handle");
		rawrbox::RENDERER->stencil()->drawTriangle(a, aUV, colA, b, bUV, colB, c, cUV, colC);
	}

	void StencilGlobal::drawBox(const rawrbox::Vector2f& pos, const rawrbox::Vector2f& size, std::optional<rawrbox::Color> col) {
		if (rawrbox::RENDERER == nullptr) throw std::runtime_error("Invalid stencil handle");
		rawrbox::RENDERER->stencil()->drawBox(pos, size, col.value_or(rawrbox::Colors::White()));
	}

	void StencilGlobal::drawTexture(const rawrbox::Vector2f& pos, const rawrbox::Vector2f& size, const rawrbox::TextureBase& tex, const std::optional<rawrbox::Color> col, const std::optional<rawrbox::Vector2f> uvStart, const std::optional<rawrbox::Vector2f> uvEnd) {
		if (rawrbox::RENDERER == nullptr) throw std::runtime_error("Invalid stencil handle");
		rawrbox::RENDERER->stencil()->drawTexture(pos, size, tex, col.value_or(rawrbox::Colors::White()), uvStart.value_or(rawrbox::Vector2f{0, 0}), uvEnd.value_or(rawrbox::Vector2f{1, 1}));
	}

	void StencilGlobal::drawCircle(const rawrbox::Vector2f& pos, const rawrbox::Vector2f& size, std::optional<rawrbox::Color> col, std::optional<size_t> roundness, std::optional<float> angleStart, std::optional<float> angleEnd) {
		if (rawrbox::RENDERER == nullptr) throw std::runtime_error("Invalid stencil handle");
		rawrbox::RENDERER->stencil()->drawCircle(pos, size, col.value_or(rawrbox::Colors::White()), roundness.value_or(32), angleEnd.value_or(0.F), angleStart.value_or(360.F));
	}

	void StencilGlobal::drawLine(const rawrbox::Vector2& from, const rawrbox::Vector2& to, const std::optional<rawrbox::Color> col) {
		if (rawrbox::RENDERER == nullptr) throw std::runtime_error("Invalid stencil handle");
		rawrbox::RENDERER->stencil()->drawLine(from, to, col.value_or(rawrbox::Colors::White()));
	}

	void StencilGlobal::drawText(const rawrbox::Font& font, const std::string& text, const rawrbox::Vector2f& pos, const std::optional<rawrbox::Color>& col, const std::optional<rawrbox::Alignment> alignX, const std::optional<rawrbox::Alignment> alignY) {
		if (rawrbox::RENDERER == nullptr) throw std::runtime_error("Invalid stencil handle");
		rawrbox::RENDERER->stencil()->drawText(font, text, pos, col.value_or(rawrbox::Colors::White()), alignX.value_or(rawrbox::Alignment::Center), alignY.value_or(rawrbox::Alignment::Center));
	}
	// -------

	// Pop & Push ----
	void StencilGlobal::pushOffset(const rawrbox::Vector2f& offset) {
		if (rawrbox::RENDERER == nullptr) throw std::runtime_error("Invalid stencil handle");
		rawrbox::RENDERER->stencil()->pushOffset(offset);
	}

	void StencilGlobal::popOffset() {
		if (rawrbox::RENDERER == nullptr) throw std::runtime_error("Invalid stencil handle");
		rawrbox::RENDERER->stencil()->popOffset();
	}

	void StencilGlobal::pushLocalOffset() {
		if (rawrbox::RENDERER == nullptr) throw std::runtime_error("Invalid stencil handle");
		rawrbox::RENDERER->stencil()->pushLocalOffset();
	}

	void StencilGlobal::popLocalOffset() {
		if (rawrbox::RENDERER == nullptr) throw std::runtime_error("Invalid stencil handle");
		rawrbox::RENDERER->stencil()->popLocalOffset();
	}

	void StencilGlobal::pushRotation(float rotation, const rawrbox::Vector2f& origin) {
		if (rawrbox::RENDERER == nullptr) throw std::runtime_error("Invalid stencil handle");
		rawrbox::RENDERER->stencil()->pushRotation({rotation, origin});
	}

	void StencilGlobal::popRotation() {
		if (rawrbox::RENDERER == nullptr) throw std::runtime_error("Invalid stencil handle");
		rawrbox::RENDERER->stencil()->popRotation();
	}

	void StencilGlobal::pushOutline(float thickness, float stipple) {
		if (rawrbox::RENDERER == nullptr) throw std::runtime_error("Invalid stencil handle");
		rawrbox::RENDERER->stencil()->pushOutline({thickness, stipple});
	}

	void StencilGlobal::popOutline() {
		if (rawrbox::RENDERER == nullptr) throw std::runtime_error("Invalid stencil handle");
		rawrbox::RENDERER->stencil()->popOutline();
	}

	void StencilGlobal::pushClipping(const rawrbox::AABB& rect) {
		if (rawrbox::RENDERER == nullptr) throw std::runtime_error("Invalid stencil handle");
		rawrbox::RENDERER->stencil()->pushClipping(rect.cast<int>());
	}

	void StencilGlobal::popClipping() {
		if (rawrbox::RENDERER == nullptr) throw std::runtime_error("Invalid stencil handle");
		rawrbox::RENDERER->stencil()->popClipping();
	}

	void StencilGlobal::pushScale(const rawrbox::Vector2f& scale) {
		if (rawrbox::RENDERER == nullptr) throw std::runtime_error("Invalid stencil handle");
		rawrbox::RENDERER->stencil()->pushScale(scale);
	}

	void StencilGlobal::popScale() {
		if (rawrbox::RENDERER == nullptr) throw std::runtime_error("Invalid stencil handle");
		rawrbox::RENDERER->stencil()->popScale();
	}
	// ----

	void StencilGlobal::registerLua(lua_State* L) {
		luabridge::getGlobalNamespace(L)
		    .beginNamespace("stencil", {})

		    // Draw
		    .addFunction("drawTriangle", &StencilGlobal::drawTriangle)
		    .addFunction("drawBox", &StencilGlobal::drawBox)
		    .addFunction("drawTexture", &StencilGlobal::drawTexture)
		    .addFunction("drawCircle", &StencilGlobal::drawCircle)
		    .addFunction("drawLine", &StencilGlobal::drawLine)
		    .addFunction("drawText", &StencilGlobal::drawText)
		    // -----

		    // Pop & push
		    .addFunction("pushOffset", &StencilGlobal::pushOffset)
		    .addFunction("popOffset", &StencilGlobal::popOffset)

		    .addFunction("pushLocalOffset", &StencilGlobal::pushLocalOffset)
		    .addFunction("popLocalOffset", &StencilGlobal::popLocalOffset)

		    .addFunction("pushRotation", &StencilGlobal::pushRotation)
		    .addFunction("popRotation", &StencilGlobal::popRotation)

		    .addFunction("pushOutline", &StencilGlobal::pushOutline)
		    .addFunction("popOutline", &StencilGlobal::popOutline)

		    .addFunction("pushClipping", &StencilGlobal::pushClipping)
		    .addFunction("popClipping", &StencilGlobal::popClipping)

		    .addFunction("pushScale", &StencilGlobal::pushScale)
		    .addFunction("popScale", &StencilGlobal::popScale)
		    // -----

		    .endNamespace();
	}
} // namespace rawrbox
