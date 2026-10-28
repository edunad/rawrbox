#include <rawrbox/render/stencil.hpp>
#include <rawrbox/render/utils/loading.hpp>
#include <rawrbox/render/utils/pipeline.hpp>

#include <fmt/format.h>

#include <algorithm>

namespace rawrbox {
	std::vector<rawrbox::LoadingTask> LoadingUtils::_tasks = {};

	void LoadingUtils::add(const std::string& id, const std::string& title, size_t done, size_t total, const std::string& detail) {
		const auto fnd = std::ranges::find_if(_tasks, [&id](const auto& task) {
			return task.id == id;
		});

		if (fnd == _tasks.end()) {
			_tasks.push_back({.id = id, .title = title, .detail = detail, .done = done, .total = total});
			return;
		}

		fnd->title = title;
		fnd->detail = detail;
		fnd->done = done;
		fnd->total = total;
	}

	void LoadingUtils::remove(const std::string& id) {
		std::erase_if(_tasks, [&id](const auto& task) {
			return task.id == id;
		});
	}

	void LoadingUtils::clear() {
		_tasks.clear();
	}

	bool LoadingUtils::hasTasks() {
		return !_tasks.empty();
	}

	const std::vector<rawrbox::LoadingTask>& LoadingUtils::getTasks() {
		return _tasks;
	}

	void LoadingUtils::draw(rawrbox::Stencil& stencil, const rawrbox::Vector2f& size) {
		std::string title;
		std::string detail;

		float progress = 1.F;

		if (rawrbox::PipelineUtils::isCompiling()) { // Shader & required tasks
			title = "COMPILING SHADERS";
			detail = rawrbox::PipelineUtils::getCompilingName();
			progress = rawrbox::PipelineUtils::getCompileProgress();
		} else if (!_tasks.empty()) { // Custom tasks
			const auto& task = _tasks.front();

			title = task.title;
			detail = task.detail.empty() ? fmt::format("{}/{}", task.done, task.total) : task.detail;
			if (task.total != 0) progress = std::clamp(static_cast<float>(task.done) / static_cast<float>(task.total), 0.F, 1.F);
		} else {
			return;
		}

		constexpr rawrbox::Vector2f bar = {220.F, 4.F};
		const rawrbox::Vector2f barPos = {(size.x - bar.x) * 0.5F, size.y - 45.F};

		stencil.drawText(fmt::format("{} {}%", title, static_cast<int>(progress * 100.F)), {size.x * 0.5F, barPos.y - 18.F}, rawrbox::Colors::White(), rawrbox::Colors::Transparent(), rawrbox::Alignment::Center);

		stencil.drawBox(barPos, bar, rawrbox::Color::RGBHex(0x2a2a2d));
		stencil.drawBox(barPos, {bar.x * progress, bar.y}, rawrbox::Colors::White());

		stencil.drawText(detail, {size.x * 0.5F, barPos.y + 12.F}, rawrbox::Colors::Gray(), rawrbox::Colors::Transparent(), rawrbox::Alignment::Center);
	}
} // namespace rawrbox