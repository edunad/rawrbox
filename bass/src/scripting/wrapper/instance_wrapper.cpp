
#include <rawrbox/bass/scripting/wrapper/instance_wrapper.hpp>
#include <rawrbox/scripting/utils/lua.hpp>

namespace rawrbox {
	SoundInstanceWrapper::SoundInstanceWrapper(const std::shared_ptr<rawrbox::SoundInstance>& instance) : _ref(instance) {}
	SoundInstanceWrapper::~SoundInstanceWrapper() { this->_ref.reset(); }

	void SoundInstanceWrapper::play() {
		if (!this->isValid()) return;
		this->_ref.lock()->play();
	}

	void SoundInstanceWrapper::pause() {
		if (!this->isValid()) return;
		this->_ref.lock()->pause();
	}

	void SoundInstanceWrapper::stop() {
		if (!this->isValid()) return;
		this->_ref.lock()->stop();
	}

	// UTILS ----
	uint32_t SoundInstanceWrapper::id() const {
		if (!this->isValid()) throw std::runtime_error("Invalid sound instance");
		return this->_ref.lock()->id();
	}

	bool SoundInstanceWrapper::isValid() const {
		return !this->_ref.expired() && this->_ref.lock()->isValid();
	}

	bool SoundInstanceWrapper::isCreated() const {
		if (!this->isValid()) throw std::runtime_error("Invalid sound instance");
		return this->_ref.lock()->isCreated();
	}

	bool SoundInstanceWrapper::is3D() const {
		if (!this->isValid()) throw std::runtime_error("Invalid sound instance");
		return this->_ref.lock()->is3D();
	}

	float SoundInstanceWrapper::getTempo() const {
		if (!this->isValid()) throw std::runtime_error("Invalid sound instance");
		return this->_ref.lock()->getTempo();
	}

	float SoundInstanceWrapper::getVolume() const {
		if (!this->isValid()) throw std::runtime_error("Invalid sound instance");
		return this->_ref.lock()->getVolume();
	}

	double SoundInstanceWrapper::getSeek() const {
		if (!this->isValid()) throw std::runtime_error("Invalid sound instance");
		return this->_ref.lock()->getSeek();
	}

	bool SoundInstanceWrapper::isLooping() const {
		if (!this->isValid()) throw std::runtime_error("Invalid sound instance");
		return this->_ref.lock()->isLooping();
	}

	bool SoundInstanceWrapper::isPlaying() const {
		if (!this->isValid()) throw std::runtime_error("Invalid sound instance");
		return this->_ref.lock()->isPlaying();
	}

	bool SoundInstanceWrapper::isPaused() const {
		if (!this->isValid()) throw std::runtime_error("Invalid sound instance");
		return this->_ref.lock()->isPaused();
	}

	bool SoundInstanceWrapper::isHTTPStream() const {
		if (!this->isValid()) throw std::runtime_error("Invalid sound instance");
		return this->_ref.lock()->isHTTPStream();
	}

	const sol::table SoundInstanceWrapper::getFFT(int bass_length, sol::state& lua) const {
		if (!this->isValid()) throw std::runtime_error("Invalid sound instance");

		// NOLINTBEGIN(cppcoreguidelines-slicing)
		sol::state_view view = lua;
		// NOLINTEND(cppcoreguidelines-slicing)
		return rawrbox::LuaUtils::vectorToLua(this->_ref.lock()->getFFT(bass_length), view);
	}

	const rawrbox::Vector3f& SoundInstanceWrapper::getPosition() const {
		if (!this->isValid()) throw std::runtime_error("Invalid sound instance");
		return this->_ref.lock()->getPosition();
	}
	// ------------------

	void SoundInstanceWrapper::setBeatSettings(float bandwidth, float center_freq, float release_time) {
		if (!this->isValid()) throw std::runtime_error("Invalid sound instance");
		return this->_ref.lock()->setBeatSettings(bandwidth, center_freq, release_time);
	}

	void SoundInstanceWrapper::setVolume(float volume) {
		if (!this->isValid()) throw std::runtime_error("Invalid sound instance");
		return this->_ref.lock()->setVolume(volume);
	}

	void SoundInstanceWrapper::setTempo(float tempo) {
		if (!this->isValid()) throw std::runtime_error("Invalid sound instance");
		return this->_ref.lock()->setTempo(tempo);
	}

	void SoundInstanceWrapper::seek(double seek) {
		if (!this->isValid()) throw std::runtime_error("Invalid sound instance");
		return this->_ref.lock()->seek(seek);
	}

	void SoundInstanceWrapper::setLooping(bool loop) {
		if (!this->isValid()) throw std::runtime_error("Invalid sound instance");
		return this->_ref.lock()->setLooping(loop);
	}

	void SoundInstanceWrapper::setPosition(const rawrbox::Vector3f& location) {
		if (!this->isValid()) throw std::runtime_error("Invalid sound instance");
		return this->_ref.lock()->setPosition(location);
	}

	void SoundInstanceWrapper::set3D(float maxDistance, sol::optional<float> minDistance) {
		if (!this->isValid()) throw std::runtime_error("Invalid sound instance");
		return this->_ref.lock()->set3D(maxDistance, minDistance.value_or(0.F));
	}

	void SoundInstanceWrapper::onBEAT(sol::function callback) {
		if (!isValid()) return;
		this->_ref.lock()->onBEAT += [callback](double power) {
			return rawrbox::LuaUtils::runCallback(callback, power);
		};
	}

	void SoundInstanceWrapper::onBPM(sol::function callback) {
		if (!isValid()) return;
		this->_ref.lock()->onBPM += [callback](float bpm) {
			return rawrbox::LuaUtils::runCallback(callback, bpm);
		};
	}

	void SoundInstanceWrapper::onEnd(sol::function callback) {
		if (!isValid()) return;
		this->_ref.lock()->onEnd += [callback]() {
			return rawrbox::LuaUtils::runCallback(callback);
		};
	}

	void SoundInstanceWrapper::registerLua(sol::state& lua) {
		lua.new_usertype<SoundInstanceWrapper>("SoundInstance",
		    sol::no_constructor,
		    "play", &SoundInstanceWrapper::play,
		    "pause", &SoundInstanceWrapper::pause,
		    "stop", &SoundInstanceWrapper::stop,

		    // UTILS ---
		    "id", &SoundInstanceWrapper::id,

		    "isValid", &SoundInstanceWrapper::isValid,
		    "isCreated", &SoundInstanceWrapper::isCreated,
		    "is3D", &SoundInstanceWrapper::is3D,

		    "getTempo", &SoundInstanceWrapper::getTempo,
		    "getVolume", &SoundInstanceWrapper::getVolume,
		    "getSeek", &SoundInstanceWrapper::getSeek,
		    "isLooping", &SoundInstanceWrapper::isLooping,
		    "isPlaying", &SoundInstanceWrapper::isPlaying,
		    "isPaused", &SoundInstanceWrapper::isPaused,
		    "isHTTPStream", &SoundInstanceWrapper::isHTTPStream,

		    "getFFT", &SoundInstanceWrapper::getFFT,
		    "getPosition", &SoundInstanceWrapper::getPosition,
		    // -----

		    "setBeatSettings", &SoundInstanceWrapper::setBeatSettings,
		    "setVolume", &SoundInstanceWrapper::setVolume,
		    "setTempo", &SoundInstanceWrapper::setTempo,
		    "seek", &SoundInstanceWrapper::seek,
		    "setLooping", &SoundInstanceWrapper::setLooping,
		    "setPosition", &SoundInstanceWrapper::setPosition,
		    "set3D", &SoundInstanceWrapper::set3D,

		    // CALLBACKS ---
		    "onBEAT", &SoundInstanceWrapper::onBEAT,
		    "onBPM", &SoundInstanceWrapper::onBPM,
		    "onEnd", &SoundInstanceWrapper::onEnd);
	}
} // namespace rawrbox
