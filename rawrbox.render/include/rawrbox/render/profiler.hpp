#pragma once

#include <tracy/Tracy.hpp>

#ifdef TRACY_ENABLE
	#include <rawrbox/render/render_config.hpp>

	#include <RefCntAutoPtr.hpp>

	#include <DeviceContext.h>
	#include <Query.h>
	#include <RenderDevice.h>

	#include <array>
	#include <string_view>
	#include <vector>

namespace rawrbox {
	class GPUProfiler {
	protected:
		std::vector<Diligent::RefCntAutoPtr<Diligent::IQuery>> _queries = {};

		std::array<uint32_t, RB_RENDER_PROFILER_FRAMES> _used = {};
		std::array<bool, RB_RENDER_PROFILER_FRAMES> _pending = {};

		uint32_t _frame = 0;
		uint32_t _reserved = 0;
		uint32_t _plotted = 0;
		uint8_t _id = 0;

		Diligent::IDeviceContext* _context = nullptr; // Owned by the renderer

	public:
		GPUProfiler(Diligent::IRenderDevice& device, Diligent::IDeviceContext& context);
		GPUProfiler(const GPUProfiler&) = delete;
		GPUProfiler(GPUProfiler&&) = delete;
		GPUProfiler& operator=(const GPUProfiler&) = delete;
		GPUProfiler& operator=(GPUProfiler&&) = delete;
		virtual ~GPUProfiler() = default;

		[[nodiscard]] virtual bool begin(std::string_view name, uint32_t line, std::string_view file, std::string_view function);
		virtual void end();

		virtual void plot(const char* name);
		virtual void collect();

		[[nodiscard]] virtual uint32_t getDraws() const;
	};

	class RenderZone {
	protected:
		tracy::ScopedZone _zone;
		rawrbox::GPUProfiler* _profiler = nullptr; // Owned by the renderer

		uint32_t _draws = 0;
		bool _gpu = false;

	public:
		RenderZone(std::string_view name, uint32_t line, std::string_view file, std::string_view function);
		RenderZone(const RenderZone&) = delete;
		RenderZone(RenderZone&&) = delete;
		RenderZone& operator=(const RenderZone&) = delete;
		RenderZone& operator=(RenderZone&&) = delete;
		virtual ~RenderZone();

		static void plot(const char* name);
	};
} // namespace rawrbox

	#define RAWRBOX_RENDER_ZONE(name) const rawrbox::RenderZone TracyConcat(__rawrbox_render_zone, TracyLine)(name, TracyLine, TracyFile, TracyFunction)
	#define RAWRBOX_RENDER_PLOT(name) rawrbox::RenderZone::plot(name)
#else
	#define RAWRBOX_RENDER_ZONE(name)
	#define RAWRBOX_RENDER_PLOT(name)
#endif
