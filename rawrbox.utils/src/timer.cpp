#include <rawrbox/utils/time.hpp>
#include <rawrbox/utils/timer.hpp>

#include <utility>

namespace rawrbox {
	uint32_t TIMER::ID = 0;
	std::unordered_map<std::string, std::unique_ptr<rawrbox::TIMER>> TIMER::timers = {};

	// STATIC -----
	void TIMER::update() {
		if (timers.empty()) return;

		auto time = static_cast<float>(rawrbox::TimeUtils::time());
		for (auto it = timers.begin(); it != timers.end();) {
			auto& timer = (*it).second;
			if (timer == nullptr) {
				it = timers.erase(it);
				continue;
			}

			if (timer->_paused || time < timer->_nextTick) {
				++it;
				continue;
			}

			if (timer->_func != nullptr) timer->_func(); // Tick
			if (timer == nullptr) {                      // If timer was deleted after callback
				it = timers.erase(it);
				continue;
			}

			// Life
			if (!timer->_infinite) timer->_ticks++;
			if (!timer->_infinite && timer->_ticks >= timer->_iterations) {
				if (timer->_onComplete) timer->_onComplete();
				it = timers.erase(it);
				continue;
			}

			timer->_nextTick += timer->_msDelay;
			++it;
		}
	}

	rawrbox::TIMER* TIMER::simple(const std::string& id, float msDelay, std::function<void()> func, std::function<void()> onComplete) {
		return create(id, 1, msDelay, std::move(func), std::move(onComplete));
	}

	rawrbox::TIMER* TIMER::simple(float msDelay, std::function<void()> func, std::function<void()> onComplete) {
		return create(1, msDelay, std::move(func), std::move(onComplete));
	}

	rawrbox::TIMER* TIMER::create(const std::string& id, int reps, float msDelay, std::function<void()> func, std::function<void()> onComplete) {
		return init(id, reps, msDelay, std::move(func), std::move(onComplete));
	}

	rawrbox::TIMER* TIMER::create(int reps, float msDelay, std::function<void()> func, std::function<void()> onComplete) {
		return init("", reps, msDelay, std::move(func), std::move(onComplete));
	}

	rawrbox::TIMER* TIMER::init(const std::string& id, int reps, float msDelay, std::function<void()> func, std::function<void()> onComplete) {
		std::string _id = id.empty() ? std::to_string(++ID) : id;
		if (exists(_id)) return nullptr;

		auto t = std::make_unique<rawrbox::TIMER>();
		t->_msDelay = msDelay;
		t->_func = std::move(func);
		t->_onComplete = std::move(onComplete);
		t->_iterations = reps;
		t->_ticks = 0;
		t->_id = _id;
		t->_infinite = reps <= 0;
		t->_nextTick = static_cast<float>(rawrbox::TimeUtils::time()) + t->_msDelay;

		timers[_id] = std::move(t);
		return timers[_id].get();
	}

	bool TIMER::destroy(const std::string& id) {
		auto fnd = timers.find(id);
		if (fnd == timers.end()) return false;
		fnd->second = nullptr; // Update will clean it

		return true;
	}

	bool TIMER::pause(const std::string& id, bool pause) {
		auto fnd = timers.find(id);
		if (fnd == timers.end()) return false;
		fnd->second->pause(pause);

		return true;
	}

	bool TIMER::exists(const std::string& id) {
		return timers.find(id) != timers.end();
	}

	void TIMER::clear() {
		timers.clear();
		ID = 0;
	}
	// -----------
	void TIMER::destroy() {
		auto fnd = timers.find(this->_id);
		if (fnd == timers.end()) return;

		timers.erase(fnd);
	}

	void TIMER::pause(bool pause) {
		this->_paused = pause;

		if (pause) {
			this->_pausedTime = static_cast<float>(rawrbox::TimeUtils::time());
		} else {
			this->_nextTick += (static_cast<float>(rawrbox::TimeUtils::time()) - this->_pausedTime);
			this->_pausedTime = 0;
		}
	}
} // namespace rawrbox
