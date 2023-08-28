#pragma once

#include <rawrbox/math/color.hpp>
#include <rawrbox/render/scripting/wrappers/text/font_wrapper.hpp>
#include <rawrbox/scripting/mod.hpp>
#include <rawrbox/ui/scripting/wrappers/ui_container_wrapper.hpp>

#include <sol/sol.hpp>

namespace rawrbox {
	class InputWrapper : public rawrbox::UIContainerWrapper {
	protected:
		rawrbox::Mod* _mod = nullptr;

	public:
		InputWrapper(const std::shared_ptr<rawrbox::UIContainer>& element, rawrbox::Mod* mod);

		// UTILS -----
		virtual void setHints(const sol::table& hints);

		virtual void setText(const std::string& text, sol::optional<bool> updateCharet, sol::optional<bool> preventEvent);
		[[nodiscard]] virtual const std::string& getText() const;

		virtual void setPlaceholder(const std::string& text);
		[[nodiscard]] virtual const std::string& getPlaceholder() const;

		virtual void setLimit(uint32_t limit);
		[[nodiscard]] virtual uint32_t getLimit() const;

		virtual void setFill(const std::string& fill);

		virtual void setNumericOnly(bool numeric);
		[[nodiscard]] virtual bool getNumericOnly() const;

		virtual void setReadOnly(bool read);
		[[nodiscard]] virtual bool getReadOnly() const;

		virtual void setPadding(float padding);
		[[nodiscard]] virtual float getPadding() const;

		virtual void setColor(const rawrbox::Colori& col);
		[[nodiscard]] virtual rawrbox::Colori getColor() const;

		virtual void setBorderSize(float size);
		virtual void setBorderColor(const rawrbox::Colori& col);
		[[nodiscard]] virtual rawrbox::Colori getBorderColor() const;

		virtual void setBackgroundColor(const rawrbox::Colori& col);
		[[nodiscard]] virtual rawrbox::Colori getBackgroundColor() const;

		virtual void setFont(const rawrbox::FontWrapper& font);
		virtual void setFont(const std::string& font, sol::optional<int> size);
		[[nodiscard]] virtual rawrbox::FontWrapper getFont() const;

		[[nodiscard]] virtual bool empty() const;
		[[nodiscard]] virtual size_t size() const;
		virtual void clear();
		// ----

		static void registerLua(sol::state& lua);
	};
} // namespace rawrbox
