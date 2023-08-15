#pragma once

#include <rawrbox/render/text/font.hpp>

#include <sol/sol.hpp>

namespace rawrbox {
	class FontWrapper {
		rawrbox::Font* _ref = nullptr;

	public:
		FontWrapper(rawrbox::Font* instance);
		FontWrapper(const FontWrapper&) = default;
		FontWrapper(FontWrapper&&) = default;
		FontWrapper& operator=(const FontWrapper&) = default;
		FontWrapper& operator=(FontWrapper&&) = default;
		virtual ~FontWrapper() = default;

		[[nodiscard]] virtual rawrbox::Font* getRef() const;

		// UTILS -----
		[[nodiscard]] virtual bool isValid() const;
		[[nodiscard]] virtual float getSize() const;
		[[nodiscard]] virtual float getScale() const;
		[[nodiscard]] virtual float getLineHeight() const;

		[[nodiscard]] virtual rawrbox::Vector2f getStringSize(const std::string& text) const;
		[[nodiscard]] virtual bool hasGlyph(uint32_t codepoint) const;
		// ------

		static void registerLua(sol::state& lua);
	};
} // namespace rawrbox
