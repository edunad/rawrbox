#pragma once

#include <rawrbox/math/color.hpp>
#include <rawrbox/render/resources/font.hpp>
#include <rawrbox/render/text/font.hpp>
#include <rawrbox/ui/container.hpp>
#include <rawrbox/utils/event.hpp>

#include <string>

namespace rawrbox {
	class UIInput : public rawrbox::UIContainer {
		rawrbox::Color _backgroundColor = {0.15F, 0.15F, 0.15F, 1.F};
		rawrbox::Color _border = rawrbox::Colors::Transparent;
		rawrbox::Color _textColor = rawrbox::Colors::White;

		// RESOURCES ---
		rawrbox::Font* _font = nullptr;
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

		// UTILS ----
		virtual void setHints(const std::vector<std::string>& hints);

		virtual void setText(const std::string& text, bool updateCharet = false, bool preventEvent = false);
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

		virtual void setColor(const rawrbox::Color& col);
		[[nodiscard]] virtual const rawrbox::Color& getColor() const;

		virtual void setBorderSize(float size);
		virtual void setBorderColor(const rawrbox::Color& col);
		[[nodiscard]] virtual const rawrbox::Color& getBorderColor() const;

		virtual void setBackgroundColor(const rawrbox::Color& col);
		[[nodiscard]] virtual const rawrbox::Color& getBackgroundColor() const;

		virtual void setFont(rawrbox::Font* font);
		virtual void setFont(const std::filesystem::path& font, int size = 11);
		[[nodiscard]] virtual rawrbox::Font* getFont() const;

		[[nodiscard]] virtual bool empty() const;
		[[nodiscard]] virtual size_t size() const;
		virtual void clear();
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
		[[nodiscard]] bool clipOverflow() const override;
		void draw(rawrbox::Stencil& stencil) override;
		// -------
	};
} // namespace rawrbox
