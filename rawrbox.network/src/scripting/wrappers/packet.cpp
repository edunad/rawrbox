
#include <rawrbox/network/scripting/wrappers/packet.hpp>

#include <utility>

namespace rawrbox {
	PacketWrapper::PacketWrapper(rawrbox::Packet packet) : data(std::move(packet)) {}

	// UTILS -----
	void PacketWrapper::seek(size_t offset) { this->data.seek(offset - 1); }
	void PacketWrapper::clear() { this->data.clear(); }
	size_t PacketWrapper::tell() { return this->data.tell() + 1; }
	size_t PacketWrapper::size() { return this->data.size(); }
	bool PacketWrapper::empty() { return this->data.empty(); }
	// -------------------

	// WRITE ---
	void PacketWrapper::writeBool(bool val) { this->data.write(val); }
	void PacketWrapper::writeByte(uint8_t val) { this->data.write(val); }
	void PacketWrapper::writeChar(char val) { this->data.write(val); }
	void PacketWrapper::writeShort(int16_t val) { this->data.write(val); }
	void PacketWrapper::writeUShort(uint16_t val) { this->data.write(val); }
	void PacketWrapper::writeInt(int32_t val) { this->data.write(val); }
	void PacketWrapper::writeUInt(uint32_t val) { this->data.write(val); }
	void PacketWrapper::writeLong(int64_t val) { this->data.write(val); }
	void PacketWrapper::writeULong(uint64_t val) { this->data.write(val); }
	void PacketWrapper::writeFloat(float val) { this->data.write(val); }
	void PacketWrapper::writeDouble(double val) { this->data.write(val); }
	void PacketWrapper::writeString(const std::string& val) { this->data.write(val); }
	void PacketWrapper::writeTable(const luabridge::LuaRef& val) {
		const auto json = rawrbox::LuaUtils::toJSON(val);
		if (!json) throw std::runtime_error(fmt::format("Failed to write table ──> {}", json.error()));

		std::string output;

		auto ec = glz::write<glz::opts{.prettify = false}>(json.value(), output);
		if (ec) throw std::runtime_error(fmt::format("Failed to write table"));

		this->data.write(output);
	}
	// -------

	// READ ----
	bool PacketWrapper::readBool() { return this->data.read<bool>(); }
	uint8_t PacketWrapper::readByte() { return this->data.read<uint8_t>(); }
	char PacketWrapper::readChar() { return this->data.read<char>(); }
	int16_t PacketWrapper::readShort() { return this->data.read<int16_t>(); }
	uint16_t PacketWrapper::readUShort() { return this->data.read<uint16_t>(); }
	int32_t PacketWrapper::readInt() { return this->data.read<int32_t>(); }
	uint32_t PacketWrapper::readUInt() { return this->data.read<uint32_t>(); }
	int64_t PacketWrapper::readLong() { return this->data.read<int64_t>(); }
	uint64_t PacketWrapper::readULong() { return this->data.read<uint64_t>(); }
	float PacketWrapper::readFloat() { return this->data.read<float>(); }
	double PacketWrapper::readDouble() { return this->data.read<double>(); }
	std::string PacketWrapper::readString() { return this->data.read<std::string>(); }

	luabridge::LuaRef PacketWrapper::readTable(lua_State* L) {
		glz::generic json = {};

		auto err = glz::read_json(json, this->data.read<std::string>());
		if (err != glz::error_code::none) throw std::runtime_error("Invalid table format");

		auto table = rawrbox::LuaUtils::fromJSON(L, json);
		if (!table) throw std::runtime_error(fmt::format("Invalid table ──> {}", table.error()));

		return std::move(table.value());
	}
	// ----------

	void PacketWrapper::registerLua(lua_State* L) {

		luabridge::getGlobalNamespace(L)
		    .beginClass<rawrbox::PacketWrapper>("Packet")
		    .addConstructor<void(), void(rawrbox::Packet)>()

		    // UTILS ----
		    .addFunction("seek", &PacketWrapper::seek)
		    .addFunction("clear", &PacketWrapper::clear)
		    .addFunction("tell", &PacketWrapper::tell)
		    .addFunction("size", &PacketWrapper::size)
		    .addFunction("empty", &PacketWrapper::empty)
		    // ----

		    // WRITE ---
		    .addFunction("writeBool", &PacketWrapper::writeBool)
		    .addFunction("writeByte", &PacketWrapper::writeByte)
		    .addFunction("writeChar", &PacketWrapper::writeChar)
		    .addFunction("writeShort", &PacketWrapper::writeShort)
		    .addFunction("writeUShort", &PacketWrapper::writeUShort)
		    .addFunction("writeInt", &PacketWrapper::writeInt)
		    .addFunction("writeUInt", &PacketWrapper::writeUInt)
		    .addFunction("writeLong", &PacketWrapper::writeLong)
		    .addFunction("writeULong", &PacketWrapper::writeULong)
		    .addFunction("writeFloat", &PacketWrapper::writeFloat)
		    .addFunction("writeDouble", &PacketWrapper::writeDouble)
		    .addFunction("writeString", &PacketWrapper::writeString)
		    .addFunction("writeTable", &PacketWrapper::writeTable)
		    // ----

		    // READ ---
		    .addFunction("readBool", &PacketWrapper::readBool)
		    .addFunction("readByte", &PacketWrapper::readByte)
		    .addFunction("readChar", &PacketWrapper::readChar)
		    .addFunction("readShort", &PacketWrapper::readShort)
		    .addFunction("readUShort", &PacketWrapper::readUShort)
		    .addFunction("readInt", &PacketWrapper::readInt)
		    .addFunction("readUInt", &PacketWrapper::readUInt)
		    .addFunction("readLong", &PacketWrapper::readLong)
		    .addFunction("readULong", &PacketWrapper::readULong)
		    .addFunction("readFloat", &PacketWrapper::readFloat)
		    .addFunction("readDouble", &PacketWrapper::readDouble)
		    .addFunction("readString", &PacketWrapper::readString)
		    .addFunction("readTable", &PacketWrapper::readTable)
		    // ----

		    .endClass();
	}
	// -------------------------
} // namespace rawrbox