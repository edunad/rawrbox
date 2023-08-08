#include <rawrbox/render/scripting/wrapper/stencil_wrapper.hpp>

namespace rawrbox {
	StencilWrapper::StencilWrapper(rawrbox::Stencil* stencil) : _stencil(stencil) {}

	void StencilWrapper::drawBox(const rawrbox::Vector2f& pos, const rawrbox::Vector2f& size, const rawrbox::Colori& col) {
		if (this->_stencil == nullptr) return;
		this->_stencil->drawBox(pos, size, col.cast<float>());
	}

	void StencilWrapper::registerLua(sol::state& lua) {
		lua.new_usertype<StencilWrapper>("Stencil",
		    sol::no_constructor,
		    "drawBox", &StencilWrapper::drawBox);
	}
} // namespace rawrbox
