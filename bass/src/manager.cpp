#include <rawrbox/bass/manager.hpp>
#include <rawrbox/bass/sound/instance.hpp>
#include <rawrbox/bass/static.hpp>
#include <rawrbox/bass/utils/bass.hpp>
#include <rawrbox/engine/static.hpp>

#include <bass.h>
#include <bass_fx.h>
#include <fmt/printf.h>

#include <filesystem>
#include <stdexcept>

namespace rawrbox {
	constexpr auto MAX_SOUND_INSTANCES = 5; // Max sound effects playing at the same time

	// BASS CALLBACKS (Not thread safe! We need to run these on the main thread) ------
	void CALLBACK onHTTPSoundFree(HSYNC handle, DWORD channel, DWORD data, void* user) {
		rawrbox::runOnMainThread([channel]() {
			auto& inst = rawrbox::BASS;

			for (auto it2 = inst.sounds.begin(); it2 != inst.sounds.end();) {
				if ((*it2).second->getSample() == channel) {
					it2 = inst.sounds.erase(it2);
					return;
				} else {
					++it2;
				}
			}
		});
	}

	void CALLBACK soundBEAT(uint32_t channel, double beatpos, void* user) {
		rawrbox::runOnMainThread([channel, beatpos]() {
			rawrbox::BASS.onBEAT({channel, beatpos});
		});
	}

	void CALLBACK soundBPM(uint32_t channel, float bpm, void* user) {
		rawrbox::runOnMainThread([channel, bpm]() {
			rawrbox::BASS.onBPM({channel, bpm});
		});
	}

	void CALLBACK soundEnd(HSYNC handle, DWORD channel, DWORD data, void* user) {
		rawrbox::runOnMainThread([channel]() {
			rawrbox::BASS.onSoundEnd(static_cast<uint32_t>(channel));
		});
	}
	// ----------------

	void BASSManager::initialize() {
		auto fxVersion = HIWORD(BASS_FX_GetVersion());
		if (fxVersion != BASSVERSION) throw std::runtime_error(fmt::format("[RawrBox-BASS] BASS Version missmatch! FX [{}] | BASS [{}]", fxVersion, BASSVERSION));

		this->_initialized = BASS_Init(-1, 44100, BASS_DEVICE_3D, nullptr, nullptr);
		if (this->_initialized) {
			fmt::print("[RawrBox-BASS] INITIALIZED BASS [{}] | BASS_FX [{}] \n", fxVersion, BASSVERSION);

			BASS_Start();
			BASS_Set3DFactors(1.0F, 10.0F, 1.0F);
			BASS_Apply3D();
		} else {
			throw std::runtime_error(fmt::format("[RawrBox-BASS] BASS initialize error: {}", BASS_ErrorGetCode()));
		}
	}

	void BASSManager::shutdown() {
		this->sounds.clear();
		BASS_Free();
	}

	// LOAD ----
	std::shared_ptr<rawrbox::SoundBase> BASSManager::loadSound(const std::filesystem::path& path, uint32_t flags) {
		std::string pth = path.generic_string().c_str();

		if (this->sounds.find(pth) != this->sounds.end()) return this->sounds[pth];
		if (!std::filesystem::exists(path)) throw std::runtime_error(fmt::format("[RawrBox-BASS] File '{}' not found!", pth));

		auto size = std::filesystem::file_size(path);
		if (path.generic_string().rfind(".3D") != std::string::npos) flags |= SoundFlags::SOUND_3D;

		bool is3D = (flags & SoundFlags::SOUND_3D) > 0;
		bool beatDetection = (flags & SoundFlags::BEAT_DETECTION) > 0;
		bool bpmDetection = (flags & SoundFlags::BPM_DETECTION) > 0;
		bool shouldStream = size > 1024 * 1024 * 2;

		HSAMPLE sample = 0;
		auto bassFlags = is3D ? BASS_SAMPLE_3D | BASS_SAMPLE_MONO | BASS_SAMPLE_OVER_DIST | BASS_SAMPLE_MUTEMAX : BASS_SAMPLE_OVER_POS;

		if (shouldStream) {
			sample = BASS_StreamCreateFile(false, path.generic_string().c_str(), 0, 0, bassFlags);
		} else {
			sample = BASS_SampleLoad(false, path.generic_string().c_str(), 0, 0, MAX_SOUND_INSTANCES, bassFlags);
		}

		if (sample == 0) {
			rawrbox::BASSUtils::checkBASSError();
			return nullptr;
		}

		BASS_ChannelSetSync(sample, BASS_SYNC_END, 0, &soundEnd, nullptr);

		if (beatDetection) {
			BASS_FX_BPM_BeatCallbackReset(sample);
			BASS_FX_BPM_BeatCallbackSet(sample, std::bit_cast<BPMBEATPROC*>(&soundBEAT), nullptr);
		}

		if (bpmDetection) {
			BASS_FX_BPM_CallbackReset(sample);
			BASS_FX_BPM_CallbackSet(sample, std::bit_cast<BPMPROC*>(&soundBPM), 1, 0, 0, nullptr);
		}

		sounds[pth] = std::make_shared<rawrbox::SoundBase>(sample, 0, flags, shouldStream);
		return sounds[pth];
	}

	std::shared_ptr<rawrbox::SoundBase> BASSManager::loadHTTPSound(const std::string& url, uint32_t flags) {
		if (!url.starts_with("http://") && !url.starts_with("https://")) throw std::runtime_error(fmt::format("[BASS] Invalid sound url '{}'", url));
		if (sounds.find(url) != sounds.end()) return sounds[url];

		if (url.rfind(".3D") != std::string::npos) flags |= SoundFlags::SOUND_3D;

		bool is3D = (flags & SoundFlags::SOUND_3D) > 0;
		bool beatDetection = (flags & SoundFlags::BEAT_DETECTION) > 0;
		bool bpmDetection = (flags & SoundFlags::BPM_DETECTION) > 0;
		bool noBlock = (flags & SoundFlags::NO_BLOCK) > 0;

		auto bassFlags = BASS_SAMPLE_LOOP;
		if (noBlock) bassFlags |= BASS_STREAM_BLOCK | BASS_STREAM_AUTOFREE;
		if (is3D) bassFlags |= BASS_SAMPLE_3D | BASS_SAMPLE_MONO | BASS_SAMPLE_OVER_DIST | BASS_SAMPLE_MUTEMAX;

		HSAMPLE sampleStreamed = BASS_StreamCreateURL(url.c_str(), 0, bassFlags, nullptr, nullptr);
		if (sampleStreamed == 0) {
			rawrbox::BASSUtils::checkBASSError();
			return nullptr;
		}

		BASS_ChannelSetSync(sampleStreamed, BASS_SYNC_END, 0, &soundEnd, nullptr);
		BASS_ChannelSetSync(sampleStreamed, BASS_SYNC_MIXTIME | BASS_SYNC_FREE, 0, &onHTTPSoundFree, nullptr); // Auto cleanup on free

		if (beatDetection) {
			BASS_FX_BPM_BeatCallbackReset(sampleStreamed);
			BASS_FX_BPM_BeatCallbackSet(sampleStreamed, std::bit_cast<BPMBEATPROC*>(&soundBEAT), nullptr);
		}

		if (bpmDetection) {
			BASS_FX_BPM_CallbackReset(sampleStreamed);
			BASS_FX_BPM_CallbackSet(sampleStreamed, std::bit_cast<BPMPROC*>(&soundBPM), 1, 0, 0, nullptr);
		}

		sounds[url] = std::make_shared<rawrbox::SoundBase>(sampleStreamed, 0, flags, true);
		return sounds[url];
	}
	// ----

	// UTILS -----
	void BASSManager::setHasFocus(bool focus) {
		if (!this->_initialized) return;
		setMasterVolume(this->_muteOnUnfocus ? (focus ? this->_masterVolume : 0.F) : this->_masterVolume, false); // Mute on unfocus
	}

	void BASSManager::setMasterVolume(float volume, bool set) {
		volume = std::clamp(volume, 0.F, 1.F);
		if (set) this->_masterVolume = volume;

		BASS_SetConfig(BASS_CONFIG_GVOL_MUSIC, static_cast<DWORD>(volume * 10000));
		BASS_SetConfig(BASS_CONFIG_GVOL_SAMPLE, static_cast<DWORD>(volume * 10000));
		BASS_SetConfig(BASS_CONFIG_GVOL_STREAM, static_cast<DWORD>(volume * 10000));
	}

	void BASSManager::setMuteOnUnfocus(bool active) {
		this->_muteOnUnfocus = active;
	}

	float BASSManager::getMasterVolume() const {
		return this->_masterVolume;
	}

	void BASSManager::setListenerLocation(const rawrbox::Vector3f& location, const rawrbox::Vector3f& forward, const rawrbox::Vector3f& up) {
		if (!this->_initialized) return;

		auto velo = location - this->_oldLocation;
		this->_oldLocation = location;

		BASS_3DVECTOR Bass_Player = {location.x, location.y, location.z};
		BASS_3DVECTOR Bass_Front = {forward.x, forward.y, forward.z};
		BASS_3DVECTOR Bass_Top = {up.x, up.y, up.z};
		BASS_3DVECTOR Bass_Velocity = {velo.x, velo.y, velo.z};

		BASS_Set3DPosition(&Bass_Player, &Bass_Velocity, &Bass_Front, &Bass_Top);
		BASS_Apply3D();
	}

} // namespace rawrbox
