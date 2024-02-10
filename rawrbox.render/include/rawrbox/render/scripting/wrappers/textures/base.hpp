#pragma once

#include <rawrbox/render/textures/base.hpp>
#include <rawrbox/scripting/utils/lua.hpp>

namespace rawrbox {
	class TextureWrapper {
	protected:
		rawrbox::TextureBase* _texture = nullptr;

	public:
		TextureWrapper(rawrbox::TextureBase* texture);
		TextureWrapper(const TextureWrapper&) = default;
		TextureWrapper(TextureWrapper&&) = default;
		TextureWrapper& operator=(const TextureWrapper&) = default;
		TextureWrapper& operator=(TextureWrapper&&) = default;
		virtual ~TextureWrapper() = default;

		[[nodiscard]] virtual rawrbox::TextureBase* getRef() const;

		// UTILS----
		[[nodiscard]] virtual bool hasTransparency() const;
		[[nodiscard]] virtual rawrbox::Vector2 getSize() const;
		[[nodiscard]] virtual bool isValid() const;
		// -----

		static void registerLua(lua_State* L);
	};
} // namespace rawrbox
