#pragma once

#include <rawrbox/math/color.hpp>
#include <rawrbox/math/vector2.hpp>

#include <string>
#include <vector>

namespace rawrbox {
	class Stencil;

	struct LoadingTask {
		std::string id;
		std::string title;
		std::string detail;

		size_t done = 0;
		size_t total = 0;
	};

	class LoadingUtils {
	protected:
		static std::vector<rawrbox::LoadingTask> _tasks;

		static void drawBar(rawrbox::Stencil& stencil, const rawrbox::Vector2f& pos, float progress, const rawrbox::Color& color);

	public:
		static void add(const std::string& id, const std::string& title, size_t done, size_t total, const std::string& detail = "");
		static void remove(const std::string& id);
		static void clear();

		[[nodiscard]] static bool hasTasks();
		[[nodiscard]] static const std::vector<rawrbox::LoadingTask>& getTasks();

		static void draw(rawrbox::Stencil& stencil, const rawrbox::Vector2f& size);
	};
} // namespace rawrbox