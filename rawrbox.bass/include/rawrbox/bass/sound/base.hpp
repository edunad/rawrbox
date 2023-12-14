#pragma once

#include <rawrbox/bass/sound/flags.hpp>
#include <rawrbox/bass/sound/instance.hpp>

#include <cstdint>
#include <memory>

namespace rawrbox {

	class SoundBase {
	private:
		uint32_t _sample = 0;
		uint32_t _fxSample = 0;
		uint32_t _flags = rawrbox::SoundFlags::NONE;

		bool _isStream = false;

		std::vector<std::shared_ptr<rawrbox::SoundInstance>> _instances = {};

	public:
		virtual ~SoundBase();

		SoundBase(uint32_t sample, uint32_t fx, uint32_t flags, bool stream = false);
		SoundBase(SoundBase&&) = delete;
		SoundBase& operator=(SoundBase&&) = delete;
		SoundBase(const SoundBase&) = delete;
		SoundBase& operator=(const SoundBase&) = delete;

		[[nodiscard]] bool isValid() const;
		[[nodiscard]] virtual uint32_t getSample() const;
		[[nodiscard]] virtual uint32_t getFXSample() const;

		virtual std::shared_ptr<rawrbox::SoundInstance> createInstance();
		virtual std::shared_ptr<rawrbox::SoundInstance> getInstance(size_t i);
	};
} // namespace rawrbox
