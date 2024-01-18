
#include <rawrbox/webm/loader.hpp>

#include <fmt/format.h>

#include <mutex>

namespace rawrbox {

	static std::mutex RENDER_THREAD_LOCK;
	void WEBM::preloadVideo() {
		std::lock_guard<std::mutex> lock(RENDER_THREAD_LOCK);
		this->_logger->info("Pre-loading video '{}'", fmt::format(fmt::fg(fmt::color::light_coral), this->_filePath.generic_string()));

		while (this->advance()) {
			auto frame = this->getFrame();
			if (!frame.valid()) throw this->_logger->error("Failed to find frame");

			rawrbox::WEBMImage img;
			if (!rawrbox::WEBMDecoder::decode(frame, img) || !img.valid()) throw this->_logger->error("Failed to decode frame");

			this->_preloadedFrames[frame.pos] = img;
		}

		this->reset();
	}

	void WEBM::internalLoad() {
		if (this->_reader == nullptr) throw this->_logger->error("Reader not initialized!");

		// Get the file info
		long long pos = 0;
		if (mkvparser::EBMLHeader().Parse(this->_reader.get(), pos)) {
			throw this->_logger->error("File parsing failed");
		}

		// Create a segment instance
		mkvparser::Segment* pSegment = nullptr;
		if (mkvparser::Segment::CreateInstance(this->_reader.get(), pos, pSegment)) {
			throw this->_logger->error("Failed to create file segment");
		}

		this->_segment = std::unique_ptr<mkvparser::Segment>(pSegment);

		if (this->_segment == nullptr || this->_segment->Load() < 0) {
			throw this->_logger->error("Failed to load segment");
		}

		const mkvparser::Tracks* tracks = this->_segment->GetTracks();
		const unsigned long tracksCount = tracks->GetTracksCount();

		const mkvparser::SegmentInfo* const segmentInfo = this->_segment->GetInfo();
		if (segmentInfo == nullptr) {
			throw this->_logger->error("Failed to load segment info");
		}

		if (this->_segment->GetCount() <= 0) throw this->_logger->error("Track {} does not contain any cluster data!", this->_trackId);

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
						throw this->_logger->error("Unknown video codec '{}'", codecId);
					}

					break;
				}
			}
		}
		// ----

		if (this->_video == nullptr) throw this->_logger->error("Failed to find track {}", this->_trackId);
		this->_videoTrack = this->_video->GetNumber();

		// Append extra info ----
		this->_info.frameRate = this->_video->GetFrameRate();
		this->_info.size = {static_cast<int>(this->_video->GetWidth()), static_cast<int>(this->_video->GetHeight())};
		// -----

		// Start pre-loading video if flag enabled ----
		if ((this->_flags & rawrbox::WEBMLoadFlags::PRELOAD) > 0) {
			this->preloadVideo();
		}
		// -----
	}

	WEBM::~WEBM() {
		this->_reader->Close();
		this->_reader.reset();

		this->_segment.reset();

		this->_preloadedFrames.clear();

		this->_cluster = nullptr;
		this->_blockEntry = nullptr;
		this->_block = nullptr;
		this->_video = nullptr;
	}

	void WEBM::load(const std::filesystem::path& filePath, uint32_t flags) {
		this->_flags = flags;
		this->_filePath = filePath;
		this->_reader = std::make_unique<mkvparser::MkvReader>();

		if (this->_reader->Open(filePath.string().c_str()))
			throw this->_logger->error("Filename is invalid or error while opening");

		this->internalLoad();
	}

	bool WEBM::advance() {
		if (this->_video == nullptr) throw this->_logger->error("Video not loaded! Did you call load()?");
		if (this->_paused) return false;

		if (this->eos()) {
			if (this->_loop) {
				this->reset();
				this->onEnd();
			} else {
				this->setPaused(true);
				this->onEnd();

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
		this->_frame.pos = blockFrame.pos;

		blockFrame.Read(this->_reader.get(), this->_frame.buffer.data());
		// ----------------------

		return true;
	}

	bool WEBM::eos() {
		return this->_eos;
	}

	void WEBM::reset() {
		if (this->_video == nullptr || this->_segment == nullptr) throw this->_logger->error("Video not loaded! Did you call 'load' ?");

		this->_cluster = this->_segment->GetFirst();

		this->_blockEntry = nullptr;
		this->_block = nullptr;

		this->_blockFrameIndex = 0;
		this->_eos = false;
		this->_paused = false;
	}

	void WEBM::seek(uint64_t timeMS) {
		if (this->_video == nullptr || this->_segment == nullptr) throw this->_logger->error("Video not loaded! Did you call 'load' ?");

		this->_cluster = this->_segment->FindCluster(timeMS * 1000000);

		this->_blockEntry = nullptr;
		this->_block = nullptr;

		this->_blockFrameIndex = 0;
		this->_eos = false;
	}

	bool WEBM::getNextFrame(rawrbox::WEBMImage& img) {
		if (this->_paused) return false;

		bool success = this->advance();
		if (!success) return false;

		auto frame = this->getFrame();
		if (!frame.valid()) return false;

		if (this->isPreLoaded()) {
			img = this->_preloadedFrames[frame.pos];
		} else {
			std::lock_guard<std::mutex> lock(RENDER_THREAD_LOCK);
			rawrbox::WEBMDecoder::decode(frame, img);
		}

		return img.valid();
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

	bool WEBM::isPreLoaded() {
		return (this->_flags & rawrbox::WEBMLoadFlags::PRELOAD) > 0;
	}
	// -------
} // namespace rawrbox
