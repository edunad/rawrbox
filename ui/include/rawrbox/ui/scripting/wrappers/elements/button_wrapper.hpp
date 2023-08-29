#pragma once

#include <rawrbox/math/color.hpp>
#include <rawrbox/render/scripting/wrappers/textures/base_wrapper.hpp>
#include <rawrbox/scripting/mod.hpp>
#include <rawrbox/ui/scripting/wrappers/ui_container_wrapper.hpp>

#include <sol/sol.hpp>

namespace rawrbox {
	class ButtonWrapper : public rawrbox::UIContainerWrapper {

	public:
		using UIContainerWrapper::UIContainerWrapper;

		virtual void setTextureSize(const rawrbox::Vector2i& size);
		virtual void setTextureColor(const rawrbox::Colori& col);
		[[nodiscard]] virtual rawrbox::Colori getTextureColor() const;

		virtual void setTextColor(const rawrbox::Colori& col);
		[[nodiscard]] virtual rawrbox::Colori getTextColor() const;

		virtual void setBackgroundColor(const rawrbox::Colori& col);
		[[nodiscard]] virtual rawrbox::Colori getBackgroundColor() const;

		virtual void setText(const std::string& text, sol::optional<int> size);
		[[nodiscard]] virtual const std::string& getText() const;

		virtual void setTooltip(const std::string& text);
		[[nodiscard]] virtual const std::string& getTooltip() const;

		virtual void setTexture(const std::string& path, sol::this_environment modEnv);
		virtual void setTexture(const rawrbox::TextureWrapper& texture);

		virtual void setEnabled(bool enabled);
		[[nodiscard]] virtual bool isEnabled() const;

		virtual void setBorder(bool enabled);
		[[nodiscard]] virtual bool borderEnabled() const;

		virtual void onClick(sol::function callback);

		static void registerLua(sol::state& lua);
	};
} // namespace rawrbox
