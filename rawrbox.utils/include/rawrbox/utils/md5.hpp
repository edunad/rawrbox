#pragma once

#include <algorithm>
#include <array>
#include <bit>
#include <cstdint>
#include <cstring>
#include <string>
#include <string_view>

namespace rawrbox {
	// Based on : https://www.rfc-editor.org/rfc/rfc1321
	class MD5 {
	protected:
		static constexpr std::array<uint32_t, 64> K = {
		    0xD76AA478, 0xE8C7B756, 0x242070DB, 0xC1BDCEEE, 0xF57C0FAF, 0x4787C62A, 0xA8304613, 0xFD469501,
		    0x698098D8, 0x8B44F7AF, 0xFFFF5BB1, 0x895CD7BE, 0x6B901122, 0xFD987193, 0xA679438E, 0x49B40821,
		    0xF61E2562, 0xC040B340, 0x265E5A51, 0xE9B6C7AA, 0xD62F105D, 0x02441453, 0xD8A1E681, 0xE7D3FBC8,
		    0x21E1CDE6, 0xC33707D6, 0xF4D50D87, 0x455A14ED, 0xA9E3E905, 0xFCEFA3F8, 0x676F02D9, 0x8D2A4C8A,
		    0xFFFA3942, 0x8771F681, 0x6D9D6122, 0xFDE5380C, 0xA4BEEA44, 0x4BDECFA9, 0xF6BB4B60, 0xBEBFBC70,
		    0x289B7EC6, 0xEAA127FA, 0xD4EF3085, 0x04881D05, 0xD9D4D039, 0xE6DB99E5, 0x1FA27CF8, 0xC4AC5665,
		    0xF4292244, 0x432AFF97, 0xAB9423A7, 0xFC93A039, 0x655B59C3, 0x8F0CCC92, 0xFFEFF47D, 0x85845DD1,
		    0x6FA87E4F, 0xFE2CE6E0, 0xA3014314, 0x4E0811A1, 0xF7537E82, 0xBD3AF235, 0x2AD7D2BB, 0xEB86D391};

		static constexpr std::array<int, 64> S = {
		    7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22,
		    5, 9, 14, 20, 5, 9, 14, 20, 5, 9, 14, 20, 5, 9, 14, 20,
		    4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23,
		    6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21};

		std::array<uint32_t, 4> _state = {0x67452301, 0xEFCDAB89, 0x98BADCFE, 0x10325476};
		std::array<uint8_t, 64> _buffer = {};
		uint64_t _size = 0;

		virtual void transform(const uint8_t* block) {
			std::array<uint32_t, 16> words = {};
			for (size_t i = 0; i < words.size(); i++) {
				words[i] = static_cast<uint32_t>(block[i * 4]) | (static_cast<uint32_t>(block[i * 4 + 1]) << 8U) | (static_cast<uint32_t>(block[i * 4 + 2]) << 16U) | (static_cast<uint32_t>(block[i * 4 + 3]) << 24U);
			}

			uint32_t a = this->_state[0];
			uint32_t b = this->_state[1];
			uint32_t c = this->_state[2];
			uint32_t d = this->_state[3];

			for (uint32_t i = 0; i < 64; i++) {
				uint32_t f = 0;
				uint32_t g = 0;

				if (i < 16) {
					f = (b & c) | (~b & d);
					g = i;
				} else if (i < 32) {
					f = (d & b) | (~d & c);
					g = (5 * i + 1) % 16;
				} else if (i < 48) {
					f = b ^ c ^ d;
					g = (3 * i + 5) % 16;
				} else {
					f = c ^ (b | ~d);
					g = (7 * i) % 16;
				}

				f += a + K[i] + words[g];
				a = d;
				d = c;
				c = b;
				b += std::rotl(f, S[i]);
			}

			this->_state[0] += a;
			this->_state[1] += b;
			this->_state[2] += c;
			this->_state[3] += d;
		}

	public:
		MD5() = default;
		MD5(const MD5&) = default;
		MD5(MD5&&) = default;
		MD5& operator=(const MD5&) = default;
		MD5& operator=(MD5&&) = default;
		virtual ~MD5() = default;

		virtual void update(const uint8_t* data, size_t size) {
			size_t used = this->_size % 64;
			this->_size += size;

			if (used > 0) {
				const size_t fill = std::min(size, 64 - used);
				std::memcpy(this->_buffer.data() + used, data, fill);

				data += fill;
				size -= fill;
				used += fill;

				if (used < 64) return;
				this->transform(this->_buffer.data());
			}

			while (size >= 64) {
				this->transform(data);
				data += 64;
				size -= 64;
			}

			if (size > 0) std::memcpy(this->_buffer.data(), data, size);
		}

		[[nodiscard]] virtual std::array<uint8_t, 16> finalize() {
			const uint64_t bits = this->_size * 8;

			// PADDING ---
			const size_t used = this->_size % 64;
			const size_t padding = used < 56 ? 56 - used : 120 - used;

			constexpr std::array<uint8_t, 64> pad = {0x80};
			this->update(pad.data(), padding);
			// -----------

			// LENGTH ---
			std::array<uint8_t, 8> length = {};
			for (size_t i = 0; i < length.size(); i++) {
				length[i] = static_cast<uint8_t>(bits >> (i * 8));
			}

			this->update(length.data(), length.size());
			// -----------

			std::array<uint8_t, 16> hash = {};
			for (size_t i = 0; i < hash.size(); i++) {
				hash[i] = static_cast<uint8_t>(this->_state[i / 4] >> ((i % 4) * 8));
			}

			return hash;
		}

		[[nodiscard]] static std::array<uint8_t, 16> calculate(const uint8_t* data, size_t size) {
			rawrbox::MD5 md5 = {};
			md5.update(data, size);
			return md5.finalize();
		}

		[[nodiscard]] static std::string toString(const std::array<uint8_t, 16>& hash) {
			constexpr std::string_view HEX = "0123456789abcdef";

			std::string str = {};
			str.reserve(32);

			for (const auto byte : hash) {
				str += HEX[byte >> 4U];
				str += HEX[byte & 0x0FU];
			}

			return str;
		}
	};
} // namespace rawrbox