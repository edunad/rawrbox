
#include <rawrbox/render/bindless.hpp>
#include <rawrbox/render/cameras/base.hpp>
#include <rawrbox/render/render_config.hpp>
#include <rawrbox/render/static.hpp>
#include <rawrbox/render/utils/barrier.hpp>
#include <rawrbox/render/utils/debug_draw.hpp>
#include <rawrbox/render/utils/pipeline.hpp>

#include <MapHelper.hpp>

namespace rawrbox {
	// STATIC DATA ----
	std::array<std::vector<rawrbox::DebugVertex>, 4> DebugDraw::_queues = {};
	std::array<Diligent::IPipelineState*, 4> DebugDraw::_pipelines = {};

	Diligent::RefCntAutoPtr<Diligent::IBuffer> DebugDraw::_buffer;
	std::mutex DebugDraw::_lock;
	// ----------------

	// PRIVATE ----
	size_t DebugDraw::queueIndex(bool line, bool depthTest) {
		return (depthTest ? 0U : 2U) | (line ? 1U : 0U);
	}

	void DebugDraw::push(size_t queue, const rawrbox::Vector3f& pos, uint32_t color) {
		std::lock_guard<std::mutex> lock(DebugDraw::_lock);
		DebugDraw::_queues[queue].push_back({pos, color});
	}

	void DebugDraw::init() {
		rawrbox::PipeSettings settings;

		settings.pVS = "debug/debug_draw.vsh";
		settings.pPS = "debug/debug_draw.psh";

		settings.cull = Diligent::CULL_MODE_NONE;
		settings.renderTargets = RB_RENDER_RENDER_TARGET_TARGETS;    // COLOR + GPUPick
		settings.signatures = {rawrbox::BindlessManager::signature}; // Camera
		settings.blending = {Diligent::BLEND_FACTOR_SRC_ALPHA, Diligent::BLEND_FACTOR_INV_SRC_ALPHA};
		settings.depthWrite = false;

		settings.layout = {
		    Diligent::LayoutElement{0, 0, 3, Diligent::VT_FLOAT32, false},
		    Diligent::LayoutElement{1, 0, 1, Diligent::VT_UINT32, false}};

		for (size_t i = 0; i < DebugDraw::_pipelines.size(); i++) {
			const bool line = (i & 1U) != 0;
			const bool depthTest = (i & 2U) == 0;

			settings.topology = line ? Diligent::PRIMITIVE_TOPOLOGY_LINE_LIST : Diligent::PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
			settings.depth = depthTest ? Diligent::COMPARISON_FUNC_LESS : Diligent::COMPARISON_FUNC_ALWAYS;

			const std::string name = std::string("DebugDraw::") + (depthTest ? "Depth" : "NoDepth") + (line ? "::Line" : "");
			DebugDraw::_pipelines[i] = rawrbox::PipelineUtils::createPipeline(name, settings);
		}

		// Buffer ---
		Diligent::BufferDesc desc;
		desc.Name = "RawrBox::Buffer::DebugDraw";
		desc.Usage = Diligent::USAGE_DYNAMIC;
		desc.CPUAccessFlags = Diligent::CPU_ACCESS_WRITE;
		desc.BindFlags = Diligent::BIND_VERTEX_BUFFER;
		desc.Size = static_cast<uint64_t>(RB_RENDER_MAX_DEBUG_VERTICES * sizeof(rawrbox::DebugVertex));

		rawrbox::RENDERER->device()->CreateBuffer(desc, nullptr, &DebugDraw::_buffer);
		if (DebugDraw::_buffer == nullptr) RAWRBOX_CRITICAL("Failed to create debug draw buffer");

		rawrbox::BarrierUtils::barrier({{DebugDraw::_buffer, Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_VERTEX_BUFFER, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE}});
		// -----
	}
	// ------------

	void DebugDraw::shutdown() {
		DebugDraw::_queues = {};
		RAWRBOX_DESTROY(DebugDraw::_buffer);
		DebugDraw::_pipelines = {};
	}

	void DebugDraw::line(const rawrbox::Vector3f& a, const rawrbox::Vector3f& b, const rawrbox::Colorf& color, bool depthTest) {
		const size_t queue = DebugDraw::queueIndex(true, depthTest);
		const uint32_t packed = color.pack();

		DebugDraw::push(queue, a, packed);
		DebugDraw::push(queue, b, packed);
	}

	void DebugDraw::triangle(const rawrbox::Vector3f& a, const rawrbox::Vector3f& b, const rawrbox::Vector3f& c, const rawrbox::Colorf& color, bool depthTest) {
		const size_t queue = DebugDraw::queueIndex(false, depthTest);
		const uint32_t packed = color.pack();

		DebugDraw::push(queue, a, packed);
		DebugDraw::push(queue, b, packed);
		DebugDraw::push(queue, c, packed);
	}

	void DebugDraw::quad(const rawrbox::Vector3f& a, const rawrbox::Vector3f& b, const rawrbox::Vector3f& c, const rawrbox::Vector3f& d, const rawrbox::Colorf& color, bool depthTest) {
		DebugDraw::triangle(a, b, c, color, depthTest);
		DebugDraw::triangle(a, c, d, color, depthTest);
	}

	void DebugDraw::aabb(const rawrbox::Vector3f& min, const rawrbox::Vector3f& max, const rawrbox::Colorf& color, bool depthTest) {
		const rawrbox::Vector3f corners[8] = {
		    {min.x, min.y, min.z}, {max.x, min.y, min.z}, {max.x, min.y, max.z}, {min.x, min.y, max.z},
		    {min.x, max.y, min.z}, {max.x, max.y, min.z}, {max.x, max.y, max.z}, {min.x, max.y, max.z}};

		constexpr std::array<std::pair<int, int>, 12> EDGES = {{{0, 1}, {1, 2}, {2, 3}, {3, 0}, {4, 5}, {5, 6}, {6, 7}, {7, 4}, {0, 4}, {1, 5}, {2, 6}, {3, 7}}};

		const size_t queue = DebugDraw::queueIndex(true, depthTest);
		const uint32_t packed = color.pack();

		for (const auto& [i, j] : EDGES) {
			DebugDraw::push(queue, corners[i], packed);
			DebugDraw::push(queue, corners[j], packed);
		}
	}

	void DebugDraw::bbox(const rawrbox::Vector3f& pos, const rawrbox::BBOXf& box, const rawrbox::Colorf& color, bool depthTest) {
		DebugDraw::aabb(pos + box.min, pos + box.max, color, depthTest);
	}

	void DebugDraw::draw() {
		if (rawrbox::RenderThreadPool::isRecording()) RAWRBOX_CRITICAL("'DebugDraw::draw' cannot be called inside 'parallelRender'");
		std::lock_guard<std::mutex> lock(DebugDraw::_lock);

		size_t total = 0;
		for (const auto& queue : DebugDraw::_queues) {
			total += queue.size();
		}

		if (total == 0) return;
		if (rawrbox::MAIN_CAMERA == nullptr) RAWRBOX_CRITICAL("Main camera not initialized");

		auto* context = rawrbox::RENDERER->context();

		// Upload, drop if overflow ----
		std::array<uint32_t, 4> offsets = {};
		std::array<uint32_t, 4> counts = {};
		{
			Diligent::MapHelper<rawrbox::DebugVertex> data(context, DebugDraw::_buffer, Diligent::MAP_WRITE, Diligent::MAP_FLAG_DISCARD);
			if (data == nullptr) RAWRBOX_CRITICAL("Failed to map debug draw buffer");
			rawrbox::DebugVertex* dst = data;

			size_t offset = 0;
			for (size_t i = 0; i < DebugDraw::_queues.size(); i++) {
				offsets[i] = static_cast<uint32_t>(offset);
				counts[i] = static_cast<uint32_t>(std::min(DebugDraw::_queues[i].size(), RB_RENDER_MAX_DEBUG_VERTICES - offset));
				counts[i] -= counts[i] % ((i & 1U) != 0 ? 2U : 3U);

				if (counts[i] == 0) continue;

				std::memcpy(dst + offset, DebugDraw::_queues[i].data(), counts[i] * sizeof(rawrbox::DebugVertex));
				offset += counts[i];
			}
		}
		// ---------------------------------

		const std::array<Diligent::IBuffer*, 1> buffers = {DebugDraw::_buffer};
		context->SetVertexBuffers(0, 1, buffers.data(), nullptr, Diligent::RESOURCE_STATE_TRANSITION_MODE_VERIFY, Diligent::SET_VERTEX_BUFFERS_FLAG_RESET);

		for (size_t i = 0; i < DebugDraw::_queues.size(); i++) {
			if (counts[i] == 0) continue;

			context->SetPipelineState(DebugDraw::_pipelines[i]);

			Diligent::DrawAttribs attrs;
			attrs.NumVertices = counts[i];
			attrs.StartVertexLocation = offsets[i];

			context->Draw(attrs);
		}

		for (auto& queue : DebugDraw::_queues)
			queue.clear();
	}
} // namespace rawrbox