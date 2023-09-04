#include <rawrbox/network/utils/zlib.hpp>

#include <zlib.h>

#include <bit>

namespace rawrbox {
	std::vector<uint8_t> ZLib::decode(std::vector<uint8_t>::const_iterator begin, std::vector<uint8_t>::const_iterator end) {
		z_stream infstream;
		infstream.zalloc = nullptr;
		infstream.zfree = nullptr;
		infstream.opaque = nullptr;

		std::vector<uint8_t> decoded;

		size_t uncompressedSize = std::distance(begin, end) * 2;
		if (uncompressedSize < 1024) uncompressedSize = 1024;

		decoded.resize(uncompressedSize);

		infstream.avail_in = static_cast<uInt>(std::distance(begin, end));
		infstream.avail_out = static_cast<uInt>(decoded.size());
		infstream.next_in = std::bit_cast<uint8_t*>(&*begin);
		infstream.next_out = decoded.data();

		inflateInit(&infstream);

		while (infstream.avail_in > 0) {
			if (infstream.avail_out == 0) {
				size_t growth = uncompressedSize / 2;
				uncompressedSize += growth;

				decoded.resize(uncompressedSize);
				infstream.avail_out = static_cast<uInt>(growth);
				infstream.next_out = decoded.data() + infstream.total_out;
			}

			switch (inflate(&infstream, Z_NO_FLUSH)) {
				case Z_OK: break;
				case Z_STREAM_END: break;

				default:
					{
						decoded = {};
						infstream.avail_in = 0;
						break;
					}
			}
		}

		if (inflateEnd(&infstream) != Z_OK) return {};

		if (infstream.total_out != decoded.size()) decoded.resize(infstream.total_out);
		return decoded;
	}

	std::vector<uint8_t> ZLib::encode(std::vector<uint8_t>::const_iterator begin, std::vector<uint8_t>::const_iterator end) {
		z_stream defstream;
		defstream.zalloc = Z_NULL;
		defstream.zfree = Z_NULL;
		defstream.opaque = Z_NULL;

		size_t compressedSize = std::distance(begin, end);
		if (compressedSize < 1024) compressedSize = 1024;

		std::vector<uint8_t> encoded;
		encoded.resize(compressedSize);

		defstream.avail_in = static_cast<uInt>(std::distance(begin, end));
		defstream.next_in = std::bit_cast<uint8_t*>(&*begin);
		defstream.avail_out = static_cast<uInt>(encoded.size());
		defstream.next_out = encoded.data();

		deflateInit(&defstream, Z_BEST_COMPRESSION);

		bool eof = false;
		while (!eof) {
			if (defstream.avail_out == 0) {
				size_t growth = compressedSize / 2;
				compressedSize += growth;

				encoded.resize(compressedSize);
				defstream.avail_out = static_cast<uInt>(growth);
				defstream.next_out = encoded.data() + defstream.total_out;
			}

			switch (deflate(&defstream, Z_FINISH)) {
				case Z_OK: break;
				case Z_STREAM_END: eof = true; break;

				default:
					{
						encoded = {};
						break;
					}
			}
		}

		if (deflateEnd(&defstream) != Z_OK) return {};
		if (defstream.total_out != encoded.size()) encoded.resize(defstream.total_out);
		return encoded;
	}
} // namespace rawrbox
