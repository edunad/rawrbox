#pragma once

#include <rawrbox/math/color.hpp>
#include <rawrbox/render/scripting/wrappers/text/font_wrapper.hpp>
#include <rawrbox/scripting/mod.hpp>
#include <rawrbox/ui/scripting/wrappers/ui_container_wrapper.hpp>

#include <sol/sol.hpp>

namespace rawrbox {
	class LabelWrapper : public rawrbox::UIContainerWrapper {
	protected:
		rawrbox::Mod* _mod = nullptr;

	public:
		LabelWrapper(const std::shared_ptr<rawrbox::UIContainer>& element, rawrbox::Mod* mod);

		// UTILS -----
		virtual void setColor(const rawrbox::Colori& col);
		[[nodiscard]] virtual rawrbox::Colori getColor() const;

		virtual void setShadowPos(const rawrbox::Vector2f& pos);
		[[nodiscard]] virtual const rawrbox::Vector2f& getShadowPos() const;

		virtual void setShadowColor(const rawrbox::Colori& col);
		[[nodiscard]] virtual rawrbox::Colori getShadowColor() const;

		virtual void setText(const std::string& text);
		[[nodiscard]] virtual const std::string& getText() const;

		virtual void setFont(const rawrbox::FontWrapper& font);
		virtual void setFont(const std::string& font, sol::optional<int> size);
		[[nodiscard]] virtual rawrbox::FontWrapper getFont() const;

		virtual void sizeToContents();
		// ----

		static void registerLua(sol::state& lua);
	};
} // namespace rawrbox
