
#pragma once
#include <rawrbox/math/color.hpp>
#include <rawrbox/ui/base.hpp>

#include <chrono>

namespace rawrbox {
	enum UIGraphStyle {
		BLOCK = 0,
		LINE
	};

	struct UIGraphData {
		float height = 0.F;

		rawrbox::Vector2f a;
		rawrbox::Vector2f b;
		rawrbox::Color col;
	};

	struct UIGraphCategory {
		std::chrono::high_resolution_clock::time_point timer;

	public:
		static constexpr size_t ENTRY_COUNT = 128;

		size_t id = 0;
		std::string name = "";
		size_t smoothSize = 0;
		rawrbox::Color color = rawrbox::Colors::White;

		std::array<float, ENTRY_COUNT> entries = {0};
		std::array<float, ENTRY_COUNT> smoothed = {0};

		void addEntry(float data) {
			// move times one to the right
			for (size_t i = 0; i < entries.size() - 1; i++) {
				this->entries[i] = entries[i + 1];
				this->smoothed[i] = smoothed[i + 1];
			}

			// overwrite current
			this->entries.back() = data;
		}

		void startTimer() {
			this->timer = std::chrono::high_resolution_clock::now();
		}

		void stopTimer() {
			auto time = std::chrono::duration<float>(std::chrono::high_resolution_clock::now() - this->timer).count() * 1000;
			this->addEntry(time);
		}
	};

	class UIGraph : public rawrbox::UIBase {
	protected:
		std::vector<std::unique_ptr<rawrbox::UIGraphCategory>> _categories = {};

		float _highestValue = 60.0F;
		size_t _smoothSize = 10;

		bool _autoScale = false;
		rawrbox::UIGraphStyle _style = rawrbox::UIGraphStyle::BLOCK;

		std::array<float, rawrbox::UIGraphCategory::ENTRY_COUNT> _totalTimes = {};
		std::vector<std::array<rawrbox::Vector2f, rawrbox::UIGraphCategory::ENTRY_COUNT>> _vertCats = {};

	public:
		// CATEGORY --
		virtual rawrbox::UIGraphCategory& getCategory(size_t id);
		virtual rawrbox::UIGraphCategory& addCategory(const std::string& name, const rawrbox::Color& color);
		// ----------

		// UTILS ----
		virtual void setStyle(rawrbox::UIGraphStyle style);
		virtual void setSmoothing(size_t frames);
		virtual void setAutoScale(bool val);
		virtual void setHighest(float val);
		// ---------

		// FOCUS HANDLE ---
		[[nodiscard]] bool hitTest(const rawrbox::Vector2f& point) const override;
		// -----

		// DRAW ----
		void update() override;
		[[nodiscard]] bool clipOverflow() const override;
		void draw(rawrbox::Stencil& stencil) override;
		// -------
	};
} // namespace rawrbox
