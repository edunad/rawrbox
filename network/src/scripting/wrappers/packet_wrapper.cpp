
#include <rawrbox/network/scripting/wrappers/packet_wrapper.hpp>
#include <rawrbox/scripting/utils/lua.hpp>

namespace rawrbox {
	PacketWrapper::PacketWrapper(const rawrbox::Packet& packet) : data(packet) {}

	// UTILS -----
	void PacketWrapper::seek(size_t offset) { data.seek(offset - 1); }
	void PacketWrapper::clear() { data.clear(); }
	size_t PacketWrapper::tell() { return data.tell() + 1; }
	size_t PacketWrapper::size() { return data.size(); }
	bool PacketWrapper::empty() { return data.empty(); }
	// -------------------

	// WRITE ---
	void PacketWrapper::writeBool(bool val) { data.write(val); }
	void PacketWrapper::writeByte(uint8_t val) { data.write(val); }
	void PacketWrapper::writeChar(char val) { data.write(val); }
	void PacketWrapper::writeShort(int16_t val) { data.write(val); }
	void PacketWrapper::writeUShort(uint16_t val) { data.write(val); }
	void PacketWrapper::writeInt(int32_t val) { data.write(val); }
	void PacketWrapper::writeUInt(uint32_t val) { data.write(val); }
	void PacketWrapper::writeLong(int64_t val) { data.write(val); }
	void PacketWrapper::writeULong(uint64_t val) { data.write(val); }
	void PacketWrapper::writeFloat(float val) { data.write(val); }
	void PacketWrapper::writeDouble(double val) { data.write(val); }
	void PacketWrapper::writeString(const std::string& val) { data.write(val); }
	void PacketWrapper::writeTable(sol::table val) { data.write(rawrbox::LuaUtils::luaToJsonObject(val).dump()); }
	// -------

	// READ ----
	bool PacketWrapper::readBool() { return data.read<bool>(); }
	uint8_t PacketWrapper::readByte() { return data.read<uint8_t>(); }
	char PacketWrapper::readChar() { return data.read<char>(); }
	int16_t PacketWrapper::readShort() { return data.read<int16_t>(); }
	uint16_t PacketWrapper::readUShort() { return data.read<uint16_t>(); }
	int32_t PacketWrapper::readInt() { return data.read<int32_t>(); }
	uint32_t PacketWrapper::readUInt() { return data.read<uint32_t>(); }
	int64_t PacketWrapper::readLong() { return data.read<int64_t>(); }
	uint64_t PacketWrapper::readULong() { return data.read<uint64_t>(); }
	float PacketWrapper::readFloat() { return data.read<float>(); }
	double PacketWrapper::readDouble() { return data.read<double>(); }
	std::string PacketWrapper::readString() { return data.read<std::string>(); }

	sol::table PacketWrapper::readTable(sol::this_state lua) {
		sol::state_view view = lua;
		return rawrbox::LuaUtils::jsonToLuaObject(nlohmann::json::parse(data.read<std::string>()), view);
	}
	// ----------

	void PacketWrapper::registerLua(sol::state& lua) {
		lua.new_usertype<Packet>("Packet",
		    sol::constructors<rawrbox::Packet(), rawrbox::Packet(rawrbox::Packet)>(),

		    // UTILS -----
		    "seek", &PacketWrapper::seek,
		    "tell", &PacketWrapper::tell,
		    "size", &PacketWrapper::size,
		    "clear", &PacketWrapper::clear,
		    "empty", &PacketWrapper::empty,
		    // -------------

		    // WRITE ---
		    "writeBool", &PacketWrapper::writeBool,
		    "writeByte", &PacketWrapper::writeByte,
		    "writeChar", &PacketWrapper::writeChar,
		    "writeShort", &PacketWrapper::writeShort,
		    "writeUShort", &PacketWrapper::writeUShort,
		    "writeInt", &PacketWrapper::writeInt,
		    "writeUInt", &PacketWrapper::writeUInt,
		    "writeLong", &PacketWrapper::writeLong,
		    "writeULong", &PacketWrapper::writeULong,
		    "writeFloat", &PacketWrapper::writeFloat,
		    "writeDouble", &PacketWrapper::writeDouble,
		    "writeString", &PacketWrapper::writeString,
		    "writeTable", &PacketWrapper::writeTable,
		    // ----

		    // READ ---
		    "readBool", &PacketWrapper::readBool,
		    "readByte", &PacketWrapper::readByte,
		    "readChar", &PacketWrapper::readChar,
		    "readShort", &PacketWrapper::readShort,
		    "readUShort", &PacketWrapper::readUShort,
		    "readInt", &PacketWrapper::readInt,
		    "readUInt", &PacketWrapper::readUInt,
		    "readLong", &PacketWrapper::readLong,
		    "readULong", &PacketWrapper::readULong,
		    "readFloat", &PacketWrapper::readFloat,
		    "readDouble", &PacketWrapper::readDouble,
		    "readString", &PacketWrapper::readString,
		    "readTable", &PacketWrapper::readTable
		    // ----
		);
	}
	// -------------------------
} // namespace rawrbox
