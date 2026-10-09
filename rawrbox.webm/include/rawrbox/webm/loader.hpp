#pragma once

#include <rawrbox/utils/event.hpp>
#include <rawrbox/utils/logger.hpp>
#include <rawrbox/webm/decoder.hpp>

#include <mkvparser/mkvparser.h>
#include <mkvparser/mkvreader.h>

#include <filesystem>
#include <string>
#include <unordered_map>

namespace rawrbox {
	// NOLINTBEGIN{unused-const-variable}
	namespace WEBMLoadFlags {
		const uint32_t NONE = 0;
		const uint32_t PRELOAD = 1 << 1;
	}; // namespace WEBMLoadFlags
	// NOLINTEND{unused-const-variable}

	struct WEBMInfo {
		rawrbox::Vector2u size = {};

		rawrbox::VIDEO_CODEC vCodec = rawrbox::VIDEO_CODEC::UNKNOWN;
		std::string title;

		uint64_t duration = 0;
		uint64_t timeScale = 0;
		double frameRate = 0;

		size_t frames = 0;
		bool alpha = false;
	};

	struct WEBMFrameEntry {
		uint64_t time = 0;

		const mkvparser::Block* block = nullptr;
		int frame = 0;

		long long alphaPos = 0;
		long alphaLen = 0;

		size_t image = 0;
		bool key = false;
	};

	struct WEBMBlockAdditional {
		long long pos = 0;
		long len = 0;
	};

	class WEBM {
	private:
		std::filesystem::path _filePath = {};
		uint32_t _flags = 0;

		bool _loop = false;
		bool _paused = false;
		bool _ended = false;

		bool _dirty = false;
		float _speed = 1.F;

		uint64_t _time = 0;
		uint64_t _endTime = 0;

		size_t _cursor = 0;
		size_t _shown = 0;

		rawrbox::WEBMInfo _info = {};
		rawrbox::WEBMFrame _frame = {};
		rawrbox::WEBMImage _image = {};

		std::vector<rawrbox::WEBMFrameEntry> _entries = {};
		std::vector<size_t> _keyFrames = {};

		std::vector<rawrbox::WEBMImage> _preloadedFrames = {};

		std::unique_ptr<mkvparser::MkvReader> _reader = nullptr;
		std::unique_ptr<mkvparser::Segment> _segment = nullptr;
		std::unique_ptr<rawrbox::WEBMDecoder> _decoder = nullptr;

		const mkvparser::VideoTrack* _video = nullptr;

		// LOGGER ------
		std::unique_ptr<rawrbox::Logger> _logger = std::make_unique<rawrbox::Logger>("RawrBox-WEBM");
		// -------------

		// LOADING ----
		void internalLoad();
		void loadTrack();
		void buildIndex();
		// -----------

		void preloadVideo();

		void readBlockAdd(const mkvparser::Cluster* cluster, std::unordered_map<long long, rawrbox::WEBMBlockAdditional>& additions) const;
		void readBlockGroup(long long pos, long long stop, std::unordered_map<long long, rawrbox::WEBMBlockAdditional>& additions) const;
		void readBlockExtra(long long pos, long long stop, rawrbox::WEBMBlockAdditional& additional) const;

		[[nodiscard]] rawrbox::WEBMColorHint getColorHint() const;
		[[nodiscard]] uint64_t getFrameDuration() const;

		void readFrame(size_t index);
		void present(size_t target);

		[[nodiscard]] size_t findFrame(uint64_t time) const;
		[[nodiscard]] size_t findKeyFrame(size_t index) const;

	public:
		rawrbox::Event<> onEnd;

		WEBM() = default;
		WEBM(const WEBM&) = delete;
		WEBM(WEBM&&) = delete;
		WEBM& operator=(const WEBM&) = delete;
		WEBM& operator=(WEBM&&) = delete;
		~WEBM();

		void load(const std::filesystem::path& filePath, uint32_t flags = 0);
		[[nodiscard]] bool update(float deltaTime);

		void reset();
		void seek(uint64_t timeMS);

		// UTILS ------
		[[nodiscard]] const rawrbox::WEBMImage& getImage() const;
		[[nodiscard]] const rawrbox::Vector2u& getSize() const;
		[[nodiscard]] const rawrbox::WEBMInfo& getInfo() const;
		[[nodiscard]] uint64_t getTime() const;

		[[nodiscard]] bool eos() const;

		[[nodiscard]] bool getLoop() const;
		void setLoop(bool loop);

		[[nodiscard]] bool getPaused() const;
		void setPaused(bool paused);

		[[nodiscard]] float getSpeed() const;
		void setSpeed(float speed);

		[[nodiscard]] bool isPreLoaded() const;
		// --------
	};
} // namespace rawrbox
