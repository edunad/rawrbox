#pragma once

#include <rawrbox/math/color.hpp>
#include <rawrbox/render/resources/font.hpp>
#include <rawrbox/ui/base.hpp>
#include <rawrbox/utils/event.hpp>

#include <string>

namespace rawrbox {
	class UIInput : public rawrbox::UIBase {
		rawrbox::Color _backgroundColor = rawrbox::Colors::Black;
		rawrbox::Color _border = rawrbox::Colors::Transparent;
		rawrbox::Color _textColor = rawrbox::Colors::White;

		// RESOURCES ---
		std::weak_ptr<rawrbox::Font> _font;
		// -----------------

		std::string _text = "";
		std::string _hint = "";
		std::string _placeholder = "";

		std::string _fillTextPattern = "";
		std::string _fillText = "";

		rawrbox::Vector2 _textSize{0, 0};
		rawrbox::Vector2 _charSize{0, 0};
		rawrbox::Vector2 _fillSizeChar{0, 0};

		float _borderSize = 2.F;
		float _offsetY = 0.5F;
		float _padding = 1.F;

		bool _readOnly = false;
		bool _numeric = false;

		uint32_t _limit = 0;
		size_t _charXPos = 0;

		// INTERNAL
		void genFill();

		void combo_paste();
		void combo_delete(uint32_t mods);
		void combo_backspace(uint32_t mods);

		void moveCharet(bool forward);
		// ---------------

	public:
		rawrbox::Event<uint32_t> onKey;
		rawrbox::Event<> onTextUpdate;
		rawrbox::Event<> onEnter;

		~UIInput() override;

		// UTILS ----
		void setHints(const std::vector<std::string>& hints);

		void setText(const std::string& text, bool updateCharet = false, bool preventEvent = false);
		[[nodiscard]] const std::string& getText() const;

		void setPlaceholder(const std::string& text);
		[[nodiscard]] const std::string& getPlaceholder() const;

		void setLimit(uint32_t limit);
		[[nodiscard]] uint32_t getLimit() const;

		void setFill(const std::string& fill);

		void setNumericOnly(bool numeric);
		[[nodiscard]] bool getNumericOnly() const;

		void setReadOnly(bool read);
		[[nodiscard]] bool getReadOnly() const;

		void setPadding(float padding);
		[[nodiscard]] float getPadding() const;

		void setColor(const rawrbox::Color& col);
		[[nodiscard]] const rawrbox::Color& getColor() const;

		void setBorderSize(float size);
		void setBorderColor(const rawrbox::Color& col);
		[[nodiscard]] const rawrbox::Color& getBorderColor() const;

		void setBackgroundColor(const rawrbox::Color& col);
		[[nodiscard]] const rawrbox::Color& getBackgroundColor() const;

		void setFont(std::shared_ptr<rawrbox::Font> font);
		void setFont(const std::string& font, int size = 11);
		[[nodiscard]] std::weak_ptr<rawrbox::Font> getFont() const;

		[[nodiscard]] bool empty() const;
		[[nodiscard]] size_t size() const;
		void clear();
		// ----------

		// INPUT ----
		void mouseDown(const rawrbox::Vector2i& mousePos, uint32_t button, uint32_t mods) override;
		void keyChar(uint32_t key) override;
		void key(uint32_t key, uint32_t scancode, uint32_t action, uint32_t mods) override;
		// -------

		// FOCUS HANDLE ----
		[[nodiscard]] bool lockKeyboard() const override;
		// -------

		// DRAW ----
		void draw(rawrbox::Stencil& stencil) override;
		// -------
	};
} // namespace rawrbox
