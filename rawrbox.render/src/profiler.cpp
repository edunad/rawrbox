#include <rawrbox/render/profiler.hpp>

#ifdef TRACY_ENABLE
	#include <rawrbox/engine/static.hpp>
	#include <rawrbox/render/static.hpp>
	#include <rawrbox/utils/logger.hpp>

	#include <tracy/TracyC.h>

	#include <fmt/format.h>

	#include <cstring>
	#include <thread>

namespace rawrbox {
	namespace {
		rawrbox::GPUProfiler* activeProfiler() {
			if (rawrbox::RENDERER == nullptr || std::this_thread::get_id() != rawrbox::RENDER_THREAD_ID) return nullptr;
			return rawrbox::RENDERER->profiler();
		}
	} // namespace

	// GPU PROFILER ---
	GPUProfiler::GPUProfiler(Diligent::IRenderDevice& device, Diligent::IDeviceContext& context) : _context(&context) {
		Diligent::QueryDesc desc{Diligent::QUERY_TYPE_TIMESTAMP};
		desc.Name = "RawrBox::PROFILER::TIMESTAMP";

		this->_queries.resize(RB_RENDER_PROFILER_FRAMES * RB_RENDER_PROFILER_QUERIES);
		for (auto& query : this->_queries) {
			device.CreateQuery(desc, &query);
			if (query == nullptr)
				RAWRBOX_CRITICAL("Failed to create profiler timestamp query");
		}

		// CALIBRATION ---
		Diligent::QueryDataTimestamp timestamp;

		context.EndQuery(this->_queries.front());
		context.Flush();
		context.WaitForIdle();

		if (!this->_queries.front()->GetData(&timestamp, sizeof(timestamp)) || timestamp.Frequency == 0)
			RAWRBOX_CRITICAL("Failed to read profiler calibration timestamp");
		// -------------

		const auto& adapter = device.GetAdapterInfo();
		const auto type = device.GetDeviceInfo().Type == Diligent::RENDER_DEVICE_TYPE_VULKAN ? tracy::GpuContextType::Vulkan : tracy::GpuContextType::Direct3D12;

		this->_id = static_cast<uint8_t>(tracy::GetGpuCtxCounter().fetch_add(1, std::memory_order_relaxed));

		___tracy_emit_gpu_new_context_serial({static_cast<int64_t>(timestamp.Counter), static_cast<float>(1e9 / static_cast<double>(timestamp.Frequency)), this->_id, 0, static_cast<uint8_t>(type)});
		___tracy_emit_gpu_context_name_serial({this->_id, adapter.Description, static_cast<uint16_t>(std::strlen(adapter.Description))});
	}

	bool GPUProfiler::begin(std::string_view name, uint32_t line, std::string_view file, std::string_view function) {
		if (this->_pending[this->_frame]) return false;

		auto& used = this->_used[this->_frame];
		if (used + this->_reserved + 2 > RB_RENDER_PROFILER_QUERIES) return false;

		const uint32_t query = this->_frame * RB_RENDER_PROFILER_QUERIES + used++;
		this->_reserved++;
		this->_context->EndQuery(this->_queries[query]);

		const auto srcloc = ___tracy_alloc_srcloc_name(line, file.data(), file.size(), function.data(), function.size(), name.data(), name.size(), 0);
		___tracy_emit_gpu_zone_begin_alloc_serial({srcloc, static_cast<uint16_t>(query), this->_id});

		return true;
	}

	void GPUProfiler::end() {
		const uint32_t query = this->_frame * RB_RENDER_PROFILER_QUERIES + this->_used[this->_frame]++;
		this->_reserved--;

		this->_context->EndQuery(this->_queries[query]);
		___tracy_emit_gpu_zone_end_serial({static_cast<uint16_t>(query), this->_id});
	}

	void GPUProfiler::plot(const char* name) {
		const uint32_t draws = this->getDraws();
		TracyPlot(name, static_cast<int64_t>(draws - this->_plotted));

		this->_plotted = draws;
	}

	void GPUProfiler::collect() {
		if (this->_reserved != 0) RAWRBOX_CRITICAL("{} render zones are still open!", this->_reserved);
		if (this->_used[this->_frame] != 0) this->_pending[this->_frame] = true;

		// READ ---
		for (uint32_t i = 1; i <= RB_RENDER_PROFILER_FRAMES; i++) {
			const uint32_t frame = (this->_frame + i) % RB_RENDER_PROFILER_FRAMES;
			if (!this->_pending[frame]) continue;

			const uint32_t start = frame * RB_RENDER_PROFILER_QUERIES;
			const uint32_t used = this->_used[frame];
			if (!this->_queries[start + used - 1]->GetData(nullptr, 0, false)) break;

			for (uint32_t query = start; query < start + used; query++) {
				Diligent::QueryDataTimestamp timestamp;
				this->_queries[query]->GetData(&timestamp, sizeof(timestamp));

				___tracy_emit_gpu_time_serial({static_cast<int64_t>(timestamp.Counter), static_cast<uint16_t>(query), this->_id});
			}

			this->_used[frame] = 0;
			this->_pending[frame] = false;
		}
		// ------------------

		this->_frame = (this->_frame + 1) % RB_RENDER_PROFILER_FRAMES;

		this->_context->ClearStats();
		this->_plotted = 0;
	}

	uint32_t GPUProfiler::getDraws() const {
		const auto& counters = this->_context->GetStats().CommandCounters;
		return counters.Draw + counters.DrawIndexed + counters.DrawIndirect + counters.DrawIndexedIndirect + counters.DrawMesh + counters.DrawMeshIndirect + counters.MultiDraw + counters.MultiDrawIndexed;
	}
	// ----------------

	// RENDER ZONE ---
	RenderZone::RenderZone(std::string_view name, uint32_t line, std::string_view file, std::string_view function) : _zone(line, file.data(), file.size(), function.data(), function.size(), name.data(), name.size(), 0U), _profiler(activeProfiler()) {
		if (this->_profiler == nullptr) return;

		this->_draws = this->_profiler->getDraws();
		this->_gpu = this->_profiler->begin(name, line, file, function);
	}

	RenderZone::~RenderZone() {
		if (this->_profiler == nullptr) return;
		if (this->_gpu) this->_profiler->end();

		const auto text = fmt::format("{} draws", this->_profiler->getDraws() - this->_draws);
		this->_zone.Text(text.c_str(), text.size());
	}

	void RenderZone::plot(const char* name) {
		auto* profiler = activeProfiler();
		if (profiler != nullptr) profiler->plot(name);
	}
	// ----------------
} // namespace rawrbox
#endif