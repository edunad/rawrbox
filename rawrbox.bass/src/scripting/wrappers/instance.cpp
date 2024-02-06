
#include <rawrbox/bass/scripting/wrappers/instance.hpp>
#include <rawrbox/bass/sound/instance.hpp>

namespace rawrbox {
	void SoundInstanceWrapper::registerLua(lua_State* L) {
		luabridge::getGlobalNamespace(L)
		    .beginClass<rawrbox::SoundInstance>("SoundInstance")
		    .addConstructorFrom<std::shared_ptr<rawrbox::SoundInstance>, void(uint32_t, bool, uint32_t)>()

		    .addFunction("play", &SoundInstance::play)
		    .addFunction("pause", &SoundInstance::pause)
		    .addFunction("stop", &SoundInstance::stop)

		    // UTILS ----
		    .addFunction("id", &SoundInstance::id)
		    .addFunction("isValid", &SoundInstance::isValid)
		    .addFunction("isCreated", &SoundInstance::isCreated)
		    .addFunction("is3D", &SoundInstance::is3D)

		    .addFunction("getTempo", &SoundInstance::getTempo)
		    .addFunction("getVolume", &SoundInstance::getVolume)
		    .addFunction("getSeek", &SoundInstance::getSeek)
		    .addFunction("isLooping", &SoundInstance::isLooping)
		    .addFunction("isPlaying", &SoundInstance::isPlaying)
		    .addFunction("isPaused", &SoundInstance::isPaused)
		    .addFunction("isHTTPStream", &SoundInstance::isHTTPStream)

		    .addFunction("getFFT", [](rawrbox::SoundInstance* self, int length, lua_State* L) {
			    if (self == nullptr) throw std::runtime_error("Failed to get instance");

			    auto fftData = self->getFFT(length);
			    return rawrbox::LuaUtils::vectorToTable(L, fftData);
		    })

		    .addFunction("getPosition", &SoundInstance::getPosition)

		    .addFunction("setBeatSettings", &SoundInstance::setBeatSettings)
		    .addFunction("setVolume", &SoundInstance::setVolume)
		    .addFunction("setTempo", &SoundInstance::setTempo)
		    .addFunction("seek", &SoundInstance::seek)
		    .addFunction("setLooping", &SoundInstance::setLooping)
		    .addFunction("setPosition", &SoundInstance::setPosition)
		    .addFunction("set3D", &SoundInstance::set3D)
		    // ------

		    // CALLBACKS ---
		    .addFunction("onBEAT", [](rawrbox::SoundInstance* self, const luabridge::LuaRef& callback) {
			    if (self == nullptr) throw std::runtime_error("Failed to get instance");
			    if (!callback.isCallable()) throw std::runtime_error("Invalid callback");

			    self->onBEAT += [callback](double beat) { luabridge::call(callback, beat); };
		    })
		    .addFunction("onBPM", [](rawrbox::SoundInstance* self, const luabridge::LuaRef& callback) {
			    if (self == nullptr) throw std::runtime_error("Failed to get instance");
			    if (!callback.isCallable()) throw std::runtime_error("Invalid callback");

			    self->onBPM += [callback](float beat) { luabridge::call(callback, beat); };
		    })
		    .addFunction("onEnd", [](rawrbox::SoundInstance* self, const luabridge::LuaRef& callback) {
			    if (self == nullptr) throw std::runtime_error("Failed to get instance");
			    if (!callback.isCallable()) throw std::runtime_error("Invalid callback");

			    self->onEnd += [callback]() { luabridge::call(callback); };
		    })
		    // ------
		    .endClass();
	}
} // namespace rawrbox
