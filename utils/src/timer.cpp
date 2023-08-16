#include <rawrbox/utils/time.hpp>
#include <rawrbox/utils/timer.hpp>

namespace rawrbox {
	uint32_t Timer::ID = 0;
	std::unordered_map<std::string, std::unique_ptr<rawrbox::Timer>> Timer::timers = {};

	// STATIC -----
	void Timer::update() {
		if (timers.empty()) return;

		auto time = rawrbox::TimeUtils::time();
		for (auto it2 = timers.begin(); it2 != timers.end();) {
			auto& timer = (*it2).second;
			if (timer == nullptr) {
				it2 = timers.erase(it2);
				continue;
			}

			if (timer->_paused || time < timer->_nextTick) {
				++it2;
				continue;
			}

			if (timer->_func != nullptr) timer->_func(); // Tick

			// Life
			if (!timer->_infinite) timer->_ticks++;
			if (!timer->_infinite && timer->_ticks >= timer->_iterations) {
				if (timer->_onComplete) timer->_onComplete();
				it2 = timers.erase(it2);
				continue;
			} else {
				timer->_nextTick = time + timer->_msDelay;
			}

			++it2;
		}
	}

	rawrbox::Timer* Timer::simple(const std::string& id, uint64_t msDelay, std::function<void()> func, std::function<void()> onComplete) {
		return create(id, 1, msDelay, func, onComplete);
	}

	rawrbox::Timer* Timer::simple(uint64_t msDelay, std::function<void()> func, std::function<void()> onComplete) {
		return create(1, msDelay, func, onComplete);
	}

	rawrbox::Timer* Timer::create(const std::string& id, int reps, uint64_t msDelay, std::function<void()> func, std::function<void()> onComplete) {
		return init(id, reps, msDelay, func, onComplete);
	}

	rawrbox::Timer* Timer::create(int reps, uint64_t msDelay, std::function<void()> func, std::function<void()> onComplete) {
		return init("", reps, msDelay, func, onComplete);
	}

	rawrbox::Timer* Timer::init(const std::string& id, int reps, uint64_t msDelay, std::function<void()> func, std::function<void()> onComplete) {
		std::string _id = id.empty() ? std::to_string(++ID) : id;
		if (exists(_id)) return nullptr;

		auto t = std::make_unique<rawrbox::Timer>();
		t->_msDelay = msDelay;
		t->_func = func;
		t->_onComplete = onComplete;
		t->_iterations = reps;
		t->_ticks = 0;
		t->_id = _id;
		t->_infinite = reps <= 0;
		t->_nextTick = rawrbox::TimeUtils::time() + t->_msDelay;

		timers[_id] = std::move(t);
		return timers[_id].get();
	}

	bool Timer::destroy(const std::string& id) {
		auto fnd = timers.find(id);
		if (fnd == timers.end()) return false;

		timers.erase(fnd);
		return true;
	}

	bool Timer::pause(const std::string& id, bool pause) {
		auto fnd = timers.find(id);
		if (fnd == timers.end()) return false;
		fnd->second->pause(pause);

		return true;
	}

	bool Timer::exists(const std::string& id) {
		return timers.find(id) != timers.end();
	}

	void Timer::clear() {
		timers.clear();
		ID = 0;
	}
	// -----------
	void Timer::destroy() {
		auto fnd = timers.find(this->_id);
		if (fnd == timers.end()) return;

		timers.erase(fnd);
	}

	void Timer::pause(bool pause) {
		this->_paused = pause;

		if (pause) {
			this->_pausedTime = rawrbox::TimeUtils::time();
		} else {
			this->_nextTick += (rawrbox::TimeUtils::time() - this->_pausedTime);
			this->_pausedTime = 0;
		}
	}
} // namespace rawrbox
