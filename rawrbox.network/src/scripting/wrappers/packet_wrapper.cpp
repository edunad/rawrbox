
#include <rawrbox/network/scripting/wrappers/packet_wrapper.hpp>

#include <utility>

namespace rawrbox {
	PacketWrapper::PacketWrapper(rawrbox::Packet packet) : data(std::move(packet)) {}

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
	void PacketWrapper::writeTable(const luabridge::LuaRef& val) { data.write(rawrbox::LuaUtils::luaToJsonObject(val).dump()); }
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

	luabridge::LuaRef PacketWrapper::readTable(lua_State* L) {
		return rawrbox::LuaUtils::jsonToLua(L, nlohmann::json::parse(data.read<std::string>()));
	}
	// ----------

	void PacketWrapper::registerLua(lua_State* L) {

		luabridge::getGlobalNamespace(L)
		    .beginClass<rawrbox::PacketWrapper>("Packet")
		    .addConstructor<void(), void(rawrbox::PacketWrapper), void(rawrbox::Packet)>()

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
