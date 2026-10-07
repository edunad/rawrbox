#pragma once

#include <algorithm>
#include <cstdint>
#include <vector>

namespace rawrbox {
	// Mostly used for tracking GPU dirtyranges for small updates
	class DirtyRanges {
	public:
		struct Range {
			uint64_t offset = 0;
			uint64_t size = 0;

			[[nodiscard]] uint64_t end() const {
				return this->offset + this->size;
			}

			[[nodiscard]] bool operator==(const Range& other) const = default;
		};

	private:
		std::vector<Range> _ranges = {};
		bool _full = false;

	public:
		static constexpr uint64_t MERGE_GAP = 4096;
		static constexpr size_t MAX_RANGES = 16;

		void push(uint64_t offset, uint64_t size) {
			if (size == 0 || this->_full) return;
			this->_ranges.push_back({offset, size});
		}

		void markFull() {
			this->_full = true;
			this->_ranges.clear();
		}

		void clear() {
			this->_full = false;
			this->_ranges.clear();
		}

		[[nodiscard]] bool empty() const { return !this->_full && this->_ranges.empty(); }
		[[nodiscard]] bool full() const { return this->_full; }

		[[nodiscard]] const std::vector<Range>& merge(uint64_t bufferSize) {
			if (this->_full) return this->_ranges;

			// Clamp to the buffer (ranges may predate a shrink)
			for (auto& range : this->_ranges) {
				if (range.offset >= bufferSize) {
					range.size = 0;
					continue;
				}

				range.size = std::min(range.size, bufferSize - range.offset);
			}

			std::erase_if(this->_ranges, [](const Range& r) { return r.size == 0; });
			if (this->_ranges.empty()) return this->_ranges;

			std::ranges::sort(this->_ranges, [](const Range& a, const Range& b) { return a.offset < b.offset; });

			// Merge overlapping / near ranges
			std::vector<Range> merged = {};
			merged.push_back(this->_ranges.front());

			for (size_t i = 1; i < this->_ranges.size(); i++) {
				const auto& range = this->_ranges[i];
				auto& last = merged.back();

				if (range.offset <= last.end() + MERGE_GAP) {
					last.size = std::max(last.end(), range.end()) - last.offset;
				} else {
					merged.push_back(range);
				}
			}

			this->_ranges = std::move(merged);

			uint64_t total = 0;
			for (const auto& range : this->_ranges) {
				total += range.size;
			}

			if (this->_ranges.size() > MAX_RANGES || (bufferSize > 0 && total * 2 >= bufferSize)) {
				this->markFull();
			}

			return this->_ranges;
		}
	};
} // namespace rawrbox
