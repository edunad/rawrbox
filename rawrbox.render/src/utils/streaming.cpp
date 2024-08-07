#include <rawrbox/render/static.hpp>
#include <rawrbox/render/utils/barrier.hpp>
#include <rawrbox/render/utils/streaming.hpp>

namespace rawrbox {
	StreamingBuffer::StreamingBuffer(const std::string& name, Diligent::BIND_FLAGS flags, uint32_t size, size_t contexts) : _bufferSize(size), _mapInfo(contexts) {
		auto* device = rawrbox::RENDERER->device();

		Diligent::BufferDesc buff;
		buff.Name = name.c_str();
		buff.Usage = Diligent::USAGE_DYNAMIC;
		buff.BindFlags = flags;
		buff.CPUAccessFlags = Diligent::CPU_ACCESS_WRITE;
		buff.Size = size;

		auto state = Diligent::RESOURCE_STATE_VERTEX_BUFFER;
		if (flags == Diligent::BIND_INDEX_BUFFER) state = Diligent::RESOURCE_STATE_INDEX_BUFFER;

		device->CreateBuffer(buff, nullptr, &this->_buffer);

		// Barrier ----
		rawrbox::BarrierUtils::barrier({{this->_buffer, Diligent::RESOURCE_STATE_UNKNOWN, state, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE}});
		// ------------
	}

	// UTILS ----
	Diligent::IBuffer* StreamingBuffer::buffer() { return this->_buffer; }
	void* StreamingBuffer::getCPUAddress(size_t context) { return this->_mapInfo[context].mappedData; };
	// ----------

	uint32_t StreamingBuffer::allocate(uint32_t size, size_t context) {
		auto& map = this->_mapInfo[context];

		// Check if there is enough space in the buffer
		if (map.currOffset + size > this->_bufferSize) {
			this->flush(context);
		}

		if (map.mappedData == nullptr) {
			// If current offset is zero, we are mapping the buffer for the first time after it has been flushed. Use MAP_FLAG_DISCARD flag.
			// Otherwise use MAP_FLAG_NO_OVERWRITE flag.
			map.mappedData.Map(rawrbox::RENDERER->context(), this->_buffer, Diligent::MAP_WRITE, map.currOffset == 0 ? Diligent::MAP_FLAG_DISCARD : Diligent::MAP_FLAG_NO_OVERWRITE);
		}

		auto Offset = map.currOffset;
		map.currOffset += size;

		return Offset;
	}

	void StreamingBuffer::flush(size_t context) {
		this->_mapInfo[context].mappedData.Unmap();
		this->_mapInfo[context].currOffset = 0;
	}

	void StreamingBuffer::release(size_t context) {
		if (this->_allowPersistentMap) return;
		this->_mapInfo[context].mappedData.Unmap();
	}

	void StreamingBuffer::setPersistent(bool persistent) { this->_allowPersistentMap = persistent; }
} // namespace rawrbox
