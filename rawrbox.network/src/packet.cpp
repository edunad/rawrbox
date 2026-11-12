#include <rawrbox/network/packet.hpp>

#include <fstream>
#include <iostream>
#include <utility>

namespace rawrbox {
	// Read ----
	void Packet::networkRead(rawrbox::Packet& packet) {
		packet.read(this->buffer);
	}

	void Packet::read(std::string& ret) {
		auto len = this->readLength<size_t>();
		auto start = this->buffer.begin() + this->pos;

		ret.assign(start, start + len);
		this->pos += len;
	}

	std::string Packet::readAllString() {
		auto len = this->size() - this->pos;
		this->pos += len;
		return {this->buffer.end() - len, this->buffer.end()};
	}

	bool Packet::readToFile(const std::string& filename) {
		std::fstream file(filename, std::ios::out | std::ios::binary | std::ios::trunc);
		if (!file.is_open()) return false;

		file.write(std::bit_cast<char*>(this->buffer.data()) + this->pos, this->size() - this->pos);
		file.close();

		return true;
	}

	const std::vector<uint8_t>& Packet::readAll() {
		this->pos = this->size();
		return this->buffer;
	}
	// --------

	// Write -----------
	void Packet::networkWrite(rawrbox::Packet& packet) {
		packet.write(this->buffer);
	}

	void Packet::write(const std::string& obj, bool shouldWriteLength) {
		this->write(obj.begin(), obj.end(), shouldWriteLength);
	}
	// --------

	// UTILS -----
	bool Packet::seek(size_t offset) {
		if (offset > this->size()) return false;
		this->pos = offset;
		return true;
	}

	bool Packet::seek(const std::vector<uint8_t>::iterator& offset) {
		if (offset > this->end()) return false;
		this->pos = std::distance(this->buffer.begin(), offset);
		return true;
	}

	size_t Packet::tell() const { return this->pos; }
	size_t Packet::size() const { return this->buffer.size(); }

	uint8_t* Packet::data() { return this->buffer.data(); }
	const uint8_t* Packet::data() const { return this->buffer.data(); }

	std::vector<uint8_t>& Packet::getBuffer() { return this->buffer; }
	const std::vector<uint8_t>& Packet::getBuffer() const { return this->buffer; }
	void Packet::setBuffer(std::vector<uint8_t> b) { this->buffer = std::move(b); }

	std::vector<uint8_t>::iterator Packet::begin() { return this->buffer.begin(); }
	std::vector<uint8_t>::iterator Packet::end() { return this->buffer.end(); }
	std::vector<uint8_t>::const_iterator Packet::cbegin() const { return this->buffer.cbegin(); }
	std::vector<uint8_t>::const_iterator Packet::cend() const { return this->buffer.cend(); }

	void Packet::resize(size_t size) { this->buffer.resize(size); }
	bool Packet::empty() const { return this->buffer.empty(); }

	void Packet::clear() {
		this->buffer.clear();
		this->pos = 0;
	}
	// ----------------
} // namespace rawrbox
