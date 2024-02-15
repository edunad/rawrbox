#include <rawrbox/render/scripting/wrappers/stencil.hpp>
#include <rawrbox/render/static.hpp>

namespace rawrbox {
	// Draw ----
	void StencilWrapper::drawTriangle(const rawrbox::Vector2f& a, const rawrbox::Vector2f& aUV, const rawrbox::Color& colA, const rawrbox::Vector2f& b, const rawrbox::Vector2f& bUV, const rawrbox::Color& colB, const rawrbox::Vector2f& c, const rawrbox::Vector2f& cUV, const rawrbox::Color& colC) {
		if (rawrbox::RENDERER == nullptr) throw std::runtime_error("Invalid stencil handle");
		rawrbox::RENDERER->stencil()->drawTriangle(a, aUV, colA, b, bUV, colB, c, cUV, colC);
	}

	void StencilWrapper::drawBox(const rawrbox::Vector2f& pos, const rawrbox::Vector2f& size, std::optional<rawrbox::Color> col) {
		if (rawrbox::RENDERER == nullptr) throw std::runtime_error("Invalid stencil handle");
		rawrbox::RENDERER->stencil()->drawBox(pos, size, col.value_or(rawrbox::Colors::White()));
	}

	void StencilWrapper::drawTexture(const rawrbox::Vector2f& pos, const rawrbox::Vector2f& size, const rawrbox::TextureBase& tex, const std::optional<rawrbox::Color> col, const std::optional<rawrbox::Vector2f> uvStart, const std::optional<rawrbox::Vector2f> uvEnd) {
		if (rawrbox::RENDERER == nullptr) throw std::runtime_error("Invalid stencil handle");
		rawrbox::RENDERER->stencil()->drawTexture(pos, size, tex, col.value_or(rawrbox::Colors::White()), uvStart.value_or(rawrbox::Vector2f{0, 0}), uvEnd.value_or(rawrbox::Vector2f{1, 1}));
	}

	void StencilWrapper::drawCircle(const rawrbox::Vector2f& pos, const rawrbox::Vector2f& size, std::optional<rawrbox::Color> col, std::optional<size_t> roundness, std::optional<float> angleStart, std::optional<float> angleEnd) {
		if (rawrbox::RENDERER == nullptr) throw std::runtime_error("Invalid stencil handle");
		rawrbox::RENDERER->stencil()->drawCircle(pos, size, col.value_or(rawrbox::Colors::White()), roundness.value_or(32), angleEnd.value_or(0.F), angleStart.value_or(360.F));
	}

	void StencilWrapper::drawLine(const rawrbox::Vector2& from, const rawrbox::Vector2& to, const std::optional<rawrbox::Color> col) {
		if (rawrbox::RENDERER == nullptr) throw std::runtime_error("Invalid stencil handle");
		rawrbox::RENDERER->stencil()->drawLine(from, to, col.value_or(rawrbox::Colors::White()));
	}

	void StencilWrapper::drawText(const rawrbox::Font& font, const std::string& text, const rawrbox::Vector2f& pos, const std::optional<rawrbox::Color>& col, const std::optional<rawrbox::Alignment> alignX, const std::optional<rawrbox::Alignment> alignY) {
		if (rawrbox::RENDERER == nullptr) throw std::runtime_error("Invalid stencil handle");
		rawrbox::RENDERER->stencil()->drawText(font, text, pos, col.value_or(rawrbox::Colors::White()), alignX.value_or(rawrbox::Alignment::Center), alignY.value_or(rawrbox::Alignment::Center));
	}
	// -------

	// Pop & Push ----
	void StencilWrapper::pushOffset(const rawrbox::Vector2f& offset) {
		if (rawrbox::RENDERER == nullptr) throw std::runtime_error("Invalid stencil handle");
		rawrbox::RENDERER->stencil()->pushOffset(offset);
	}

	void StencilWrapper::popOffset() {
		if (rawrbox::RENDERER == nullptr) throw std::runtime_error("Invalid stencil handle");
		rawrbox::RENDERER->stencil()->popOffset();
	}

	void StencilWrapper::pushLocalOffset() {
		if (rawrbox::RENDERER == nullptr) throw std::runtime_error("Invalid stencil handle");
		rawrbox::RENDERER->stencil()->pushLocalOffset();
	}

	void StencilWrapper::popLocalOffset() {
		if (rawrbox::RENDERER == nullptr) throw std::runtime_error("Invalid stencil handle");
		rawrbox::RENDERER->stencil()->popLocalOffset();
	}

	void StencilWrapper::pushRotation(float rotation, const rawrbox::Vector2f& origin) {
		if (rawrbox::RENDERER == nullptr) throw std::runtime_error("Invalid stencil handle");
		rawrbox::RENDERER->stencil()->pushRotation({rotation, origin});
	}

	void StencilWrapper::popRotation() {
		if (rawrbox::RENDERER == nullptr) throw std::runtime_error("Invalid stencil handle");
		rawrbox::RENDERER->stencil()->popRotation();
	}

	void StencilWrapper::pushOutline(float thickness, float stipple) {
		if (rawrbox::RENDERER == nullptr) throw std::runtime_error("Invalid stencil handle");
		rawrbox::RENDERER->stencil()->pushOutline({thickness, stipple});
	}

	void StencilWrapper::popOutline() {
		if (rawrbox::RENDERER == nullptr) throw std::runtime_error("Invalid stencil handle");
		rawrbox::RENDERER->stencil()->popOutline();
	}

	void StencilWrapper::pushClipping(const rawrbox::AABB& rect) {
		if (rawrbox::RENDERER == nullptr) throw std::runtime_error("Invalid stencil handle");
		rawrbox::RENDERER->stencil()->pushClipping(rect.cast<int>());
	}

	void StencilWrapper::popClipping() {
		if (rawrbox::RENDERER == nullptr) throw std::runtime_error("Invalid stencil handle");
		rawrbox::RENDERER->stencil()->popClipping();
	}

	void StencilWrapper::pushScale(const rawrbox::Vector2f& scale) {
		if (rawrbox::RENDERER == nullptr) throw std::runtime_error("Invalid stencil handle");
		rawrbox::RENDERER->stencil()->pushScale(scale);
	}

	void StencilWrapper::popScale() {
		if (rawrbox::RENDERER == nullptr) throw std::runtime_error("Invalid stencil handle");
		rawrbox::RENDERER->stencil()->popScale();
	}
	// ----

	void StencilWrapper::registerLua(lua_State* L) {
		luabridge::getGlobalNamespace(L)
		    .beginNamespace("stencil", {})

		    // Draw
		    .addFunction("drawTriangle", &StencilWrapper::drawTriangle)
		    .addFunction("drawBox", &StencilWrapper::drawBox)
		    .addFunction("drawTexture", &StencilWrapper::drawTexture)
		    .addFunction("drawCircle", &StencilWrapper::drawCircle)
		    .addFunction("drawLine", &StencilWrapper::drawLine)
		    .addFunction("drawText", &StencilWrapper::drawText)
		    // -----

		    // Pop & push
		    .addFunction("pushOffset", &StencilWrapper::pushOffset)
		    .addFunction("popOffset", &StencilWrapper::popOffset)

		    .addFunction("pushLocalOffset", &StencilWrapper::pushLocalOffset)
		    .addFunction("popLocalOffset", &StencilWrapper::popLocalOffset)

		    .addFunction("pushRotation", &StencilWrapper::pushRotation)
		    .addFunction("popRotation", &StencilWrapper::popRotation)

		    .addFunction("pushOutline", &StencilWrapper::pushOutline)
		    .addFunction("popOutline", &StencilWrapper::popOutline)

		    .addFunction("pushClipping", &StencilWrapper::pushClipping)
		    .addFunction("popClipping", &StencilWrapper::popClipping)

		    .addFunction("pushScale", &StencilWrapper::pushScale)
		    .addFunction("popScale", &StencilWrapper::popScale)
		    // -----

		    .endNamespace();
	}
} // namespace rawrbox
