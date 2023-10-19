#include <rawrbox/render/utils/topology.hpp>

// Adapted from https://github.com/bkaradzic/bgfx/blob/master/src/topology.cpp && https://github.com/bkaradzic/bx/blob/97332257ff86e7c35203b9d34a337d1cbb4466bc/include/bx/inline/sort.inl#L126
namespace rawrbox {
	namespace radix_sort_detail {
		constexpr uint32_t kBits = 11;
		constexpr uint32_t kHistogramSize = 1 << kBits;
		constexpr uint32_t kBitMask = kHistogramSize - 1;

	} // namespace radix_sort_detail

	inline void radixSort(uint32_t* _keys, uint32_t* _tempKeys, uint32_t _size) {
		uint32_t* keys = _keys;
		uint32_t* tempKeys = _tempKeys;

		uint32_t histogram[radix_sort_detail::kHistogramSize];
		uint16_t shift = 0;
		uint32_t pass = 0;
		for (; pass < 3; ++pass) {
			std::memset(histogram, 0, sizeof(uint32_t) * radix_sort_detail::kHistogramSize);

			bool sorted = true;
			{
				uint32_t key = keys[0];
				uint32_t prevKey = key;
				for (uint32_t ii = 0; ii < _size; ++ii, prevKey = key) {
					key = keys[ii];
					uint16_t index = (key >> shift) & radix_sort_detail::kBitMask;
					++histogram[index];
					sorted &= prevKey <= key;
				}
			}

			if (sorted) {
				goto done;
			}

			uint32_t offset = 0;
			for (unsigned int& ii : histogram) {
				uint32_t count = ii;
				ii = offset;
				offset += count;
			}

			for (uint32_t ii = 0; ii < _size; ++ii) {
				uint32_t key = keys[ii];
				uint16_t index = (key >> shift) & radix_sort_detail::kBitMask;
				uint32_t dest = histogram[index]++;
				tempKeys[dest] = key;
			}

			uint32_t* swapKeys = tempKeys;
			tempKeys = keys;
			keys = swapKeys;

			shift += radix_sort_detail::kBits;
		}

	done:
		if (0 != (pass & 1)) {
			// Odd number of passes needs to do copy to the destination.
			std::memcpy(_keys, _tempKeys, _size * sizeof(uint32_t));
		}
	}

	inline void radixSort(uint64_t* _keys, uint64_t* _tempKeys, uint32_t _size) {
		uint64_t* keys = _keys;
		uint64_t* tempKeys = _tempKeys;

		uint32_t histogram[radix_sort_detail::kHistogramSize];
		uint16_t shift = 0;
		uint32_t pass = 0;
		for (; pass < 6; ++pass) {
			std::memset(histogram, 0, sizeof(uint32_t) * radix_sort_detail::kHistogramSize);

			bool sorted = true;
			{
				uint64_t key = keys[0];
				uint64_t prevKey = key;
				for (uint32_t ii = 0; ii < _size; ++ii, prevKey = key) {
					key = keys[ii];
					uint16_t index = (key >> shift) & radix_sort_detail::kBitMask;
					++histogram[index];
					sorted &= prevKey <= key;
				}
			}

			if (sorted) {
				goto done;
			}

			uint32_t offset = 0;
			for (unsigned int& ii : histogram) {
				uint32_t count = ii;
				ii = offset;
				offset += count;
			}

			for (uint32_t ii = 0; ii < _size; ++ii) {
				uint64_t key = keys[ii];
				uint16_t index = (key >> shift) & radix_sort_detail::kBitMask;
				uint32_t dest = histogram[index]++;
				tempKeys[dest] = key;
			}

			uint64_t* swapKeys = tempKeys;
			tempKeys = keys;
			keys = swapKeys;

			shift += radix_sort_detail::kBits;
		}

	done:
		if (0 != (pass & 1)) {
			// Odd number of passes needs to do copy to the destination.
			std::memcpy(_keys, _tempKeys, _size * sizeof(uint64_t));
		}
	}

	template <typename IndexT, typename SortT>
	static uint32_t topologyConvertTriListToLineList(void* _dst, uint32_t _dstSize, const IndexT* _indices, uint32_t _numIndices, IndexT* _temp, SortT* _tempSort) {
		// Create all line pairs and sort indices.
		IndexT* dst = _temp;
		for (uint32_t ii = 0; ii < _numIndices; ii += 3) {
			const IndexT* tri = &_indices[ii];
			IndexT i0 = tri[0], i1 = tri[1], i2 = tri[2];

			if (i0 > i1) {
				std::swap(i0, i1);
			}

			if (i1 > i2) {
				std::swap(i1, i2);
			}

			if (i0 > i1) {
				std::swap(i0, i1);
			}

			// BX_ASSERT(i0 < i1 && i1 < i2, "");

			dst[1] = i0;
			dst[0] = i1;
			dst[3] = i1;
			dst[2] = i2;
			dst[5] = i0;
			dst[4] = i2;
			dst += 6;
		}

		// Sort all line pairs.
		auto* sorted = (SortT*)_temp;
		radixSort(sorted, _tempSort, _numIndices);

		uint32_t num = 0;

		// Remove all line pair duplicates.
		if (_dst == nullptr) {
			SortT last = sorted[0];
			for (uint32_t ii = 1; ii < _numIndices; ++ii) {
				if (last != sorted[ii]) {
					num += 2;
					last = sorted[ii];
				}
			}
			num += 2;
		} else {
			dst = (IndexT*)_dst;
			IndexT* end = &dst[_dstSize / sizeof(IndexT)];
			SortT last = sorted[0];

			{
				union Un {
					SortT key;
					struct {
						IndexT i0;
						IndexT i1;
					} u16;
				} un = {sorted[0]};
				dst[0] = un.u16.i0;
				dst[1] = un.u16.i1;
				dst += 2;
			}

			for (uint32_t ii = 1; ii < _numIndices && dst < end; ++ii) {
				if (last != sorted[ii]) {
					union Un {
						SortT key;
						struct {
							IndexT i0;
							IndexT i1;
						} u16;
					} un = {sorted[ii]};
					dst[0] = un.u16.i0;
					dst[1] = un.u16.i1;
					dst += 2;
					last = sorted[ii];
				}
			}

			num = uint32_t(dst - (IndexT*)_dst);
		}

		return num;
	}

	template <typename IndexT, typename SortT>
	static uint32_t topologyConvertTriListToLineList(void* _dst, uint32_t _dstSize, const IndexT* _indices, uint32_t _numIndices) {
		auto* temp = (IndexT*)malloc(_numIndices * 2 * sizeof(IndexT) * 2);
		auto* tempSort = (SortT*)&temp[_numIndices * 2];
		uint32_t num = topologyConvertTriListToLineList(_dst, _dstSize, _indices, _numIndices, temp, tempSort);
		delete temp;

		/*IndexT* temp = (IndexT*)bx::alloc(_allocator, _numIndices * 2 * sizeof(IndexT) * 2);

		auto* tempSort = (SortT*)&temp[_numIndices * 2];
		uint32_t num = topologyConvertTriListToLineList(_dst, _dstSize, _indices, _numIndices, temp, tempSort);
		bx::free(_allocator, temp);*/
		return num;
	}

	uint32_t TopologyUtils::triToLine(void* dest, uint32_t destSize, const void* indc, uint32_t indcSize, bool _index32) {
		if (_index32) {
			return topologyConvertTriListToLineList<uint32_t, uint64_t>(dest, destSize, (const uint32_t*)indc, indcSize);
		}

		return topologyConvertTriListToLineList<uint16_t, uint32_t>(dest, destSize, (const uint16_t*)indc, indcSize);
	}
} // namespace rawrbox
