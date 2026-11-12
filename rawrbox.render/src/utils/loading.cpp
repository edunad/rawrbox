#include <rawrbox/render/stencil.hpp>
#include <rawrbox/render/utils/loading.hpp>
#include <rawrbox/render/utils/pipeline.hpp>

#include <fmt/format.h>

#include <algorithm>

namespace rawrbox {
	constexpr rawrbox::Vector2f LOADING_BAR = {220.F, 4.F};
	constexpr rawrbox::Color LOADING_TASK_COLOR = rawrbox::Color::RGBHex(0x4a9eff);

	std::vector<rawrbox::LoadingTask> LoadingUtils::_tasks = {};

	void LoadingUtils::add(const std::string& id, const std::string& title, size_t done, size_t total, const std::string& detail) {
		const auto& fnd = std::ranges::find_if(LoadingUtils::_tasks, [&id](const auto& task) {
			return task.id == id;
		});

		if (fnd == LoadingUtils::_tasks.end()) {
			LoadingUtils::_tasks.push_back({.id = id, .title = title, .detail = detail, .done = done, .total = total});
			return;
		}

		fnd->title = title;
		fnd->detail = detail;
		fnd->done = done;
		fnd->total = total;
	}

	void LoadingUtils::remove(const std::string& id) {
		std::erase_if(LoadingUtils::_tasks, [&id](const auto& task) {
			return task.id == id;
		});
	}

	void LoadingUtils::clear() {
		LoadingUtils::_tasks.clear();
	}

	bool LoadingUtils::hasTasks() {
		return !LoadingUtils::_tasks.empty();
	}

	const std::vector<rawrbox::LoadingTask>& LoadingUtils::getTasks() {
		return LoadingUtils::_tasks;
	}

	void LoadingUtils::draw(rawrbox::Stencil& stencil, const rawrbox::Vector2f& size) {
		const rawrbox::LoadingTask* task = LoadingUtils::_tasks.empty() ? nullptr : &LoadingUtils::_tasks.front();

		const bool compiling = rawrbox::PipelineUtils::isCompiling();
		if (task == nullptr && !compiling) return;

		float taskProgress = 1.F;
		std::string taskDetail = {};
		if (task != nullptr) {
			if (task->total != 0) taskProgress = std::clamp(static_cast<float>(task->done) / static_cast<float>(task->total), 0.F, 1.F);
			taskDetail = task->detail.empty() ? fmt::format("{}/{}", task->done, task->total) : task->detail;
		}

		const float progress = compiling ? rawrbox::PipelineUtils::getCompileProgress() : taskProgress;

		const std::string title = compiling ? "COMPILING SHADERS" : task->title;
		const std::string detail = compiling ? rawrbox::PipelineUtils::getCompilingName() : taskDetail;

		const rawrbox::Color color = compiling ? rawrbox::Colors::White() : LOADING_TASK_COLOR;

		const bool both = compiling && task != nullptr;
		const float center = size.x * 0.5F;
		const rawrbox::Vector2f barPos = {(size.x - LOADING_BAR.x) * 0.5F, size.y - (both ? 57.F : 45.F)};

		stencil.drawText(fmt::format("{} {}%", title, static_cast<int>(progress * 100.F)), {center, barPos.y - 18.F}, color, rawrbox::Colors::Transparent(), rawrbox::Alignment::Center);
		LoadingUtils::drawBar(stencil, barPos, progress, color);
		if (both) LoadingUtils::drawBar(stencil, {barPos.x, barPos.y + 6.F}, taskProgress, LOADING_TASK_COLOR);

		const float below = barPos.y + (both ? 18.F : 12.F);
		stencil.drawText(detail, {center, below}, rawrbox::Colors::Gray(), rawrbox::Colors::Transparent(), rawrbox::Alignment::Center);
		if (both) stencil.drawText(fmt::format("{} {}% ({})", task->title, static_cast<int>(taskProgress * 100.F), taskDetail), {center, below + 12.F}, LOADING_TASK_COLOR, rawrbox::Colors::Transparent(), rawrbox::Alignment::Center);
	}

	void LoadingUtils::drawBar(rawrbox::Stencil& stencil, const rawrbox::Vector2f& pos, float progress, const rawrbox::Color& color) {
		stencil.drawBox(pos, LOADING_BAR, rawrbox::Color::RGBHex(0x2a2a2d));
		stencil.drawBox(pos, {LOADING_BAR.x * progress, LOADING_BAR.y}, color);
	}
} // namespace rawrbox