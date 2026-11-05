#pragma once

#include <rawrbox/utils/logger.hpp>

#include <RefCntAutoPtr.hpp>

#include <CommandList.h>
#include <DeviceContext.h>

#include <condition_variable>
#include <exception>
#include <functional>
#include <limits>
#include <memory>
#include <mutex>
#include <span>
#include <string>
#include <thread>
#include <vector>

namespace rawrbox {
	class RenderThreadPool {
	public:
		using SetupFunc = std::function<void(Diligent::IDeviceContext& context)>;
		using RecordFunc = std::function<void(Diligent::IDeviceContext& context, uint32_t begin, uint32_t end)>;

	protected:
		static constexpr uint64_t NO_FRAME = std::numeric_limits<uint64_t>::max();

		struct Slot {
			Diligent::RefCntAutoPtr<Diligent::IDeviceContext> context;
			uint32_t index = 0;
			std::string label;

			Diligent::RefCntAutoPtr<Diligent::ICommandList> commandList;
			std::exception_ptr error = nullptr;

			uint32_t begin = 0;
			uint32_t end = 0;

			uint64_t dirty = NO_FRAME;
		};

		struct Job {
			const RecordFunc* record = nullptr;
			const SetupFunc* setup = nullptr;

			uint64_t frame = 0;
		};

		Diligent::RefCntAutoPtr<Diligent::IDeviceContext> _immediate;

		std::vector<Slot> _slots = {};
		std::vector<std::thread> _workers = {};
		std::vector<Diligent::ICommandList*> _executeList = {};
		std::vector<uint32_t> _bounds = {};

		std::thread::id _owner;

		std::mutex _mutex;
		std::condition_variable _wake;
		std::condition_variable _done;

		Job _job = {};
		uint64_t _generation = 0;
		uint32_t _pending = 0;
		uint64_t _frame = 0;

		bool _stop = false;

		std::unique_ptr<rawrbox::Logger> _logger = std::make_unique<rawrbox::Logger>("RawrBox-RenderThreadPool");

		void workerMain(size_t slotIndex);
		void runSlot(Slot& slot, const Job& job) const;

		void split(uint32_t count, uint32_t parts, std::span<const uint32_t> weights);

	public:
		RenderThreadPool(Diligent::IDeviceContext& immediate, std::vector<Diligent::RefCntAutoPtr<Diligent::IDeviceContext>> deferred);
		RenderThreadPool(const RenderThreadPool&) = delete;
		RenderThreadPool(RenderThreadPool&&) = delete;
		RenderThreadPool& operator=(const RenderThreadPool&) = delete;
		RenderThreadPool& operator=(RenderThreadPool&&) = delete;
		~RenderThreadPool();

		bool record(uint32_t count, const RecordFunc& record, const SetupFunc& setup = nullptr, std::span<const uint32_t> weights = {});
		void finishFrame();

		[[nodiscard]] uint32_t slots() const;
		[[nodiscard]] uint32_t contextCount() const;

		[[nodiscard]] static bool isRecording();
		[[nodiscard]] static Diligent::IDeviceContext* currentContext();
		[[nodiscard]] static uint32_t contextIndex();
	};
} // namespace rawrbox