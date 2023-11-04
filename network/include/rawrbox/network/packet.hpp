#pragma once

#include <algorithm>
#include <bit>
#include <cstdint>
#include <cstring>
#include <map>
#include <optional>
#include <stdexcept>
#include <unordered_map>
#include <vector>

namespace rawrbox {
	class Packet;

	// CONCEPTS ----
	template <typename T>
	concept isNetworkReadable = requires(T t, rawrbox::Packet p) {
		{ t.networkRead(p) };
	};

	template <typename T>
	concept isNetworkWritable = requires(T t, rawrbox::Packet p) {
		{ t.networkWrite(p) };
	};
	// ----------------

	class Packet {
	protected:
		std::vector<uint8_t> buffer = {};
		size_t pos = 0;

	public:
		Packet() = default;
		Packet(const Packet&) = default;
		Packet(Packet&&) = default;
		Packet& operator=(const Packet&) = default;
		Packet& operator=(Packet&&) = default;
		virtual ~Packet() = default;

		// Reads ----
		template <class T>
		T read() {
			T ret;
			read(ret);
			return ret;
		}

		template <class T>
		void read(T& ret) {
			if constexpr (isNetworkReadable<T>) {
				ret.networkRead(*this);
			} else {
				static_assert(std::is_trivially_copyable_v<T>, "Fallback option for not a (vector, map, string, and does not supply a networkRead), T needs to be trivially copyable.");

				if (this->pos + sizeof(T) > this->buffer.size()) {
					throw std::runtime_error("[RawrBox-Packet] Reading past buffer");
				}

				std::memcpy(&ret, &this->buffer.at(pos), sizeof(T));
				this->pos += sizeof(const T);
			}
		}

		virtual void networkRead(rawrbox::Packet& packet);

		template <class T>
		void read(std::vector<T>& ret) {
			auto elms = this->readLength<size_t>();
			if (elms <= 0) return;

			while (elms-- > 0) {
				ret.push_back(this->read<T>());
			}
		}

		template <class T, size_t size>
		void read(std::array<T, size>& ret) {
			for (size_t i = 0; i < size; i++) {
				read<T>(ret[i]);
			}
		}

		template <class A, class B>
		void read(std::pair<A, B>& ret) {
			ret.first = this->read<A>();
			ret.second = this->read<B>();
		}

		template <class T>
		void read(std::optional<T>& ret) {
			if (this->read<bool>()) {
				ret = this->read<T>();
			} else {
				ret = std::nullopt;
			}
		}

		template <class A, class B>
		void read(std::map<A, B>& ret) {
			auto elms = this->readLength<size_t>();
			if (elms <= 0) return;

			while (elms-- > 0) {
				ret.emplace(this->read<std::pair<A, B>>());
			}
		}

		template <class A, class B>
		void read(std::unordered_map<A, B>& ret) {
			auto elms = this->readLength<size_t>();
			if (elms <= 0) return;

			while (elms-- > 0) {
				ret.insert(this->read<std::pair<A, B>>());
			}
		}

		template <class T = size_t>
		T readLength() {
			constexpr uint64_t maskNum = 0x7F;
			constexpr uint64_t maskFlag = 0x80;

			uint64_t ret = 0;
			uint64_t bitsReceived = 0;
			while (true) {
				uint64_t byte = read<uint8_t>();

				ret = ret | ((byte & maskNum) << bitsReceived);
				bitsReceived += 7;

				if ((byte & maskFlag) == 0) break;
			}

			return static_cast<T>(ret);
		}

		virtual void read(std::string& ret);
		virtual std::string readAllString();
		virtual bool readToFile(const std::string& filename);

		[[nodiscard]] const std::vector<uint8_t>& readAll();
		// ------

		// Write -----------
		virtual void networkWrite(rawrbox::Packet& packet);

		template <class T>
		void write(const T& obj) {
			if constexpr (isNetworkWritable<T>) {
				obj.networkWrite(*this);
			} else {
				static_assert(std::is_trivially_copyable_v<T>, "Fallback option for not a (vector, map, string, and does not supply a networkRead), T needs to be trivially copyable.");

				auto ptr = std::bit_cast<const uint8_t*>(&obj);
				this->buffer.insert(this->buffer.begin() + this->pos, ptr, ptr + sizeof(const T));
				this->pos += sizeof(const T);
			}
		}

		template <class T>
		void write(const std::string& obj, bool shouldWriteLength = true) {
			this->write(obj.begin(), obj.end(), shouldWriteLength);
		}

		template <class T>
		void write(const std::vector<T>& obj, bool shouldWriteLength = true) {
			this->write(obj.begin(), obj.end(), shouldWriteLength);
		}

		template <class A, class B>
		void write(const std::pair<A, B>& obj) {
			this->write(obj.first);
			this->write(obj.second);
		}

		template <class T>
		void write(const std::optional<T>& obj) {
			auto val = obj.has_value();

			this->write(val);
			if (val) this->write(obj.value());
		}

		template <class A, class B>
		void write(const std::map<A, B>& obj, bool shouldWriteLength = true) {
			this->write(obj.begin(), obj.end(), shouldWriteLength);
		}

		template <class A, class B>
		void write(const std::unordered_map<A, B>& obj, bool shouldWriteLength = true) {
			this->write(obj.begin(), obj.end(), shouldWriteLength);
		}

		template <class T, size_t size>
		void write(const std::array<T, size>& obj, bool shouldWriteLength = false) {
			this->write(obj.begin(), obj.end(), shouldWriteLength);
		}

		template <class IterType>
		void write(IterType begin, IterType end, bool shouldWriteLength = true) {
			if (shouldWriteLength) this->writeLength(std::distance(begin, end));

			std::for_each(begin, end, [this](const auto& row) {
				this->write(row);
			});
		}

		void write(const std::string& obj, bool shouldWriteLength = true);

		template <class T = size_t>
		void writeLength(T size) {
			if (size < 0) throw std::runtime_error("[RawrBox-Packet] invalid length");
			if (size == 0) {
				write<uint8_t>(0);
				return;
			}

			auto remaining = static_cast<size_t>(size);
			constexpr uint64_t maskNum = 0x7F;
			constexpr uint64_t maskFlag = 0x80;

			while (remaining > 0) {
				bool hasMore = (remaining >> 7) > 0;
				write(static_cast<uint8_t>((remaining & maskNum) | (hasMore ? maskFlag : 0x00ULL)));

				remaining >>= 7;
			}
		}
		// -----------------

		// UTILS -----
		bool seek(size_t offset);
		bool seek(std::vector<uint8_t>::iterator offset);

		[[nodiscard]] size_t tell() const;
		[[nodiscard]] size_t size() const;

		uint8_t* data();
		[[nodiscard]] const uint8_t* data() const;

		std::vector<uint8_t>& getBuffer();
		[[nodiscard]] const std::vector<uint8_t>& getBuffer() const;
		void setBuffer(std::vector<uint8_t> b);

		std::vector<uint8_t>::iterator begin();
		std::vector<uint8_t>::iterator end();
		[[nodiscard]] std::vector<uint8_t>::const_iterator cbegin() const;
		[[nodiscard]] std::vector<uint8_t>::const_iterator cend() const;

		void resize(size_t size);
		[[nodiscard]] bool empty() const;

		void clear();
		// ----------------
	};

} // namespace rawrbox
