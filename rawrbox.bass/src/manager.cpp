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

	// LOGGER ------
	std::unique_ptr<rawrbox::Logger> BASS::_logger = std::make_unique<rawrbox::Logger>("RawrBox-BASS");
	// -------------

	// STATIC VARS ----
	bool BASS::_initialized = false;
	bool BASS::_shutdown = false;
	bool BASS::_muteOnUnfocus = true;
	float BASS::_masterVolume = 1.F;
	rawrbox::Vector3f BASS::_oldLocation = {};

	// PUBLIC
	std::unordered_map<std::string, std::unique_ptr<rawrbox::SoundBase>> BASS::sounds = {};

	rawrbox::EventNamed<std::pair<uint32_t, double>> BASS::onBEAT;
	rawrbox::EventNamed<std::pair<uint32_t, float>> BASS::onBPM;
	rawrbox::EventNamed<uint32_t> BASS::onSoundEnd;
	// ------------

	// BASS CALLBACKS (Not thread safe! We need to run these on the main thread) ------
	void CALLBACK onHTTPSoundFree(HSYNC /*handle*/, DWORD channel, DWORD /*data*/, void* /*user*/) {
		if (rawrbox::BASS::_shutdown) return;

		rawrbox::runOnRenderThread([channel]() {
			for (auto it = rawrbox::BASS::sounds.begin(); it != rawrbox::BASS::sounds.end();) {
				if ((*it).second != nullptr && (*it).second->getSample() == channel) {
					it = rawrbox::BASS::sounds.erase(it);
					continue;
				}
				++it;
			}
		});
	}

	void CALLBACK soundBEAT(uint32_t channel, double beatpos, void* /*user*/) {
		if (rawrbox::BASS::_shutdown) return;

		rawrbox::runOnRenderThread([channel, beatpos]() {
			rawrbox::BASS::onBEAT({channel, beatpos});
		});
	}

	void CALLBACK soundBPM(uint32_t channel, float bpm, void* /*user*/) {
		if (rawrbox::BASS::_shutdown) return;

		rawrbox::runOnRenderThread([channel, bpm]() {
			rawrbox::BASS::onBPM({channel, bpm});
		});
	}

	void CALLBACK soundEnd(HSYNC /*handle*/, DWORD channel, DWORD /*data*/, void* /*user*/) {
		if (rawrbox::BASS::_shutdown) return;

		rawrbox::runOnRenderThread([channel]() {
			rawrbox::BASS::onSoundEnd(static_cast<uint32_t>(channel));
		});
	}
	// ----------------

	void BASS::initialize() {
		auto fxVersion = HIWORD(BASS_FX_GetVersion());
		if (fxVersion != BASSVERSION) throw _logger->error("BASS Version missmatch! FX [{}] | BASS [{}]", fxVersion, BASSVERSION);

		_initialized = (BASS_Init(-1, 44100, BASS_DEVICE_3D, nullptr, nullptr) != 0);
		if (_initialized) {
			_logger->info("Initialized BASS [{}] and BASS_FX [{}]", fxVersion, BASSVERSION);

			BASS_Start();
			BASS_Set3DFactors(1.0F, 10.0F, 1.0F);
			BASS_Apply3D();
		} else {
			throw _logger->error("BASS initialize error: {}", BASS_ErrorGetCode());
		}
	}

	void BASS::shutdown() {
		if (!_initialized || _shutdown) return;
		_shutdown = true;

		sounds.clear();
		BASS_Free();

		_initialized = false;

		_logger->warn("BASS Shutdown");
		_logger.reset();
	}

	// LOAD ----
	rawrbox::SoundBase* BASS::loadSound(const std::filesystem::path& path, uint32_t flags) {
		std::string pth = path.generic_string();

		if (sounds.find(pth) != sounds.end()) return sounds[pth].get();
		if (!std::filesystem::exists(path)) throw _logger->error("File '{}' not found!", pth);

		auto size = std::filesystem::file_size(path);
		if (path.generic_string().rfind(".3D") != std::string::npos) flags |= SoundFlags::SOUND_3D;

		bool is3D = (flags & SoundFlags::SOUND_3D) > 0;
		bool beatDetection = (flags & SoundFlags::BEAT_DETECTION) > 0;
		bool bpmDetection = (flags & SoundFlags::BPM_DETECTION) > 0;
		bool shouldStream = size > 1024 * 1024 * 2;

		HSAMPLE sample = 0;
		auto bassFlags = is3D ? BASS_SAMPLE_3D | BASS_SAMPLE_MONO | BASS_SAMPLE_OVER_DIST | BASS_SAMPLE_MUTEMAX : BASS_SAMPLE_OVER_POS;

		if (shouldStream) {
			sample = BASS_StreamCreateFile(0, path.generic_string().c_str(), 0, 0, bassFlags);
		} else {
			sample = BASS_SampleLoad(0, path.generic_string().c_str(), 0, 0, MAX_SOUND_INSTANCES, bassFlags);
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
		if (!url.starts_with("http://") && !url.starts_with("https://")) throw _logger->error("Invalid sound url '{}'", url);
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
