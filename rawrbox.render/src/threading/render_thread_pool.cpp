#include <rawrbox/render/threading/render_thread_pool.hpp>
#include <rawrbox/utils/thread_utils.hpp>

#include <fmt/format.h>

#include <algorithm>
#include <utility>

namespace rawrbox {
	namespace {
		thread_local bool RECORDING = false;
		thread_local Diligent::IDeviceContext* CONTEXT = nullptr;
		thread_local uint32_t CONTEXT_INDEX = 0;

		class RecordingScope {
		public:
			RecordingScope(Diligent::IDeviceContext& context, uint32_t index) {
				RECORDING = true;

				CONTEXT = &context;
				CONTEXT_INDEX = index;
			}

			RecordingScope(const RecordingScope&) = delete;
			RecordingScope(RecordingScope&&) = delete;
			RecordingScope& operator=(const RecordingScope&) = delete;
			RecordingScope& operator=(RecordingScope&&) = delete;
			~RecordingScope() {
				RECORDING = false;
				CONTEXT = nullptr;
				CONTEXT_INDEX = 0;
			}
		};
	} // namespace

	RenderThreadPool::RenderThreadPool(Diligent::IDeviceContext& immediate, std::vector<Diligent::RefCntAutoPtr<Diligent::IDeviceContext>> deferred) : _immediate(&immediate), _owner(std::this_thread::get_id()) {
		if (immediate.GetDesc().IsDeferred) RAWRBOX_CRITICAL("Expected an immediate context");

		this->_slots.resize(deferred.size());
		for (size_t i = 0; i < deferred.size(); i++) {
			auto& context = deferred[i];
			if (context == nullptr || !context->GetDesc().IsDeferred) RAWRBOX_CRITICAL("Context #{} is not a deferred", i);

			this->_slots[i].context = std::move(context);
			this->_slots[i].index = static_cast<uint32_t>(i + 1);
			this->_slots[i].label = fmt::format("RawrBox::RenderJob::{}", i + 1);
		}

		const size_t workers = this->_slots.size();
		this->_workers.reserve(workers);

		try {
			for (size_t i = 0; i < workers; i++) {
				this->_workers.emplace_back(&RenderThreadPool::workerMain, this, i);
			}
		} catch (...) {
			{
				std::lock_guard<std::mutex> lock(this->_mutex);
				this->_stop = true;
			}

			this->_wake.notify_all();
			for (auto& worker : this->_workers) {
				if (worker.joinable()) worker.join();
			}

			throw;
		}

		this->_executeList.reserve(this->_slots.size());
		this->_bounds.reserve(this->_slots.size() + 2);
		this->_logger->info("Recording on {} contexts -> {} worker threads", fmt::styled(this->slots(), fmt::fg(fmt::color::coral)), fmt::styled(workers, fmt::fg(fmt::color::coral)));
	}

	RenderThreadPool::~RenderThreadPool() {
		{
			std::lock_guard<std::mutex> lock(this->_mutex);
			this->_stop = true;
		}

		this->_wake.notify_all();
		for (auto& worker : this->_workers) {
			if (worker.joinable()) worker.join();
		}
	}

	void RenderThreadPool::workerMain(size_t slotIndex) {
		rawrbox::ThreadUtils::setName(fmt::format("rawrbox:render_worker_{}", slotIndex));

		auto& slot = this->_slots[slotIndex];
		uint64_t seen = 0;

		while (true) {
			Job job = {};

			{
				std::unique_lock<std::mutex> lock(this->_mutex);
				this->_wake.wait(lock, [this, &seen]() { return this->_stop || this->_generation != seen; });
				if (this->_stop) break;

				seen = this->_generation;
				job = this->_job;
			}

			this->runSlot(slot, job);

			{
				std::lock_guard<std::mutex> lock(this->_mutex);
				if (--this->_pending == 0) this->_done.notify_one();
			}
		}

		if (slot.dirty != NO_FRAME) slot.context->FinishFrame();
	}

	void RenderThreadPool::runSlot(Slot& slot, const Job& job) const {
		if (slot.begin >= slot.end) return;

		auto& context = *slot.context;
		const RecordingScope scope(context, slot.index);

		try {
			if (slot.dirty != NO_FRAME && slot.dirty != job.frame) context.FinishFrame();
			context.Begin(this->_immediate->GetDesc().ContextId);
			context.InsertDebugLabel(slot.label.c_str());
		} catch (...) {
			slot.error = std::current_exception();
			return;
		}

		slot.dirty = job.frame;

		try {
			if (job.setup != nullptr && *job.setup) (*job.setup)(context);
			(*job.record)(context, slot.begin, slot.end);
		} catch (...) {
			slot.error = std::current_exception();
		}

		try {
			context.FinishCommandList(&slot.commandList);
		} catch (...) {
			if (slot.error == nullptr) slot.error = std::current_exception();
		}
	}

	void RenderThreadPool::split(uint32_t count, uint32_t parts, std::span<const uint32_t> weights) {
		this->_bounds.assign(parts + 1, count);
		this->_bounds.front() = 0;

		uint64_t total = 0;
		uint64_t heaviest = 0;

		for (const uint32_t weight : weights) {
			total += weight;
			heaviest = std::max<uint64_t>(heaviest, weight);
		}

		if (total == 0) {
			for (uint32_t i = 1; i < parts; i++) {
				this->_bounds[i] = static_cast<uint32_t>(static_cast<uint64_t>(count) * i / parts);
			}

			return;
		}

		const auto fits = [&weights, parts](uint64_t capacity) {
			uint32_t used = 1;
			uint64_t load = 0;

			for (const uint32_t weight : weights) {
				if (load + weight > capacity) {
					if (++used > parts) return false;
					load = 0;
				}

				load += weight;
			}

			return true;
		};

		uint64_t capacity = heaviest;
		uint64_t high = total;

		while (capacity < high) {
			const uint64_t mid = capacity + (high - capacity) / 2;

			if (fits(mid)) {
				high = mid;
			} else {
				capacity = mid + 1;
			}
		}

		uint32_t part = 1;
		uint64_t load = 0;

		for (uint32_t i = 0; i < count && part < parts; i++) {
			if (load + weights[i] > capacity) {
				this->_bounds[part++] = i;
				load = 0;
			}

			load += weights[i];
		}
	}

	bool RenderThreadPool::record(uint32_t count, const RecordFunc& record, const SetupFunc& setup, std::span<const uint32_t> weights) {
		if (std::this_thread::get_id() != this->_owner) RAWRBOX_CRITICAL("'record' must be called from the owner thread");

		if (RECORDING) RAWRBOX_CRITICAL("'record' cannot be called from inside a render job");
		if (!record) RAWRBOX_CRITICAL("Invalid record callback");
		if (!weights.empty() && weights.size() != count) RAWRBOX_CRITICAL("Expected {} weights, got {}", count, weights.size());

		if (count == 0) return false;

		const uint32_t parts = std::min(this->slots(), count);
		if (parts > 1) this->split(count, parts, weights);

		if (parts <= 1 || this->_bounds[1] >= count) {
			const RecordingScope scope(*this->_immediate, 0);
			record(*this->_immediate, 0, count);

			return false;
		}

		for (uint32_t i = 0; i < static_cast<uint32_t>(this->_slots.size()); i++) {
			auto& slot = this->_slots[i];

			slot.commandList.Release();
			slot.error = nullptr;

			if (i + 1 < parts) {
				slot.begin = this->_bounds[i + 1];
				slot.end = this->_bounds[i + 2];
			} else {
				slot.begin = 0;
				slot.end = 0;
			}
		}

		const Job job = {
		    .record = &record,
		    .setup = &setup,

		    .frame = this->_frame};

		{
			std::lock_guard<std::mutex> lock(this->_mutex);

			this->_job = job;
			this->_pending = static_cast<uint32_t>(this->_workers.size());

			++this->_generation;
		}

		this->_wake.notify_all();

		std::exception_ptr error = nullptr;
		if (this->_bounds[0] < this->_bounds[1]) {
			const RecordingScope scope(*this->_immediate, 0);

			try {
				record(*this->_immediate, this->_bounds[0], this->_bounds[1]);
			} catch (...) {
				error = std::current_exception();
			}
		}

		{
			std::unique_lock<std::mutex> lock(this->_mutex);
			this->_done.wait(lock, [this]() { return this->_pending == 0; });
			this->_job = {};
		}

		this->_executeList.clear();

		for (auto& slot : this->_slots) {
			if (slot.error != nullptr && error == nullptr) error = slot.error;
			slot.error = nullptr;

			if (slot.commandList != nullptr) this->_executeList.push_back(slot.commandList);
		}

		const bool executed = error == nullptr && !this->_executeList.empty();
		if (executed) this->_immediate->ExecuteCommandLists(static_cast<uint32_t>(this->_executeList.size()), this->_executeList.data());

		this->_executeList.clear();
		for (auto& slot : this->_slots) {
			slot.commandList.Release();
		}

		if (error != nullptr) std::rethrow_exception(error);
		return executed;
	}

	void RenderThreadPool::finishFrame() {
		if (std::this_thread::get_id() != this->_owner) RAWRBOX_CRITICAL("'finishFrame' must be called inside the owner thread");
		if (RECORDING) RAWRBOX_CRITICAL("'finishFrame' cannot be called from inside a render job");

		++this->_frame;
	}

	uint32_t RenderThreadPool::slots() const {
		return static_cast<uint32_t>(this->_slots.size()) + 1U;
	}

	uint32_t RenderThreadPool::contextCount() const {
		return static_cast<uint32_t>(this->_slots.size()) + 1U;
	}

	bool RenderThreadPool::isRecording() { return RECORDING; }
	Diligent::IDeviceContext* RenderThreadPool::currentContext() { return CONTEXT; }
	uint32_t RenderThreadPool::contextIndex() { return CONTEXT_INDEX; }
} // namespace rawrbox