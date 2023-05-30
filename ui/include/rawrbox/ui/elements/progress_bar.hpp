
#pragma once
#include <rawrbox/render/text/font.hpp>
#include <rawrbox/render/texture/base.hpp>
#include <rawrbox/ui/base.hpp>
#include <rawrbox/utils/event.hpp>

namespace rawrbox {
	class UIProgressBar : public rawrbox::UIBase {
	protected:
		// RESOURCES ---
		std::shared_ptr<rawrbox::TextureBase> _bg = nullptr;
		std::shared_ptr<rawrbox::TextureBase> _overlay = nullptr;

		std::weak_ptr<rawrbox::Font> _font_11;
		// -----------------

		rawrbox::Vector2f _labelSize = {};
		rawrbox::Color _progressColor = rawrbox::Color(0.8F, 0.8F, 0.8F, 1.F);

		bool _percent = true;
		float _value = 0;

	public:
		rawrbox::Event<float> onValueChange;

		~UIProgressBar() override;

		// UTILS ----
		void initialize() override;

		void showPercent(bool show);
		[[nodiscard]] bool isPercentVisible() const;

		void setTitle(const std::string& title);
		[[nodiscard]] const std::string& getTitle() const;

		void setBarColor(const rawrbox::Color& color);
		[[nodiscard]] const rawrbox::Color& getBarColor() const;

		void setValue(float value);
		[[nodiscard]] float getValue() const;
		// ---------

		// FOCUS HANDLE ---
		[[nodiscard]] bool hitTest(const rawrbox::Vector2f& point) const override;
		// -----

		// DRAW ----
		void draw(rawrbox::Stencil& stencil) override;
		void afterDraw(rawrbox::Stencil& stencil) override;
		// -------
	};
} // namespace rawrbox
