
#pragma once
#include <rawrbox/render/text/font.hpp>
#include <rawrbox/render/textures/base.hpp>
#include <rawrbox/ui/container.hpp>
#include <rawrbox/utils/event.hpp>

namespace rawrbox {
	class UIProgressBar : public rawrbox::UIContainer {
	protected:
		// RESOURCES ---
		rawrbox::TextureBase* _bg = nullptr;
		rawrbox::TextureBase* _overlay = nullptr;

		rawrbox::Font* _font = nullptr;
		// -----------------

		rawrbox::Vector2f _labelSize = {};
		rawrbox::Color _progressColor = rawrbox::Color(0.8F, 0.8F, 0.8F, 1.F);

		bool _percent = true;
		float _value = 0;

#ifdef RAWRBOX_SCRIPTING
		void initializeLua() override;
#endif

	public:
		rawrbox::Event<float> onValueChange;

		UIProgressBar() = default;
		UIProgressBar(const UIProgressBar&) = default;
		UIProgressBar(UIProgressBar&&) = delete;
		UIProgressBar& operator=(const UIProgressBar&) = default;
		UIProgressBar& operator=(UIProgressBar&&) = delete;
		~UIProgressBar() override {
			this->_font = nullptr;
		}

		// UTILS ----
		void initialize() override;

		virtual void showPercent(bool show);
		[[nodiscard]] virtual bool isPercentVisible() const;

		virtual void setBarColor(const rawrbox::Color& color);
		[[nodiscard]] virtual const rawrbox::Color& getBarColor() const;

		virtual void setValue(float value);
		[[nodiscard]] virtual float getValue() const;
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
