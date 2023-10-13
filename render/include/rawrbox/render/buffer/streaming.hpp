#pragma once

#include <Common/interface/RefCntAutoPtr.hpp>
#include <Graphics/GraphicsTools/interface/MapHelper.hpp>
#include <Platforms/Basic/interface/DebugUtilities.hpp>

#include <Graphics/GraphicsEngine/interface/Buffer.h>

#include <cstdint>
#include <vector>

namespace rawrbox {
	struct StreamingMapInfo {
		Diligent::MapHelper<uint8_t> mappedData;
		uint32_t currOffset = 0;
	};

	class StreamingBuffer {
	private:
		Diligent::RefCntAutoPtr<Diligent::IBuffer> _buffer;
		uint32_t _bufferSize = 0;
		bool _allowPersistentMap = false;

		// We need to keep track of mapped data for every context
		std::vector<StreamingMapInfo> _mapInfo = {};

	public:
		StreamingBuffer(const std::string& name, Diligent::BIND_FLAGS flags, uint32_t size, size_t contexts);

		// UTILS ----
		Diligent::IBuffer* buffer();
		void* getCPUAddress(size_t context);
		// ----------

		uint32_t allocate(uint32_t size, size_t context);
		void flush(size_t context);

		void release(size_t context);
		void setPersistent(bool persistent);
	};

} // namespace rawrbox
