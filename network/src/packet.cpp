#include <rawrbox/network/packet.hpp>

#include <fstream>
#include <iostream>

namespace rawrbox {
	// Read ----
	void Packet::networkRead(rawrbox::Packet& packet) {
		packet.read(buffer);
	}

	void Packet::read(std::string& ret) {
		auto len = this->readLength<size_t>();
		auto start = buffer.begin() + pos;

		ret.assign(start, start + len);
		pos += len;
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
		return buffer;
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
		if (offset > size()) return false;
		this->pos = offset;
		return true;
	}

	bool Packet::seek(std::vector<uint8_t>::iterator offset) {
		if (offset > end()) return false;
		this->pos = std::distance(buffer.begin(), offset);
		return true;
	}

	size_t Packet::tell() const { return pos; }
	size_t Packet::size() const { return buffer.size(); }

	uint8_t* Packet::data() { return buffer.data(); }
	const uint8_t* Packet::data() const { return buffer.data(); }

	std::vector<uint8_t>& Packet::getBuffer() { return buffer; }
	const std::vector<uint8_t>& Packet::getBuffer() const { return buffer; }
	void Packet::setBuffer(std::vector<uint8_t> b) { buffer = b; }

	std::vector<uint8_t>::iterator Packet::begin() { return buffer.begin(); }
	std::vector<uint8_t>::iterator Packet::end() { return buffer.end(); }
	std::vector<uint8_t>::const_iterator Packet::cbegin() const { return buffer.cbegin(); }
	std::vector<uint8_t>::const_iterator Packet::cend() const { return buffer.cend(); }

	void Packet::resize(size_t size) { buffer.resize(size); }
	bool Packet::empty() const { return buffer.empty(); }

	void Packet::clear() {
		buffer.clear();
		pos = 0;
	}
	// ----------------
} // namespace rawrbox
