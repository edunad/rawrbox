
#include <rawrbox/render/webm/loader.hpp>

#include <fmt/format.h>

namespace rawrbox {
	WEBM::~WEBM() {
		this->_reader->Close();
		this->_reader.reset();

		this->_segment.reset();

		this->_cluster = nullptr;
		this->_blockEntry = nullptr;
		this->_block = nullptr;
		this->_video = nullptr;
	}

	void WEBM::load(const std::filesystem::path& filePath) {
		this->_reader = std::make_unique<mkvparser::MkvReader>();

		if (this->_reader->Open(filePath.string().c_str()))
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

		if (this->_segment == nullptr || this->_segment->Load() < 0)
			throw std::runtime_error("[WEBM] Failed to load segment");

		const mkvparser::Tracks* tracks = this->_segment->GetTracks();
		const unsigned long tracksCount = tracks->GetTracksCount();

		const mkvparser::SegmentInfo* const segmentInfo = this->_segment->GetInfo();
		if (segmentInfo == nullptr)
			throw std::runtime_error("[WEBM] Failed to load segment info");

		if (this->_segment->GetCount() <= 0) throw std::runtime_error(fmt::format("[WEBM] Track {} does not contain any cluster data!", this->_trackId));

		this->_info.duration = segmentInfo->GetDuration();
		this->_info.timeScale = segmentInfo->GetTimeCodeScale();
		if (segmentInfo->GetTitleAsUTF8() != nullptr) {
			this->_info.title = segmentInfo->GetTitleAsUTF8();
		} else {
			this->_info.title = "UNKNOWN";
		}
		// --------

		// Find the video & audio track
		for (unsigned long i = 0; i < tracksCount; ++i) {
			const mkvparser::Track* track = tracks->GetTrackByIndex(i);
			const std::string codecId = track->GetCodecId();

			if (!codecId.empty()) {
				if ((this->_video == nullptr || static_cast<uint64_t>(i) != this->_trackId) && track->GetType() == mkvparser::Track::kVideo) {
					if (codecId == "V_VP9") this->_info.vCodec = rawrbox::VIDEO_CODEC::VIDEO_VP9;
					if (codecId == "V_VP8") this->_info.vCodec = rawrbox::VIDEO_CODEC::VIDEO_VP8;

					if (this->_info.vCodec != rawrbox::VIDEO_CODEC::UNKNOWN) {
						this->_video = dynamic_cast<const mkvparser::VideoTrack*>(track);
					} else {
						throw std::runtime_error(fmt::format("[WEBM] Unknown video codec '{}'\n", codecId));
					}

					break;
				}
			}
		}
		// ----

		if (this->_video == nullptr) throw std::runtime_error(fmt::format("[WEBM] Failed to find track {}", this->_trackId));
		this->_videoTrack = this->_video->GetNumber();

		// Append extra info ----
		this->_info.frameRate = this->_video->GetFrameRate();
		this->_info.size = {static_cast<int>(this->_video->GetWidth()), static_cast<int>(this->_video->GetHeight())};
		// -----
	}

	bool WEBM::advance() {
		if (this->_video == nullptr) throw std::runtime_error("[WEBM] Video not loaded! Did you call load()?");
		if (this->_paused) return false;

		if (this->eos()) {
			if (_loop) {
				this->reset();
			} else {
				return false; // Reached the end
			}
		}

		if (this->_cluster == nullptr) {
			this->_cluster = this->_segment->GetFirst();
		}

		// Find the next cluster ----
		bool blockEntryEOS = false;
		do {
			bool getNewBlock = false;
			long status = 0;

			if (!this->_blockEntry && !blockEntryEOS) {
				status = this->_cluster->GetFirst(this->_blockEntry);
				getNewBlock = true;
			} else if (blockEntryEOS || this->_blockEntry->EOS()) {
				this->_cluster = this->_segment->GetNext(this->_cluster);

				if (this->_cluster == nullptr || this->_cluster->EOS()) {
					this->_eos = true;
					return false; // Reached end
				}

				status = this->_cluster->GetFirst(this->_blockEntry);

				blockEntryEOS = false;
				getNewBlock = true;
			} else if (this->_block == nullptr || this->_blockFrameIndex == this->_block->GetFrameCount() || this->_block->GetTrackNumber() != this->_videoTrack) {
				status = this->_cluster->GetNext(this->_blockEntry, this->_blockEntry);

				if (this->_blockEntry == nullptr || this->_blockEntry->EOS()) {
					blockEntryEOS = true;
					continue;
				}

				getNewBlock = true;
			}

			if (status || !this->_blockEntry) {
				return false; // Reached end
			}

			if (getNewBlock) {
				this->_block = this->_blockEntry->GetBlock();
				this->_blockFrameIndex = 0;
			}

		} while (blockEntryEOS || this->_block->GetTrackNumber() != this->_videoTrack);
		// ----------------------

		// Read the frame for decoding
		if (this->_block == nullptr) return false;

		const mkvparser::Block::Frame& blockFrame = this->_block->GetFrame(this->_blockFrameIndex++);
		this->_frame.buffer.resize(blockFrame.len);
		this->_frame.codec = this->_info.vCodec;

		blockFrame.Read(this->_reader.get(), this->_frame.buffer.data());
		// ----------------------

		return true;
	}

	bool WEBM::eos() {
		return this->_eos;
	}

	void WEBM::reset() {
		if (this->_video == nullptr || this->_segment == nullptr) throw std::runtime_error("[WEBM] Video not loaded! Did you call 'load' ?");

		this->_cluster = this->_segment->GetFirst();

		this->_blockEntry = nullptr;
		this->_block = nullptr;

		this->_blockFrameIndex = 0;
		this->_eos = false;
	}

	void WEBM::seek(uint64_t timeMS) {
		if (this->_video == nullptr || this->_segment == nullptr) throw std::runtime_error("[WEBM] Video not loaded! Did you call 'load' ?");

		this->_cluster = this->_segment->FindCluster(timeMS * 1000000);

		this->_blockEntry = nullptr;
		this->_block = nullptr;

		this->_blockFrameIndex = 0;
		this->_eos = false;
	}

	// UTILS ------
	const rawrbox::Vector2i& WEBM::getSize() const {
		return this->_info.size;
	}

	const rawrbox::WEBMFrame& WEBM::getFrame() const {
		return this->_frame;
	}

	const rawrbox::WEBMInfo& WEBM::getInfo() const {
		return this->_info;
	}

	bool WEBM::getLoop() { return this->_loop; }
	void WEBM::setLoop(bool loop) {
		if (loop != this->_loop) this->reset();
		this->_loop = loop;
	}

	bool WEBM::getPaused() { return this->_paused; }
	void WEBM::setPaused(bool paused) {
		this->_paused = paused;
	}
	// -------
} // namespace rawrbox
