#pragma once

#include <rawrbox/render/webm/decoder.hpp>

#include <mkvparser/mkvparser.h>
#include <mkvparser/mkvreader.h>

#include <filesystem>
#include <memory>
#include <string>

namespace rawrbox {
	enum class VIDEO_CODEC {
		NO_VIDEO = 0,
		VIDEO_VP8,
		VIDEO_VP9
	};

	struct WEBMInfo {
		rawrbox::VIDEO_CODEC vCodec = rawrbox::VIDEO_CODEC::NO_VIDEO;
		std::string title;

		uint64_t duration = 0;
		uint64_t timeScale = 0;
	};

	class WEBM {
	private:
		uint32_t _trackId = 0;

		rawrbox::WEBMInfo _info = {};

		std::unique_ptr<mkvparser::MkvReader> _reader = nullptr;
		std::unique_ptr<mkvparser::Segment> _segment = nullptr;

		const mkvparser::VideoTrack* _video = nullptr;

	public:
		WEBM() = default;
		WEBM(const WEBM&) = delete;
		WEBM(WEBM&&) = delete;
		WEBM& operator=(const WEBM&) = delete;
		WEBM& operator=(WEBM&&) = delete;
		~WEBM();

		void load(const std::filesystem::path& filePath);
		rawrbox::WEBMFrame getVideoFrame(uint32_t frame);
	};
} // namespace rawrbox
