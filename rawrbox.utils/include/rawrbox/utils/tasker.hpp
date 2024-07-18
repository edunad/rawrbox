
#pragma once

#include <functional>
#include <memory>
#include <queue>
#include <stdexcept>

namespace rawrbox {
	class Task {

	public:
		Task() = default;
		Task(const Task&) = default;
		Task(Task&&) = delete;
		Task& operator=(const Task&) = default;
		Task& operator=(Task&&) = delete;
		virtual ~Task() = default;

		virtual void run(const std::function<void()>& /*onComplete*/) { throw std::runtime_error("Invalid"); };
		virtual void cancel(){};
	};

	template <typename T = rawrbox::Task>
		requires(std::derived_from<T, rawrbox::Task>)
	class Tasker {
	protected:
		std::queue<std::unique_ptr<T>> _tasks = {};
		std::vector<std::unique_ptr<T>> _executing = {};

		size_t _concurrent = 1;

		void runTask(std::unique_ptr<T>&& task) {
			auto* ptr = task.get();
			this->_executing.push_back(std::move(task));

			ptr->run([this, ptr]() {
				std::erase_if(this->_executing, [ptr](auto& elm) {
					return elm.get() == ptr;
				});

				if (!this->_tasks.empty()) {
					auto nextTask = std::move(this->_tasks.front());
					this->_tasks.pop();

					runTask(std::move(nextTask));
				}
			});
		}

	public:
		Tasker(size_t concurrent = 1) : _concurrent(concurrent) {}

		template <typename... Args>
		void addTask(Args&&... args) {
			auto task = std::make_unique<T>(std::forward<Args>(args)...);

			if (this->_executing.size() < this->_concurrent) {
				runTask(std::move(task));
				return;
			}

			this->_tasks.push(std::move(task));
		}

		void clear() {
			// Cancel executing tasks ----
			for (auto& task : this->_executing) {
				task->cancel();
			}

			this->_executing.clear();
			// ------

			// Cancel pending tasks --
			while (this->_tasks.size() > 0) {
				this->_tasks.front()->cancel();
				this->_tasks.pop();
			}
			//------
		}

		// UTILS ---
		[[nodiscard]] size_t total() const { return this->_tasks.size(); }
		// ---------
	};
} // namespace rawrbox
