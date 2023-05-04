#pragma once

#include <rawrbox/bass/sound_flags.hpp>
#include <rawrbox/bass/sound_instance.hpp>

#include <cstdint>
#include <memory>

namespace rawrBox {

	class Sound {
	private:
		uint32_t _sample = 0;
		uint32_t _fxSample = 0;
		bool _isStream = false;

		uint32_t _flags = rawrBox::SoundFlags::NONE;

	public:
		virtual ~Sound();

		Sound() = default;
		Sound(Sound&&) = delete;
		Sound& operator=(Sound&&) = delete;
		Sound(const Sound&) = delete;
		Sound& operator=(const Sound&) = delete;

		std::shared_ptr<rawrBox::SoundInstance> createInstance();
	};
} // namespace rawrBox
