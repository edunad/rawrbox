
#include <rawrbox/webm/loader.hpp>

#include <common/webmids.h>
#include <fmt/format.h>

#include <algorithm>
#include <iterator>
#include <utility>

namespace rawrbox {
	WEBM::~WEBM() {
		this->_decoder.reset();
		this->_video = nullptr;

		this->_segment.reset();
		this->_reader.reset();

		this->_entries.clear();
		this->_keyFrames.clear();
		this->_preloadedFrames.clear();
	}

	// PRIVATE ------
	void WEBM::internalLoad() {
		if (this->_reader == nullptr) RAWRBOX_CRITICAL("Reader not initialized!");

		long long pos = 0;
		mkvparser::EBMLHeader header;
		if (header.Parse(this->_reader.get(), pos) != 0) RAWRBOX_CRITICAL("File parsing failed");

		mkvparser::Segment* segment = nullptr;
		if (mkvparser::Segment::CreateInstance(this->_reader.get(), pos, segment) != 0 || segment == nullptr)
			RAWRBOX_CRITICAL("Failed to create file segment");

		this->_segment = std::unique_ptr<mkvparser::Segment>(segment);

		if (this->_segment->Load() < 0) RAWRBOX_CRITICAL("Failed to load segment");
		if (this->_segment->GetCount() <= 0) RAWRBOX_CRITICAL("Video does not contain any cluster data!");

		const mkvparser::SegmentInfo* segmentInfo = this->_segment->GetInfo();
		if (segmentInfo == nullptr) RAWRBOX_CRITICAL("Failed to load segment info");

		this->_info.duration = static_cast<uint64_t>(std::max(0LL, segmentInfo->GetDuration()));
		this->_info.timeScale = static_cast<uint64_t>(std::max(0LL, segmentInfo->GetTimeCodeScale()));
		this->_info.title = segmentInfo->GetTitleAsUTF8() != nullptr ? segmentInfo->GetTitleAsUTF8() : "UNKNOWN";

		this->loadTrack();

		this->_decoder = std::make_unique<rawrbox::WEBMDecoder>(this->_info.vCodec, this->getColorHint());
		this->buildIndex();

		if (this->isPreLoaded()) this->preloadVideo();
		this->present(0);
	}

	void WEBM::loadTrack() {
		const mkvparser::Tracks* tracks = this->_segment->GetTracks();
		if (tracks == nullptr) RAWRBOX_CRITICAL("Failed to load tracks");

		for (unsigned long i = 0; i < tracks->GetTracksCount(); ++i) {
			const mkvparser::Track* track = tracks->GetTrackByIndex(i);
			if (track == nullptr || track->GetType() != mkvparser::Track::kVideo) continue;

			const char* codecId = track->GetCodecId();
			if (codecId == nullptr) continue;

			const std::string codec = codecId;
			if (codec == "V_VP9") {
				this->_info.vCodec = rawrbox::VIDEO_CODEC::VIDEO_VP9;
			} else if (codec == "V_VP8") {
				this->_info.vCodec = rawrbox::VIDEO_CODEC::VIDEO_VP8;
			} else {
				this->_logger->warn("Skipping video track {} with unsupported codec '{}'", track->GetNumber(), codec);
				continue;
			}

			this->_video = static_cast<const mkvparser::VideoTrack*>(track);
			break;
		}

		if (this->_video == nullptr) RAWRBOX_CRITICAL("Unsupported video track found! Only VP8 and VP9 are supported");
		this->_info.size = {static_cast<uint32_t>(this->_video->GetWidth()), static_cast<uint32_t>(this->_video->GetHeight())};
	}

	void WEBM::buildIndex() {
		const long long trackNumber = this->_video->GetNumber();
		const uint64_t frameDuration = this->getFrameDuration();

		std::unordered_map<long long, rawrbox::WEBMBlockAdditional> additions = {};
		std::vector<long long> times = {};

		this->_entries.clear();
		this->_keyFrames.clear();

		for (const mkvparser::Cluster* cluster = this->_segment->GetFirst(); cluster != nullptr && !cluster->EOS(); cluster = this->_segment->GetNext(cluster)) {
			additions.clear();
			bool scanned = false;

			const mkvparser::BlockEntry* entry = nullptr;
			if (cluster->GetFirst(entry) < 0) RAWRBOX_CRITICAL("Failed to parse cluster");

			while (entry != nullptr && !entry->EOS()) {
				const mkvparser::Block* block = entry->GetBlock();

				if (block != nullptr && block->GetTrackNumber() == trackNumber) {
					if (!scanned && entry->GetKind() == mkvparser::BlockEntry::kBlockGroup) {
						this->readBlockAdd(cluster, additions);
						scanned = true;
					}

					const auto addition = additions.find(block->m_start);
					const long long blockTime = block->GetTime(cluster);

					for (int i = 0; i < block->GetFrameCount(); i++) {
						rawrbox::WEBMFrameEntry frame = {};

						frame.block = block;
						frame.frame = i;
						frame.key = i == 0 && block->IsKey();

						if (i == 0 && addition != additions.end()) {
							frame.alphaPos = addition->second.pos;
							frame.alphaLen = addition->second.len;
						}

						if (frame.key) this->_keyFrames.push_back(this->_entries.size());

						times.push_back(blockTime + static_cast<long long>(frameDuration) * i);
						this->_entries.push_back(frame);
					}
				}

				if (cluster->GetNext(entry, entry) < 0) RAWRBOX_CRITICAL("Failed to parse cluster block");
			}
		}

		if (this->_entries.empty()) return;

		const long long start = *std::ranges::min_element(times);
		uint64_t previous = 0;

		for (size_t i = 0; i < this->_entries.size(); i++) {
			previous = std::max(previous, static_cast<uint64_t>(times[i] - start));
			this->_entries[i].time = previous;
		}

		uint64_t duration = frameDuration;
		if (duration == 0 && this->_entries.size() > 1) duration = this->_entries.back().time / (this->_entries.size() - 1);
		if (duration == 0) duration = 1000000000ULL / 30ULL;

		this->_endTime = this->_entries.back().time + duration;

		this->_info.frames = this->_entries.size();
		this->_info.frameRate = this->_video->GetFrameRate() > 0.0 ? this->_video->GetFrameRate() : 1000000000.0 / static_cast<double>(duration);
		this->_info.alpha = std::ranges::any_of(this->_entries, [](const rawrbox::WEBMFrameEntry& frame) { return frame.alphaLen > 0; });
	}

	void WEBM::preloadVideo() {
		this->_logger->debug("Pre-loading video '{}'", fmt::styled(this->_filePath.generic_string(), fmt::fg(fmt::color::light_coral)));

		this->_preloadedFrames.clear();
		this->_preloadedFrames.reserve(this->_entries.size());

		for (size_t i = 0; i < this->_entries.size(); i++) {
			this->readFrame(i);

			if (this->_decoder->decode(this->_frame)) {
				this->_decoder->convert(this->_preloadedFrames.emplace_back());
			}

			this->_entries[i].image = this->_preloadedFrames.empty() ? 0 : this->_preloadedFrames.size() - 1;
		}

		if (this->_preloadedFrames.empty()) RAWRBOX_CRITICAL("Failed to decode any frame");

		this->_frame = {};
		this->_decoder.reset();

		this->_logger->debug("Done pre-loading '{}'", fmt::styled(this->_filePath.generic_string(), fmt::fg(fmt::color::light_coral)));
	}

	void WEBM::readBlockAdd(const mkvparser::Cluster* cluster, std::unordered_map<long long, rawrbox::WEBMBlockAdditional>& additions) const {
		auto* reader = this->_reader.get();

		long long pos = cluster->m_element_start;
		const long long stop = pos + cluster->GetElementSize();

		long long id = 0;
		long long size = 0;

		if (mkvparser::ParseElementHeader(reader, pos, stop, id, size) != 0 || id != libwebm::kMkvCluster) return;

		while (pos < stop) {
			if (mkvparser::ParseElementHeader(reader, pos, stop, id, size) != 0 || size > stop - pos) return;
			if (id == libwebm::kMkvBlockGroup) this->readBlockGroup(pos, pos + size, additions);

			pos += size;
		}
	}

	void WEBM::readBlockGroup(long long pos, long long stop, std::unordered_map<long long, rawrbox::WEBMBlockAdditional>& additions) const {
		auto* reader = this->_reader.get();

		long long blockStart = -1;
		rawrbox::WEBMBlockAdditional additional = {};

		long long id = 0;
		long long size = 0;

		while (pos < stop) {
			if (mkvparser::ParseElementHeader(reader, pos, stop, id, size) != 0 || size > stop - pos) return;

			switch (id) {
				case libwebm::kMkvBlock:
					blockStart = pos;
					break;

				case libwebm::kMkvBlockAdditions:
					long long morePos = pos;
					const long long moreStop = pos + size;

					long long moreId = 0;
					long long moreSize = 0;

					while (morePos < moreStop) {
						if (mkvparser::ParseElementHeader(reader, morePos, moreStop, moreId, moreSize) != 0 || moreSize > moreStop - morePos) break;
						if (moreId == libwebm::kMkvBlockMore) this->readBlockExtra(morePos, morePos + moreSize, additional);

						morePos += moreSize;
					}
					break;
			}

			pos += size;
		}

		if (blockStart >= 0 && additional.len > 0) additions[blockStart] = additional;
	}

	void WEBM::readBlockExtra(long long pos, long long stop, rawrbox::WEBMBlockAdditional& additional) const {
		auto* reader = this->_reader.get();

		long long addId = 1;
		rawrbox::WEBMBlockAdditional data = {};

		long long id = 0;
		long long size = 0;

		while (pos < stop) {
			if (mkvparser::ParseElementHeader(reader, pos, stop, id, size) != 0 || size > stop - pos) return;

			switch (id) {
				case libwebm::kMkvBlockAddID:
					addId = mkvparser::UnserializeUInt(reader, pos, size);
					break;
				case libwebm::kMkvBlockAdditional:
					data.pos = pos;
					data.len = static_cast<long>(size);
					break;
			}

			pos += size;
		}

		if (addId == 1 && data.len > 0) additional = data;
	}

	rawrbox::WEBMColorHint WEBM::getColorHint() const {
		rawrbox::WEBMColorHint hint = {};

		const mkvparser::Colour* colour = this->_video->GetColour();
		if (colour == nullptr) return hint;

		// Horrible i know
		switch (colour->matrix_coefficients) {
			case 0:
				hint.space = rawrbox::YUVColorSpace::RGB;
				break;
			case 1:
				hint.space = rawrbox::YUVColorSpace::BT709;
				break;
			case 5:
			case 6:
				hint.space = rawrbox::YUVColorSpace::BT601;
				break;
			case 7:
				hint.space = rawrbox::YUVColorSpace::SMPTE240;
				break;
			case 9:
			case 10:
				hint.space = rawrbox::YUVColorSpace::BT2020;
				break;
			default:
				break;
		}

		switch (colour->range) {
			case 1:
				hint.scale = rawrbox::YUVLuminanceScale::ITU;
				break;
			case 2:
				hint.scale = rawrbox::YUVLuminanceScale::FULL;
				break;
			default:
				break;
		}

		return hint;
	}

	uint64_t WEBM::getFrameDuration() const {
		const uint64_t duration = this->_video->GetDefaultDuration();
		if (duration > 0) return duration;

		const double rate = this->_video->GetFrameRate();
		if (rate > 0.0) return static_cast<uint64_t>(1000000000.0 / rate);

		return 0;
	}

	void WEBM::readFrame(size_t index) {
		const auto& entry = this->_entries[index];
		const auto& blockFrame = entry.block->GetFrame(entry.frame);

		this->_frame.buffer.resize(static_cast<size_t>(blockFrame.len));
		if (blockFrame.Read(this->_reader.get(), this->_frame.buffer.data()) != 0) RAWRBOX_CRITICAL("Failed to read frame {}", index);

		this->_frame.alpha.resize(static_cast<size_t>(entry.alphaLen));
		if (entry.alphaLen > 0 && this->_reader->Read(entry.alphaPos, entry.alphaLen, this->_frame.alpha.data()) != 0) RAWRBOX_CRITICAL("Failed to read alpha frame {}", index);
	}

	void WEBM::present(const size_t target) {
		if (target < this->_cursor || target >= this->_entries.size()) return;

		if (this->isPreLoaded()) {
			this->_shown = target;
			this->_cursor = target + 1;

			this->_dirty = true;
			return;
		}

		const size_t start = std::max(this->_cursor, this->findKeyFrame(target));
		bool visible = false;

		for (size_t i = start; i <= target; i++) {
			this->readFrame(i);
			visible = this->_decoder->decode(this->_frame);
		}

		this->_cursor = target + 1;
		if (!visible) return;

		this->_decoder->convert(this->_image);
		this->_shown = target;
		this->_dirty = true;
	}

	size_t WEBM::findFrame(const uint64_t time) const {
		const auto it = std::upper_bound(this->_entries.begin(), this->_entries.end(), time, [](uint64_t value, const rawrbox::WEBMFrameEntry& entry) { return value < entry.time; });
		if (it == this->_entries.begin()) return 0;

		return static_cast<size_t>(std::distance(this->_entries.begin(), it)) - 1;
	}

	size_t WEBM::findKeyFrame(size_t index) const {
		const auto it = std::upper_bound(this->_keyFrames.begin(), this->_keyFrames.end(), index);
		if (it == this->_keyFrames.begin()) return 0;

		return *std::prev(it);
	}
	// -------------

	void WEBM::load(const std::filesystem::path& filePath, uint32_t flags) {
		this->_flags = flags;
		this->_filePath = filePath;

		this->_time = 0;
		this->_cursor = 0;
		this->_shown = 0;
		this->_ended = false;
		this->_dirty = false;

		this->_image = {};
		this->_video = nullptr;
		this->_decoder.reset();
		this->_segment.reset();
		this->_preloadedFrames.clear();

		this->_reader = std::make_unique<mkvparser::MkvReader>();
		if (this->_reader->Open(filePath.string().c_str()) != 0) RAWRBOX_CRITICAL("Failed to open '{}'", filePath.generic_string());

		this->internalLoad();
	}

	bool WEBM::update(float deltaTime) {
		if (this->_entries.empty()) RAWRBOX_CRITICAL("Video not loaded! Did you call 'load' ?");

		if (!this->_paused && !this->_ended) {
			this->_time += static_cast<uint64_t>(static_cast<double>(deltaTime) * 1000000000.0 * static_cast<double>(this->_speed));

			const bool reachedEnd = this->_time >= this->_endTime;
			if (reachedEnd) {
				if (this->_loop) {
					this->_time %= this->_endTime;
					this->_cursor = 0;
				} else {
					this->_time = this->_endTime;
					this->_ended = true;
				}
			}

			this->present(this->_ended ? this->_entries.size() - 1 : this->findFrame(this->_time));
			if (reachedEnd) this->onEnd();
		}

		return std::exchange(this->_dirty, false);
	}

	void WEBM::reset() {
		this->_paused = false;
		this->seek(0);
	}

	void WEBM::seek(uint64_t timeMS) {
		if (this->_entries.empty()) RAWRBOX_CRITICAL("Video not loaded! Did you call 'load' ?");

		this->_time = timeMS >= this->_endTime / 1000000ULL ? this->_endTime : timeMS * 1000000ULL;
		this->_ended = false;

		const size_t target = this->findFrame(this->_time);
		if (target < this->_cursor) this->_cursor = 0;

		this->present(target);
	}

	// UTILS ------
	const rawrbox::WEBMImage& WEBM::getImage() const {
		if (this->isPreLoaded() && !this->_preloadedFrames.empty()) return this->_preloadedFrames[this->_entries[this->_shown].image];
		return this->_image;
	}

	const rawrbox::Vector2u& WEBM::getSize() const {
		return this->_info.size;
	}

	const rawrbox::WEBMInfo& WEBM::getInfo() const {
		return this->_info;
	}

	uint64_t WEBM::getTime() const {
		return this->_time / 1000000ULL;
	}

	bool WEBM::eos() const {
		return this->_ended;
	}

	bool WEBM::getLoop() const { return this->_loop; }
	void WEBM::setLoop(bool loop) {
		this->_loop = loop;
	}

	bool WEBM::getPaused() const { return this->_paused; }
	void WEBM::setPaused(bool paused) {
		this->_paused = paused;
	}

	float WEBM::getSpeed() const { return this->_speed; }
	void WEBM::setSpeed(float speed) {
		this->_speed = std::max(0.F, speed);
	}

	bool WEBM::isPreLoaded() const {
		return (this->_flags & rawrbox::WEBMLoadFlags::PRELOAD) > 0;
	}
	// -------
} // namespace rawrbox
