
#include <rawrbox/render/webm/loader.hpp>

#include <fmt/format.h>

namespace rawrbox {
	void WEBM::load(const std::filesystem::path& filePath) {
		this->_reader = std::make_unique<mkvparser::MkvReader>();

		if (this->_reader->Open(filePath.string().c_str()) != 0)
			throw std::runtime_error("[WEBM] Filename is invalid or error while opening");

		// Get the file info
		long long pos = 0;
		if (mkvparser::EBMLHeader().Parse(this->_reader.get(), pos))
			throw std::runtime_error("[WEBM] File parsing failed");

		// Create a segment instance
		mkvparser::Segment* pSegment = nullptr;
		if (mkvparser::Segment::CreateInstance(this->_reader.get(), pos, pSegment))
			throw std::runtime_error("[WEBM] Failed to create file segment");

		this->_segment = std::unique_ptr<mkvparser::Segment>(pSegment);

		if (this->_segment->Load() < 0)
			throw std::runtime_error("[WEBM] Failed to load segment");

		const mkvparser::Tracks* tracks = this->_segment->GetTracks();
		const unsigned long tracksCount = tracks->GetTracksCount();

		const mkvparser::SegmentInfo* const segmentInfo = pSegment->GetInfo();
		if (segmentInfo == nullptr)
			throw std::runtime_error("[WEBM] Failed to load segment info");

		this->_info.duration = segmentInfo->GetDuration();
		this->_info.timeScale = segmentInfo->GetTimeCodeScale();
		this->_info.title = segmentInfo->GetTitleAsUTF8();
		// --------

		// Find the video & audio track
		for (unsigned long i = 0; i < tracksCount; ++i) {
			const mkvparser::Track* track = tracks->GetTrackByIndex(i);
			const std::string codecId = track->GetCodecId();

			if (!codecId.empty()) {
				if ((this->_video == nullptr || static_cast<uint64_t>(i) != this->_trackId) && track->GetType() == mkvparser::Track::kVideo) {
					if (codecId == "V_VP8") this->_info.vCodec = rawrbox::VIDEO_CODEC::VIDEO_VP8;
					if (codecId == "V_VP9") this->_info.vCodec = rawrbox::VIDEO_CODEC::VIDEO_VP9;

					if (this->_info.vCodec != rawrbox::VIDEO_CODEC::NO_VIDEO) {
						this->_video = dynamic_cast<const mkvparser::VideoTrack*>(track);
					} else {
						fmt::print("[WEBM] Unknown video codec '{}'\n", codecId);
					}

					break;
				}
			}
		}
		// ----

		if (this->_video == nullptr) throw std::runtime_error(fmt::format("[WEBM] Failed to find track {}", this->_trackId));
		if (this->_segment->GetCount() <= 0) throw std::runtime_error(fmt::format("[WEBM] Track {} does not contain any cluster data!", this->_trackId));
	}

	rawrbox::WEBMFrame WEBM::getVideoFrame(uint32_t frame) {
		if (this->_video == nullptr) throw std::runtime_error("[WEBM] Video track not set!");
		rawrbox::WEBMFrame webFrame = {};

		// Read frame
		const long videoTrackNumber = this->_video->GetNumber();
		const mkvparser::Cluster* cluster = this->_segment->GetFirst();

		// Find the frame cluster
		while (cluster != nullptr && !cluster->EOS()) {
			const mkvparser::BlockEntry* blockEntry = nullptr;
			if (cluster->GetFirst(blockEntry))
				throw std::runtime_error("[WEBM] Error parsing the first block of the cluster!");

			while (blockEntry != nullptr && !blockEntry->EOS()) {
				const mkvparser::Block* const pBlock = blockEntry->GetBlock();
				if (pBlock->GetTrackNumber() == videoTrackNumber) {
					const mkvparser::Block::Frame& blockFrame = pBlock->GetFrame(frame);

					webFrame.bufferSize = blockFrame.len;
					blockFrame.Read(this->_reader.get(), webFrame.buffer);
					return webFrame;
				}
			}

			if (cluster->GetNext(blockEntry, blockEntry))
				throw std::runtime_error("[WEBM] Error parsing the next block of the cluster!");
		}

		throw std::runtime_error("[WEBM] Failed to find cluster for the given frame!");
		// ------
	}
} // namespace rawrbox
