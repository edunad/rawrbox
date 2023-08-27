#pragma once

#include <algorithm>
#include <bit>
#include <cstdint>
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

	enum class LengthType {
		UInt8,
		UInt16,
		UInt32,
		UInt64
	};

	class Packet {
	protected:
		std::vector<uint8_t> buffer = {};
		size_t pos = 0;
		LengthType lengthFormat = LengthType::UInt16;

	public:
		Packet() = default;
		Packet(const Packet&) = default;
		Packet(Packet&&) = delete;
		Packet& operator=(const Packet&) = default;
		Packet& operator=(Packet&&) = delete;
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

		template <class T>
		T readLength() {
			switch (this->lengthFormat) {
				case LengthType::UInt8: return static_cast<T>(this->read<uint8_t>());
				case LengthType::UInt16: return static_cast<T>(this->read<uint16_t>());
				case LengthType::UInt32: return static_cast<T>(this->read<uint32_t>());
				case LengthType::UInt64: return static_cast<T>(this->read<uint64_t>());
				default: throw std::runtime_error("[RawrBox-Packet] Unknown lengthFormat");
			}
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

		template <class T>
		void writeLength(T size) {
			switch (this->lengthFormat) {
				case LengthType::UInt8: this->write(static_cast<uint8_t>(size)); break;
				case LengthType::UInt16: this->write(static_cast<uint16_t>(size)); break;
				case LengthType::UInt32: this->write(static_cast<uint32_t>(size)); break;
				case LengthType::UInt64: this->write(static_cast<uint64_t>(size)); break;
				default: throw std::runtime_error("[RawrBox-Packet] Unknown lengthFormat");
			}
		}
		// -----------------

		// UTILS -----
		void setlengthFormat(LengthType format);

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
