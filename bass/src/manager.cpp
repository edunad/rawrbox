#include <rawrbox/bass/manager.hpp>
#include <rawrbox/bass/sound/instance.hpp>
#include <rawrbox/bass/utils/bass.hpp>
#include <rawrbox/engine/static.hpp>

#include <bass.h>
#include <bass_fx.h>
#include <fmt/printf.h>

#include <filesystem>
#include <stdexcept>

namespace rawrbox {
	constexpr auto MAX_SOUND_INSTANCES = 5; // Max sound effects playing at the same time

	// STATIC VARS ----
	bool rawrbox::BASS::_initialized = false;
	bool rawrbox::BASS::_muteOnUnfocus = true;
	float rawrbox::BASS::_masterVolume = 1.F;
	rawrbox::Vector3f rawrbox::BASS::_oldLocation = {};

	// PUBLIC
	std::unordered_map<std::string, std::unique_ptr<rawrbox::SoundBase>> rawrbox::BASS::sounds = {};

	rawrbox::EventNamed<std::pair<uint32_t, double>> rawrbox::BASS::onBEAT;
	rawrbox::EventNamed<std::pair<uint32_t, float>> rawrbox::BASS::onBPM;
	rawrbox::EventNamed<uint32_t> rawrbox::BASS::onSoundEnd;
	// ------------

	// BASS CALLBACKS (Not thread safe! We need to run these on the main thread) ------
	void CALLBACK onHTTPSoundFree(HSYNC  /*handle*/, DWORD channel, DWORD  /*data*/, void*  /*user*/) {
		rawrbox::runOnRenderThread([channel]() {
			for (auto it2 = rawrbox::BASS::sounds.begin(); it2 != rawrbox::BASS::sounds.end();) {
				if ((*it2).second->getSample() == channel) {
					it2 = rawrbox::BASS::sounds.erase(it2);
					continue;
				} else {
					++it2;
				}
			}
		});
	}

	void CALLBACK soundBEAT(uint32_t channel, double beatpos, void*  /*user*/) {
		rawrbox::runOnRenderThread([channel, beatpos]() {
			rawrbox::BASS::onBEAT({channel, beatpos});
		});
	}

	void CALLBACK soundBPM(uint32_t channel, float bpm, void*  /*user*/) {
		rawrbox::runOnRenderThread([channel, bpm]() {
			rawrbox::BASS::onBPM({channel, bpm});
		});
	}

	void CALLBACK soundEnd(HSYNC  /*handle*/, DWORD channel, DWORD  /*data*/, void*  /*user*/) {
		rawrbox::runOnRenderThread([channel]() {
			rawrbox::BASS::onSoundEnd(static_cast<uint32_t>(channel));
		});
	}
	// ----------------

	void BASS::initialize() {
		auto fxVersion = HIWORD(BASS_FX_GetVersion());
		if (fxVersion != BASSVERSION) throw std::runtime_error(fmt::format("[RawrBox-BASS] BASS Version missmatch! FX [{}] | BASS [{}]", fxVersion, BASSVERSION));

		_initialized = BASS_Init(-1, 44100, BASS_DEVICE_3D, nullptr, nullptr);
		if (_initialized) {
			fmt::print("[RawrBox-BASS] INITIALIZED BASS [{}] | BASS_FX [{}] \n", fxVersion, BASSVERSION);

			BASS_Start();
			BASS_Set3DFactors(1.0F, 10.0F, 1.0F);
			BASS_Apply3D();
		} else {
			throw std::runtime_error(fmt::format("[RawrBox-BASS] BASS initialize error: {}", BASS_ErrorGetCode()));
		}
	}

	void BASS::shutdown() {
		sounds.clear();
		BASS_Free();

		fmt::print("[RawrBox-BASS] BASS Shutdown \n");
	}

	// LOAD ----
	rawrbox::SoundBase* BASS::loadSound(const std::filesystem::path& path, uint32_t flags) {
		std::string pth = path.generic_string().c_str();

		if (sounds.find(pth) != sounds.end()) return sounds[pth].get();
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

		if (sample == 0) rawrbox::BASSUtils::checkBASSError();
		BASS_ChannelSetSync(sample, BASS_SYNC_END, 0, &soundEnd, nullptr);

		if (beatDetection) {
			BASS_FX_BPM_BeatCallbackReset(sample);
			BASS_FX_BPM_BeatCallbackSet(sample, std::bit_cast<BPMBEATPROC*>(&soundBEAT), nullptr);
		}

		if (bpmDetection) {
			BASS_FX_BPM_CallbackReset(sample);
			BASS_FX_BPM_CallbackSet(sample, std::bit_cast<BPMPROC*>(&soundBPM), 1, 0, 0, nullptr);
		}

		sounds[pth] = std::make_unique<rawrbox::SoundBase>(sample, 0, flags, shouldStream);
		return sounds[pth].get();
	}

	rawrbox::SoundBase* BASS::loadHTTPSound(const std::string& url, uint32_t flags) {
		if (!url.starts_with("http://") && !url.starts_with("https://")) throw std::runtime_error(fmt::format("[BASS] Invalid sound url '{}'", url));
		if (sounds.find(url) != sounds.end()) return sounds[url].get();

		if (url.rfind(".3D") != std::string::npos) flags |= SoundFlags::SOUND_3D;

		bool is3D = (flags & SoundFlags::SOUND_3D) > 0;
		bool beatDetection = (flags & SoundFlags::BEAT_DETECTION) > 0;
		bool bpmDetection = (flags & SoundFlags::BPM_DETECTION) > 0;
		bool noBlock = (flags & SoundFlags::NO_BLOCK) > 0;

		auto bassFlags = BASS_SAMPLE_LOOP;
		if (noBlock) bassFlags |= BASS_STREAM_BLOCK | BASS_STREAM_AUTOFREE;
		if (is3D) bassFlags |= BASS_SAMPLE_3D | BASS_SAMPLE_MONO | BASS_SAMPLE_OVER_DIST | BASS_SAMPLE_MUTEMAX;

		HSAMPLE sampleStreamed = BASS_StreamCreateURL(url.c_str(), 0, bassFlags, nullptr, nullptr);
		if (sampleStreamed == 0) rawrbox::BASSUtils::checkBASSError();

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

		sounds[url] = std::make_unique<rawrbox::SoundBase>(sampleStreamed, 0, flags, true);
		return sounds[url].get();
	}
	// ----

	// UTILS -----
	void BASS::setHasFocus(bool focus) {
		if (!_initialized) return;
		setMasterVolume(_muteOnUnfocus ? (focus ? _masterVolume : 0.F) : _masterVolume, false); // Mute on unfocus
	}

	void BASS::setMasterVolume(float volume, bool set) {
		volume = std::clamp(volume, 0.F, 1.F);
		if (set) _masterVolume = volume;

		BASS_SetConfig(BASS_CONFIG_GVOL_MUSIC, static_cast<DWORD>(volume * 10000));
		BASS_SetConfig(BASS_CONFIG_GVOL_SAMPLE, static_cast<DWORD>(volume * 10000));
		BASS_SetConfig(BASS_CONFIG_GVOL_STREAM, static_cast<DWORD>(volume * 10000));
	}

	void BASS::setMuteOnUnfocus(bool active) {
		_muteOnUnfocus = active;
	}

	float BASS::getMasterVolume() {
		return _masterVolume;
	}

	void BASS::setListenerLocation(const rawrbox::Vector3f& location, const rawrbox::Vector3f& forward, const rawrbox::Vector3f& up) {
		if (!_initialized) return;

		auto velo = location - _oldLocation;
		_oldLocation = location;

		BASS_3DVECTOR Bass_Player = {location.x, location.y, location.z};
		BASS_3DVECTOR Bass_Front = {forward.x, forward.y, forward.z};
		BASS_3DVECTOR Bass_Top = {up.x, up.y, up.z};
		BASS_3DVECTOR Bass_Velocity = {velo.x, velo.y, velo.z};

		BASS_Set3DPosition(&Bass_Player, &Bass_Velocity, &Bass_Front, &Bass_Top);
		BASS_Apply3D();
	}

} // namespace rawrbox
