#pragma once

#include <rawrbox/network/packet.hpp>

#include <sol/sol.hpp>

namespace rawrbox {
	class PacketWrapper {
	protected:
		rawrbox::Packet data;

	public:
		PacketWrapper() = default;
		PacketWrapper(const PacketWrapper&) = default;
		PacketWrapper(PacketWrapper&&) = delete;
		PacketWrapper& operator=(const PacketWrapper&) = default;
		PacketWrapper& operator=(PacketWrapper&&) = delete;
		PacketWrapper(rawrbox::Packet packet);
		virtual ~PacketWrapper() = default;

		// UTILS ---
		virtual void seek(size_t offset);
		virtual void clear();
		virtual size_t tell();
		virtual size_t size();
		virtual bool empty();
		// ----

		// WRITE ---
		virtual void writeBool(bool val);
		virtual void writeByte(uint8_t val);
		virtual void writeChar(char val);
		virtual void writeShort(int16_t val);
		virtual void writeUShort(uint16_t val);
		virtual void writeInt(int32_t val);
		virtual void writeUInt(uint32_t val);
		virtual void writeLong(int64_t val);
		virtual void writeULong(uint64_t val);
		virtual void writeFloat(float val);
		virtual void writeDouble(double val);
		virtual void writeString(const std::string& val);
		virtual void writeTable(sol::table val);
		// ---

		// WRITE ---
		virtual bool readBool();
		virtual uint8_t readByte();
		virtual char readChar();
		virtual int16_t readShort();
		virtual uint16_t readUShort();
		virtual int32_t readInt();
		virtual uint32_t readUInt();
		virtual int64_t readLong();
		virtual uint64_t readULong();
		virtual float readFloat();
		virtual double readDouble();
		virtual std::string readString();
		virtual sol::table readTable(sol::this_state lua);
		// ---

		static void registerLua(sol::state& lua);
	};
} // namespace rawrbox
