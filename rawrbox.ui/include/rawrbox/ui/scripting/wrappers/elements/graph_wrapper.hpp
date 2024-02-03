#pragma once

#include <rawrbox/math/color.hpp>
#include <rawrbox/render/scripting/wrappers/text/font_wrapper.hpp>
#include <rawrbox/scripting/mod.hpp>
#include <rawrbox/ui/elements/graph.hpp>
#include <rawrbox/ui/scripting/wrappers/ui_container_wrapper.hpp>

#include <sol/sol.hpp>

namespace rawrbox {
	class GraphWrapper : public rawrbox::UIContainerWrapper {

	public:
		using UIContainerWrapper::UIContainerWrapper;

		// CATEGORY --
		virtual rawrbox::UIGraphCategory& getCategory(size_t id);
		virtual rawrbox::UIGraphCategory& addCategory(const std::string& name, const rawrbox::Colori& color);
		// ----------

		// UTILS -----
		virtual void setStyle(rawrbox::UIGraphStyle style);
		virtual void setSmoothing(size_t frames);
		virtual void setAutoScale(bool val);
		virtual void setHighest(float val);
		// ----

		// TEXT ----
		virtual void setShowLegend(bool mode);
		[[nodiscard]] virtual bool getShowLegend() const;

		virtual void addText(const std::string& text, float val);

		virtual void setTextLineColor(const rawrbox::Colori& col);
		[[nodiscard]] virtual rawrbox::Colori getTextLineColor() const;

		virtual void setTextColor(const rawrbox::Colori& col);
		[[nodiscard]] virtual rawrbox::Colori getTextColor() const;

		virtual void setTextShadowPos(const rawrbox::Vector2f& pos);
		[[nodiscard]] virtual const rawrbox::Vector2f& getTextShadowPos() const;

		virtual void setTextShadowColor(const rawrbox::Colori& col);
		[[nodiscard]] virtual rawrbox::Colori getTextShadowColor() const;

		virtual void setFont(const rawrbox::FontWrapper& font);
		virtual void setFont(const std::string& font, sol::optional<uint16_t> size, sol::this_environment modEnv);
		[[nodiscard]] virtual rawrbox::FontWrapper getFont() const;
		// ----

		static void registerLua(sol::state& lua);
	};
} // namespace rawrbox
