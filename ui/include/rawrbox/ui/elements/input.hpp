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
		std::weak_ptr<rawrbox::Font> _consola;
		// -----------------

		std::string _text = "";
		std::string _hint = "";
		std::string _placeholder = "";

		std::string _fillTextPattern = "";
		std::string _fillText = "";

		rawrbox::Vector2 _borderSize{2, 2};

		rawrbox::Vector2 _textSize{0, 0};
		rawrbox::Vector2 _charSize{0, 0};
		rawrbox::Vector2 _fillSizeChar{0, 0};

		bool _readOnly = false;
		bool _numeric = false;

		uint32_t _padding = 0;
		uint32_t _limit = 0;

		size_t _charXPos = 0;
		float _offsetY = 0.5F;

		void combo_paste();
		void combo_delete(uint32_t mods);
		void combo_backspace(uint32_t mods);
		void moveCharet(bool forward);

		void genFill();

	public:
		rawrbox::Event<uint32_t> onKey;
		rawrbox::Event<> onTextUpdate;
		rawrbox::Event<> onEnter;

		~UIInput() override;

		// UTILS ----
		void setHints(const std::vector<std::string>& hints);

		void setText(const std::string& text, bool updateCharet = false, bool preventEvent = false);
		const std::string& getText();

		void setPlaceholder(const std::string& text);
		const std::string& getPlaceholder();

		void setLimit(unsigned int limit);
		unsigned int& getLimit();

		void setFill(const std::string& fill);

		void setNumericOnly(bool numeric);
		bool getNumericOnly();

		void setReadOnly(bool read);
		bool getReadOnly();

		void setColor(const rawrbox::Color& col);
		[[nodiscard]] const rawrbox::Color& getColor() const;

		void setBorderSize(const rawrbox::Vector2& size);
		void setBorderColor(const rawrbox::Color& col);
		[[nodiscard]] const rawrbox::Color& getBorderColor() const;

		void setBackgroundColor(const rawrbox::Color& col);
		[[nodiscard]] const rawrbox::Color& getBackgroundColor() const;

		void setFont(std::shared_ptr<rawrbox::Font> font);
		void setFont(const std::string& font, int size = 11);
		[[nodiscard]] const std::weak_ptr<rawrbox::Font> getFont() const;

		void clear();
		bool empty();
		size_t size();

		void setPadding(uint32_t padding);
		[[nodiscard]] const uint32_t getPadding() const;
		// ----------

		// INPUT ----
		void mouseDown(const rawrbox::Vector2i& mousePos, uint32_t button, uint32_t mods) override;
		void keyChar(unsigned int key) override;
		void key(uint32_t key, uint32_t scancode, uint32_t action, uint32_t mods) override;
		// -------

		// DRAW ----
		void draw(rawrbox::Stencil& stencil) override;
		// -------

		// virtual bool lockKeyboard() override;
	};
} // namespace rawrbox
