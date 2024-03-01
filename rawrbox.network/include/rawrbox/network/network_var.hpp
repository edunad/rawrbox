
#pragma once

#include <rawrbox/network/network_array.hpp>
#include <rawrbox/utils/crc.hpp>

#include <functional>
#include <type_traits>
#include <vector>

template <typename T>
struct is_vector : public std::false_type {};
template <typename T>
struct is_map : public std::false_type {};
template <typename T>
struct is_unorderedmap : public std::false_type {};

template <typename T, typename A>
struct is_vector<std::vector<T, A>> : public std::true_type {};
template <typename T, typename A>
struct is_map<std::map<T, A>> : public std::true_type {};
template <typename T, typename A>
struct is_unorderedmap<std::unordered_map<T, A>> : public std::true_type {};

template <typename T>
struct is_deltaUMap : public std::false_type {};
template <typename T>
struct is_deltaMap : public std::false_type {};
template <typename T>
struct is_deltaVector : public std::false_type {};

template <typename T, typename A>
struct is_deltaUMap<rawrbox::UMapDelta<T, A>> : public std::true_type {};
template <typename T, typename A>
struct is_deltaMap<rawrbox::MapDelta<T, A>> : public std::true_type {};
template <typename T>
struct is_deltaVector<rawrbox::VectorDelta<T>> : public std::true_type {};

namespace rawrbox {
	template <typename T>
	struct NetVar {
	protected:
		rawrbox::Packet _cache = {};

		uint32_t _crc = 0;
		bool _initialized = false;

		T _val;

	public:
		std::function<void()> onNetBeforeUpdate = nullptr;
		std::function<void()> onNetUpdate = nullptr;
		std::function<void()> onUpdate = nullptr;

		NetVar() { this->set({}); }
		NetVar(const NetVar&) = delete;
		NetVar(NetVar&&) = delete;
		NetVar& operator=(NetVar&&) = delete;
		NetVar(const T& a) { this->set(a); }
		~NetVar() = default;

		NetVar<T>& operator=(const NetVar<T>& a) {
			this->set(a.get());
			return *this;
		}

		NetVar<T>& operator=(const T& a) {
			this->set(a);
			return *this;
		}

		T& get() { return this->_val; }
		[[nodiscard]] T get() const { return this->_val; }

		bool set(const T& a, bool crcGen = true) {
			if (this->_initialized) {
				if constexpr (is_vector<T>::value) {
					if (a.size() == _val.size() && std::equal(a.begin(), a.end(), this->_val.begin())) return false;
				} else if constexpr (is_map<T>::value) {
					// TODO: Handle map
				} else if constexpr (is_unorderedmap<T>::value) {
					// TODO: Handle map
				} else if constexpr (is_deltaVector<T>::value || is_deltaMap<T>::value || is_deltaUMap<T>::value) {
					// TODO: Handle delta
				} else {
					if (a == this->_val) return false;
				}
			}

			this->_val = a; // Set new val
			this->_initialized = true;

			if (crcGen) this->update(); // Client does not need CRC / buffer data
			if (this->onUpdate != nullptr) this->onUpdate();

			return true;
		}

		// UTILS ---
		[[nodiscard]] bool isInitialized() const { return _initialized; }
		bool isDirty(uint32_t crc) { return crc == this->_crc; }
		[[nodiscard]] uint32_t getCRC() const { return this->_crc; }

		void update() {
			this->_cache.clear();
			this->_cache.write(_val);
			this->_crc = CRC::Calculate(this->_cache.data(), this->_cache.size() * sizeof(uint8_t), CRC::CRC_32()); // Instead of CRC should we just increase an int
		}
		// --------

		// NETWORKING ---
		void networkRead(rawrbox::Packet& packet) {
			bool hasData = packet.read<bool>();
			if (!hasData) return;

			if (this->_initialized && this->onNetBeforeUpdate != nullptr) this->onNetBeforeUpdate();

			if constexpr (is_deltaVector<T>::value || is_deltaMap<T>::value || is_deltaUMap<T>::value) {
				this->_val.read(packet);
				this->_initialized = true;
			} else {
				this->set(packet.read<T>(), false);
			}

			if (this->onNetUpdate != nullptr) this->onNetUpdate();
		}

		void networkWrite(rawrbox::Packet& packet) const {
			// write it all to the packet
			packet.write(this->_crc);
			packet.write(this->_cache.size());
			packet.write(this->_cache.getBuffer(), false);
		}
		// ---------
	};
} // namespace rawrbox
