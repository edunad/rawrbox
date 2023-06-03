#include <rawrbox/utils/time_utils.hpp>
#include <rawrbox/utils/timer.hpp>

namespace rawrbox {
	uint32_t Timer::ID = 0;
	std::unordered_map<std::string, rawrbox::Timer> Timer::timers = {};

	// STATIC -----
	void Timer::update() {
		if (timers.empty()) return;

		auto time = rawrbox::curtime();
		for (auto it2 = timers.begin(); it2 != timers.end();) {

			auto& timer = (*it2).second;
			if (timer._paused || time < timer._nextTick) {
				++it2;
				continue;
			}

			if (timer._func != nullptr) timer._func(); // Tick

			// Life
			if (!timer._infinite) timer._iterations--;
			if (timer._iterations <= 0) {
				it2 = timers.erase(it2);
				continue;
			} else {
				timer._nextTick = time + timer._delay;
			}

			++it2;
		}
	}

	rawrbox::Timer Timer::simple(int msDelay, std::function<void()> func) {
		return create(1, msDelay, func);
	}

	rawrbox::Timer Timer::create(int reps, int msDelay, std::function<void()> func) {
		rawrbox::Timer t;
		t._delay = msDelay;
		t._func = func;
		t._iterations = reps;
		t._id = std::to_string(ID++);
		t._infinite = reps <= 0;

		t.start();
		return t;
	}

	const bool Timer::isRunning(const std::string& id) {
		return timers.find(id) != timers.end();
	}

	void Timer::clear() {
		timers.clear();
		ID = 0;
	}
	// -----------
	void Timer::start() {
		if (isRunning(this->_id)) return;

		this->_nextTick = rawrbox::curtime() + this->_delay;
		timers[this->_id] = *this;
	}

	void Timer::stop() {
		auto fnd = timers.find(this->_id);
		if (fnd == timers.end()) return;

		timers.erase(fnd);
	}

	void Timer::setPaused(bool pause) {
		this->_paused = pause;

		if (pause) {
			this->_pausedTime = rawrbox::curtime();
		} else {
			this->_nextTick += (rawrbox::curtime() - this->_pausedTime);
			this->_pausedTime = 0.F;
		}
	}
} // namespace rawrbox
