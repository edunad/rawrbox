
#pragma once

#include <rawrbox/network/packet.hpp>

#include <map>
#include <optional>
#include <unordered_map>
#include <vector>

namespace rawrbox {
	template <typename KEY, typename VAL>
	using DeltaData = std::vector<std::pair<KEY, std::optional<VAL>>>;

	template <typename T>
	using DeltaChangelog = std::vector<std::pair<bool, T>>;

	template <typename T>
	struct VectorDelta : private std::vector<T> {

	public:
		DeltaChangelog<size_t> changelog; // added?, changed index

		using std::vector<T>::vector;
		using std::vector<T>::at;
		using std::vector<T>::clear;
		using typename std::vector<T>::iterator;
		using typename std::vector<T>::const_iterator;
		using std::vector<T>::begin;
		using std::vector<T>::end;
		using std::vector<T>::cbegin;
		using std::vector<T>::cend;
		using std::vector<T>::crbegin;
		using std::vector<T>::crend;
		using std::vector<T>::empty;
		using std::vector<T>::size;
		using std::vector<T>::operator[];
		using std::vector<T>::front;
		using std::vector<T>::back;
		using std::vector<T>::resize;

		std::vector<T>::const_iterator find(const T& key) const { return std::find(begin(), end(), key); }
		bool operator==(const VectorDelta<T>& other) const { return other.size() == size() && std::equal(other.begin(), other.end(), begin()); }
		bool operator!=(const VectorDelta<T>& other) { return !operator==(other); }

		void push_back(const T& a, bool track = true) {
			std::vector<T>::push_back(a);
			if (track) this->changelog.push_back({true, size() - 1});
		}

		void insert(std::vector<T>::const_iterator index, const T& a, bool track = true) {
			if (track) this->changelog.push_back({true, std::distance<typename std::vector<T>::const_iterator>(cbegin(), index)});
			std::vector<T>::insert(index, a);
		}

		DeltaData<size_t, T> calculate() {
			DeltaData<size_t, T> diff;
			for (auto& indx : this->changelog) {
				if (indx.first)
					diff.push_back({indx.second, at(indx.second)});
				else
					diff.push_back({indx.second, std::nullopt});
			}

			this->changelog.clear();
			return diff;
		}

		void reserve(size_t size) {
			std::vector<T>::reserve(size);
			this->changelog.reserve(size);
		}

		void erase(std::vector<T>::const_iterator index, bool track = true) {
			if (track) this->changelog.push_back({false, std::distance<typename std::vector<T>::const_iterator>(this->cbegin(), index)});
			std::vector<T>::erase(index);
		}

		void read(rawrbox::Packet& packet) {
			for (auto& change : packet.read<DeltaData<size_t, T>>()) {
				if (change.second.has_value())
					std::vector<T>::insert(cbegin() + change.first, change.second.value(), false);
				else
					std::vector<T>::erase(cbegin() + change.first, false);
			}
		}

		// NETWORKING ---
		void networkWrite(rawrbox::Packet& packet) const {
			// NOLINTBEGIN(cppcoreguidelines-pro-type-const-cast)
			packet.write(const_cast<VectorDelta<T>*>(this)->calculate()); // ewww
										      // NOLINTEND(cppcoreguidelines-pro-type-const-cast)
		}
		// ---------
	};

	template <typename KEY, typename VAL>
	struct MapDelta : private std::map<KEY, VAL> {
	public:
		DeltaChangelog<KEY> changelog; // added?, changed index

		using std::map<KEY, VAL>::map;
		using std::map<KEY, VAL>::at;
		using std::map<KEY, VAL>::clear;
		using typename std::map<KEY, VAL>::iterator;
		using typename std::map<KEY, VAL>::const_iterator;
		using std::map<KEY, VAL>::find;
		using std::map<KEY, VAL>::begin;
		using std::map<KEY, VAL>::end;
		using std::map<KEY, VAL>::cbegin;
		using std::map<KEY, VAL>::cend;
		using std::map<KEY, VAL>::empty;
		using std::map<KEY, VAL>::size;
		using std::map<KEY, VAL>::insert_or_assign;

		// TODO ?
		/*bool operator==(const VectorDelta<T>& other) const { return other.size() == size() && std::equal(other.begin(), other.end(), begin()); }
		bool operator !=(const VectorDelta<T>& other) { return !operator==(other); }*/

		VAL& operator[](const KEY& key) {
			this->changelog.push_back({true, key});
			return std::map<KEY, VAL>::operator[](key);
		}

		void erase(const KEY& key, bool record = true) {
			if (record) this->changelog.push_back({false, key});
			std::map<KEY, VAL>::erase(key);
		}

		DeltaData<KEY, VAL> calculate() {
			DeltaData<KEY, VAL> diff;
			for (auto& indx : changelog) {
				if (indx.first)
					diff.push_back({indx.second, at(indx.second)});
				else
					diff.push_back({indx.second, std::nullopt});
			}

			this->changelog.clear();
			return diff;
		}

		void read(rawrbox::Packet& packet) {
			for (auto& change : packet.read<DeltaData<KEY, VAL>>()) {
				if (change.second.has_value())
					insert_or_assign(change.first, change.second.value());
				else
					erase(change.first, false);
			}
		}

		// NETWORKING ---
		void networkWrite(rawrbox::Packet& packet) const {
			// NOLINTBEGIN(cppcoreguidelines-pro-type-const-cast)
			packet.write(const_cast<MapDelta<KEY, VAL>*>(this)->calculate()); // ewww
											  // NOLINTEND(cppcoreguidelines-pro-type-const-cast)
		}
		// ---------
	};

	template <typename KEY, typename VAL>
	struct UMapDelta : private std::unordered_map<KEY, VAL> {
	public:
		DeltaChangelog<KEY> changelog; // added?, changed index

		using std::unordered_map<KEY, VAL>::unordered_map;
		using std::unordered_map<KEY, VAL>::at;
		using std::unordered_map<KEY, VAL>::clear;
		using typename std::unordered_map<KEY, VAL>::iterator;
		using typename std::unordered_map<KEY, VAL>::const_iterator;
		using std::unordered_map<KEY, VAL>::find;
		using std::unordered_map<KEY, VAL>::begin;
		using std::unordered_map<KEY, VAL>::end;
		using std::unordered_map<KEY, VAL>::cbegin;
		using std::unordered_map<KEY, VAL>::cend;
		using std::unordered_map<KEY, VAL>::empty;
		using std::unordered_map<KEY, VAL>::size;
		using std::unordered_map<KEY, VAL>::insert_or_assign;

		// TODO ?
		/*bool operator==(const VectorDelta<T>& other) const { return other.size() == size() && std::equal(other.begin(), other.end(), begin()); }
		bool operator !=(const VectorDelta<T>& other) { return !operator==(other); }*/
		VAL& operator[](const KEY& key) {
			this->changelog.push_back({true, key});
			return std::unordered_map<KEY, VAL>::operator[](key);
		}

		void erase(const KEY& key, bool record = true) {
			if (record) this->changelog.push_back({false, key});
			std::unordered_map<KEY, VAL>::erase(key);
		}

		DeltaData<KEY, VAL> calculate() {
			DeltaData<KEY, VAL> diff;
			for (auto& indx : this->changelog) {
				if (indx.first)
					diff.push_back({indx.second, at(indx.second)});
				else
					diff.push_back({indx.second, std::nullopt});
			}

			this->changelog.clear();
			return diff;
		}

		void read(rawrbox::Packet& packet) {
			for (auto& change : packet.read<DeltaData<KEY, VAL>>()) {
				if (change.second.has_value())
					insert_or_assign(change.first, change.second.value());
				else
					erase(change.first, false);
			}
		}

		// NETWORKING ---
		void networkWrite(rawrbox::Packet& packet) const {
			// NOLINTBEGIN(cppcoreguidelines-pro-type-const-cast)
			packet.write(const_cast<UMapDelta<KEY, VAL>*>(this)->calculate()); // ewww
											   // NOLINTEND(cppcoreguidelines-pro-type-const-cast)
		}
		// ---------
	};
} // namespace rawrbox
