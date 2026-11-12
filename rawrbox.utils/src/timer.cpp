#include <rawrbox/utils/time.hpp>
#include <rawrbox/utils/timer.hpp>

#include <tracy/Tracy.hpp>

#include <utility>

namespace rawrbox {
	uint32_t TIMER::ID = 0;
	std::unordered_map<std::string, std::unique_ptr<rawrbox::TIMER>> TIMER::timers = {};

	// STATIC -----
	void TIMER::update() {
		if (TIMER::timers.empty()) return;
		ZoneScopedN("TIMER::update");

		const auto time = static_cast<float>(rawrbox::TimeUtils::time());
		for (auto it = TIMER::timers.begin(); it != TIMER::timers.end();) {
			auto& timer = it->second;
			if (timer == nullptr) {
				it = TIMER::timers.erase(it);
				continue;
			}

			if (timer->_paused || time < timer->_nextTick) {
				++it;
				continue;
			}

			if (timer->_func != nullptr) { // Tick
				ZoneTransientN(zone, it->first.c_str(), true);
				timer->_func();
			}

			if (timer == nullptr) { // If timer was deleted after callback
				it = TIMER::timers.erase(it);
				continue;
			}

			// Life
			if (!timer->_infinite) timer->_ticks++;
			if (!timer->_infinite && timer->_ticks >= timer->_iterations) {
				if (timer->_onComplete) timer->_onComplete();
				it = TIMER::timers.erase(it);
				continue;
			}

			timer->_nextTick += timer->_msDelay;
			++it;
		}
	}

	rawrbox::TIMER* TIMER::simple(const std::string& id, float msDelay, std::function<void()> func, std::function<void()> onComplete) {
		return TIMER::create(id, 1, msDelay, std::move(func), std::move(onComplete));
	}

	rawrbox::TIMER* TIMER::simple(float msDelay, std::function<void()> func, std::function<void()> onComplete) {
		return TIMER::create(1, msDelay, std::move(func), std::move(onComplete));
	}

	rawrbox::TIMER* TIMER::create(const std::string& id, int reps, float msDelay, std::function<void()> func, std::function<void()> onComplete) {
		return TIMER::init(id, reps, msDelay, std::move(func), std::move(onComplete));
	}

	rawrbox::TIMER* TIMER::create(int reps, float msDelay, std::function<void()> func, std::function<void()> onComplete) {
		return TIMER::init("", reps, msDelay, std::move(func), std::move(onComplete));
	}

	rawrbox::TIMER* TIMER::init(const std::string& id, int reps, float msDelay, std::function<void()> func, std::function<void()> onComplete) {
		std::string _id = id.empty() ? std::to_string(++TIMER::ID) : id;
		if (TIMER::exists(_id)) return nullptr;

		auto t = std::make_unique<rawrbox::TIMER>();
		t->_msDelay = msDelay;
		t->_func = std::move(func);
		t->_onComplete = std::move(onComplete);
		t->_iterations = reps;
		t->_ticks = 0;
		t->_id = _id;
		t->_infinite = reps <= 0;
		t->_nextTick = static_cast<float>(rawrbox::TimeUtils::time()) + t->_msDelay;

		TIMER::timers[_id] = std::move(t);
		return TIMER::timers[_id].get();
	}

	bool TIMER::destroy(const std::string& id) {
		const auto& fnd = TIMER::timers.find(id);
		if (fnd == TIMER::timers.end()) return false;
		fnd->second = nullptr; // Update will clean it

		return true;
	}

	bool TIMER::pause(const std::string& id, bool pause) {
		const auto& fnd = TIMER::timers.find(id);
		if (fnd == TIMER::timers.end()) return false;
		fnd->second->pause(pause);

		return true;
	}

	bool TIMER::exists(const std::string& id) {
		return TIMER::timers.contains(id);
	}

	void TIMER::clear() {
		TIMER::timers.clear();
		TIMER::ID = 0;
	}
	// -----------
	void TIMER::destroy() {
		const auto& fnd = this->timers.find(this->_id);
		if (fnd == this->timers.end()) return;

		this->timers.erase(fnd);
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