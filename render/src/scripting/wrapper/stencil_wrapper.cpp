#include <rawrbox/render/scripting/wrapper/stencil_wrapper.hpp>

namespace rawrbox {
	StencilWrapper::StencilWrapper(rawrbox::Stencil* stencil) : _stencil(stencil) {}

	// Draw ----
	void StencilWrapper::drawTriangle(const rawrbox::Vector2f& a, const rawrbox::Vector2f& aUV, const rawrbox::Colori& colA, const rawrbox::Vector2f& b, const rawrbox::Vector2f& bUV, const rawrbox::Colori& colB, const rawrbox::Vector2f& c, const rawrbox::Vector2f& cUV, const rawrbox::Colori& colC) {
		if (this->_stencil == nullptr) return;
		this->_stencil->drawTriangle(a, aUV, colA.cast<float>(), b, bUV, colB.cast<float>(), c, cUV, colC.cast<float>());
	}

	void StencilWrapper::drawBox(const rawrbox::Vector2f& pos, const rawrbox::Vector2f& size, const rawrbox::Colori& col) {
		if (this->_stencil == nullptr) return;
		this->_stencil->drawBox(pos, size, col.cast<float>());
	}

	void StencilWrapper::drawCircle(const rawrbox::Vector2f& pos, const rawrbox::Vector2f& size, const rawrbox::Colori& col, size_t roundness, float angleStart, float angleEnd) {
		if (this->_stencil == nullptr) return;
		this->_stencil->drawCircle(pos, size, col.cast<float>(), roundness, angleStart, angleEnd);
	}

	void StencilWrapper::drawLine(const rawrbox::Vector2& from, const rawrbox::Vector2& to, const rawrbox::Colori& col) {
		if (this->_stencil == nullptr) return;
		this->_stencil->drawLine(from, to, col.cast<float>());
	}
	// -------

	// Pop & Push ----
	void StencilWrapper::pushOffset(const rawrbox::Vector2f& offset) {
		if (this->_stencil != nullptr) this->_stencil->pushOffset(offset);
	}

	void StencilWrapper::popOffset() {
		if (this->_stencil != nullptr) this->_stencil->popOffset();
	}

	void StencilWrapper::pushLocalOffset() {
		if (this->_stencil != nullptr) this->_stencil->pushLocalOffset();
	}

	void StencilWrapper::popLocalOffset() {
		if (this->_stencil != nullptr) this->_stencil->popLocalOffset();
	}

	void StencilWrapper::pushRotation(float rotation, const rawrbox::Vector2f& origin) {
		if (this->_stencil != nullptr) this->_stencil->pushRotation({rotation, origin});
	}

	void StencilWrapper::popRotation() {
		if (this->_stencil != nullptr) this->_stencil->popRotation();
	}

	void StencilWrapper::pushOutline(float thickness, float stipple) {
		if (this->_stencil != nullptr) this->_stencil->pushOutline({thickness, stipple});
	}

	void StencilWrapper::popOutline() {
		if (this->_stencil != nullptr) this->_stencil->popOutline();
	}

	void StencilWrapper::pushClipping(const rawrbox::AABB& rect) {
		if (this->_stencil != nullptr) this->_stencil->pushClipping(rect);
	}

	void StencilWrapper::popClipping() {
		if (this->_stencil != nullptr) this->_stencil->popClipping();
	}

	void StencilWrapper::pushScale(const rawrbox::Vector2f& scale) {
		if (this->_stencil != nullptr) this->_stencil->pushScale(scale);
	}

	void StencilWrapper::popScale() {
		if (this->_stencil != nullptr) this->_stencil->popScale();
	}

	void StencilWrapper::pushDisableCulling() {
		if (this->_stencil != nullptr) this->_stencil->pushDisableCulling();
	}

	void StencilWrapper::popDisableCulling() {
		if (this->_stencil != nullptr) this->_stencil->popDisableCulling();
	}
	// ----

	void StencilWrapper::registerLua(sol::state& lua) {
		lua.new_usertype<StencilWrapper>("Stencil",
		    sol::no_constructor,

		    // Draw
		    "drawTriangle", &StencilWrapper::drawTriangle,
		    "drawBox", &StencilWrapper::drawBox,
		    "drawCircle", &StencilWrapper::drawCircle,
		    "drawLine", &StencilWrapper::drawLine,

		    // Pop & push
		    "pushOffset", &StencilWrapper::pushOffset,
		    "popOffset", &StencilWrapper::popOffset,
		    "pushLocalOffset", &StencilWrapper::pushLocalOffset,
		    "popLocalOffset", &StencilWrapper::popLocalOffset,
		    "pushRotation", &StencilWrapper::pushRotation,
		    "popRotation", &StencilWrapper::popRotation,
		    "pushOutline", &StencilWrapper::pushOutline,
		    "popOutline", &StencilWrapper::popOutline,
		    "pushClipping", &StencilWrapper::pushClipping,
		    "popClipping", &StencilWrapper::popClipping,
		    "pushScale", &StencilWrapper::pushScale,
		    "popScale", &StencilWrapper::popScale,
		    "pushDisableCulling", &StencilWrapper::pushDisableCulling,
		    "popDisableCulling", &StencilWrapper::popDisableCulling);
	}
} // namespace rawrbox
