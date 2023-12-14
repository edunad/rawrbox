#pragma once

#include <rawrbox/utils/event.hpp>
#include <rawrbox/webm/decoder.hpp>

#include <mkvparser/mkvparser.h>
#include <mkvparser/mkvreader.h>

#include <filesystem>
#include <string>

namespace rawrbox {
	// NOLINTBEGIN{unused-const-variable}
	namespace WEBMLoadFlags {
		const uint32_t NONE = 0;
		const uint32_t PRELOAD = 1 << 1;
	}; // namespace WEBMLoadFlags
	// NOLINTEND{unused-const-variable}

	struct WEBMInfo {
		rawrbox::Vector2i size = {};

		rawrbox::VIDEO_CODEC vCodec = rawrbox::VIDEO_CODEC::UNKNOWN;
		std::string title;

		uint64_t duration = 0;
		uint64_t timeScale = 0;
		double frameRate = 0;
	};

	class WEBM {
	private:
		std::filesystem::path _filePath;

		uint32_t _trackId = 0;
		int _blockFrameIndex = 0;
		int _videoTrack = 0;

		bool _loop = false;
		bool _paused = false;
		bool _eos = false;

		uint32_t _flags = 0;

		rawrbox::WEBMInfo _info = {};
		rawrbox::WEBMFrame _frame = {};
		std::unordered_map<long long, rawrbox::WEBMImage> _preloadedFrames = {};

		std::unique_ptr<mkvparser::MkvReader> _reader = nullptr;
		std::unique_ptr<mkvparser::Segment> _segment = nullptr;

		const mkvparser::Cluster* _cluster = nullptr;
		const mkvparser::BlockEntry* _blockEntry = nullptr;
		const mkvparser::Block* _block = nullptr;
		const mkvparser::VideoTrack* _video = nullptr;

		void preloadVideo();

	public:
		rawrbox::Event<> onEnd;

		WEBM() = default;
		WEBM(const WEBM&) = delete;
		WEBM(WEBM&&) = delete;
		WEBM& operator=(const WEBM&) = delete;
		WEBM& operator=(WEBM&&) = delete;
		~WEBM();

		void load(const std::filesystem::path& filePath, uint32_t flags = 0);

		bool advance();
		bool eos();

		void reset();
		void seek(uint64_t timeMS);

		[[nodiscard]] bool getNextFrame(rawrbox::WEBMImage& img);

		// UTILS ------
		[[nodiscard]] const rawrbox::Vector2i& getSize() const;
		[[nodiscard]] const rawrbox::WEBMFrame& getFrame() const;
		[[nodiscard]] const rawrbox::WEBMInfo& getInfo() const;

		bool getLoop();
		void setLoop(bool loop);

		bool getPaused();
		void setPaused(bool loop);

		bool isPreLoaded();
		// --------
	};
} // namespace rawrbox
