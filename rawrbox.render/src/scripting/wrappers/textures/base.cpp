
#include <rawrbox/render/scripting/wrappers/textures/base.hpp>

namespace rawrbox {
	TextureWrapper::TextureWrapper(rawrbox::TextureBase* texture) : _texture(texture) {}
	rawrbox::TextureBase* TextureWrapper::getRef() const { return this->_texture; }

	// UTILS----
	bool TextureWrapper::hasTransparency() const {
		if (this->_texture == nullptr) throw std::runtime_error("Invalid texture handle");
		return this->_texture->hasTransparency();
	}

	rawrbox::Vector2 TextureWrapper::getSize() const {
		if (this->_texture == nullptr) throw std::runtime_error("Invalid texture handle");
		return this->_texture->getSize().cast<float>(); // Lua only has vector2 float
	}

	bool TextureWrapper::isValid() const {
		if (this->_texture == nullptr) return false;
		return this->_texture->isValid();
	}
	// -----

	void TextureWrapper::registerLua(lua_State* L) {
		luabridge::getGlobalNamespace(L)
		    .beginClass<TextureWrapper>("TextureBase")
		    .addConstructor<void(rawrbox::TextureBase*)>()

		    // UTILS----
		    .addFunction("hasTransparency", &TextureWrapper::hasTransparency)
		    .addFunction("getSize", &TextureWrapper::getSize)
		    .addFunction("isValid", &TextureWrapper::isValid)
		    // -----

		    .endClass();
	}
} // namespace rawrbox
