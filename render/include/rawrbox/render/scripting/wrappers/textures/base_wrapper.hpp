#pragma once

#include <rawrbox/render/texture/base.hpp>

#include <sol/sol.hpp>

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
		[[nodiscard]] virtual const rawrbox::Vector2i getSize() const;
		[[nodiscard]] virtual bool isValid() const;

		[[nodiscard]] virtual std::array<float, 4> getData() const;
		// -----

		static void registerLua(sol::state& lua);
	};
} // namespace rawrbox
