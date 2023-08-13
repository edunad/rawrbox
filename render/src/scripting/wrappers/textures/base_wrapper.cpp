#pragma once

#include <rawrbox/render/scripting/wrappers/textures/base_wrapper.hpp>
namespace rawrbox {
	TextureWrapper::TextureWrapper(rawrbox::TextureBase* texture) : _texture(texture) {}

	// UTILS----
	bool TextureWrapper::hasTransparency() const {
		if (this->_texture == nullptr) return false;
		return this->_texture->hasTransparency();
	}

	const rawrbox::Vector2i TextureWrapper::getSize() const {
		if (this->_texture == nullptr) return {};
		return this->_texture->getSize();
	}

	bool TextureWrapper::valid() const {
		if (this->_texture == nullptr) return false;
		return this->_texture->valid();
	}

	std::array<float, 4> TextureWrapper::getData() const {
		if (this->_texture == nullptr) return {};
		return this->_texture->getData();
	}
	// -----

	void TextureWrapper::registerLua(sol::state& lua) {
		lua.new_usertype<TextureWrapper>("TextureBase",
		    sol::no_constructor,

		    // UTILS----
		    "hasTransparency", &TextureWrapper::hasTransparency,
		    "getSize", &TextureWrapper::getSize,
		    "valid", &TextureWrapper::valid,
		    "getData", &TextureWrapper::getData
		    // -----
		);
	}
} // namespace rawrbox
