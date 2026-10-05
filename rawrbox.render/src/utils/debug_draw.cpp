
#include <rawrbox/render/bindless.hpp>
#include <rawrbox/render/cameras/base.hpp>
#include <rawrbox/render/render_config.hpp>
#include <rawrbox/render/static.hpp>
#include <rawrbox/render/utils/barrier.hpp>
#include <rawrbox/render/utils/debug_draw.hpp>
#include <rawrbox/render/utils/pipeline.hpp>

#include <MapHelper.hpp>

namespace rawrbox {
	static_assert(sizeof(rawrbox::DebugVertex) == 16, "DebugVertex must stay tightly packed (pos + RGBA8)");

	// STATIC DATA ----
	std::array<std::vector<rawrbox::DebugVertex>, 4> DebugDraw::_queues = {};
	std::array<Diligent::IPipelineState*, 4> DebugDraw::_pipelines = {};

	Diligent::RefCntAutoPtr<Diligent::IBuffer> DebugDraw::_buffer;
	size_t DebugDraw::_bufferVertices = 0;
	// ----------------

	// PRIVATE ----
	size_t DebugDraw::queueIndex(bool line, bool depthTest) {
		return (depthTest ? 0U : 2U) | (line ? 1U : 0U);
	}

	void DebugDraw::push(size_t queue, const rawrbox::Vector3f& pos, uint32_t color) {
		_queues[queue].push_back({pos, color});
	}

	void DebugDraw::createPipelines() {
		rawrbox::PipeSettings settings;

		settings.pVS = "debug_draw.vsh";
		settings.pPS = "debug_draw.psh";

		settings.cull = Diligent::CULL_MODE_NONE;
		settings.renderTargets = RB_RENDER_RENDER_TARGET_TARGETS;    // COLOR + GPUPick
		settings.signatures = {rawrbox::BindlessManager::signature}; // Camera
		settings.blending = {Diligent::BLEND_FACTOR_SRC_ALPHA, Diligent::BLEND_FACTOR_INV_SRC_ALPHA};
		settings.depthWrite = false;

		settings.layout = {
		    Diligent::LayoutElement{0, 0, 3, Diligent::VT_FLOAT32, false},
		    Diligent::LayoutElement{1, 0, 1, Diligent::VT_UINT32, false}};

		for (size_t i = 0; i < _pipelines.size(); i++) {
			const bool line = (i & 1U) != 0;
			const bool depthTest = (i & 2U) == 0;

			settings.topology = line ? Diligent::PRIMITIVE_TOPOLOGY_LINE_LIST : Diligent::PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
			settings.depth = depthTest ? Diligent::COMPARISON_FUNC_LESS : Diligent::COMPARISON_FUNC_ALWAYS;

			const std::string name = std::string("DebugDraw::") + (depthTest ? "Depth" : "NoDepth") + (line ? "::Line" : "");
			_pipelines[i] = rawrbox::PipelineUtils::createPipeline(name, settings);
		}
	}

	void DebugDraw::ensureBuffer(size_t vertices) {
		if (_buffer != nullptr && vertices <= _bufferVertices) return;

		size_t size = std::max(MIN_BUFFER_VERTICES, _bufferVertices);
		while (size < vertices) {
			size *= 2;
		}

		RAWRBOX_DESTROY(_buffer);

		Diligent::BufferDesc desc;
		desc.Name = "RawrBox::Buffer::DebugDraw";
		desc.Usage = Diligent::USAGE_DYNAMIC;
		desc.CPUAccessFlags = Diligent::CPU_ACCESS_WRITE;
		desc.BindFlags = Diligent::BIND_VERTEX_BUFFER;
		desc.Size = static_cast<uint64_t>(size * sizeof(rawrbox::DebugVertex));

		rawrbox::RENDERER->device()->CreateBuffer(desc, nullptr, &_buffer);
		if (_buffer == nullptr) RAWRBOX_CRITICAL("Failed to create debug draw buffer");

		rawrbox::BarrierUtils::barrier({{_buffer, Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_VERTEX_BUFFER, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE}});
		_bufferVertices = size;
	}
	// ------------

	void DebugDraw::shutdown() {
		for (auto& queue : _queues) {
			queue = {};
		}

		RAWRBOX_DESTROY(_buffer);

		_pipelines = {};
		_bufferVertices = 0;
	}

	void DebugDraw::line(const rawrbox::Vector3f& a, const rawrbox::Vector3f& b, const rawrbox::Colorf& color, bool depthTest) {
		const size_t queue = queueIndex(true, depthTest);
		const uint32_t packed = color.pack();

		push(queue, a, packed);
		push(queue, b, packed);
	}

	void DebugDraw::triangle(const rawrbox::Vector3f& a, const rawrbox::Vector3f& b, const rawrbox::Vector3f& c, const rawrbox::Colorf& color, bool depthTest) {
		const size_t queue = queueIndex(false, depthTest);
		const uint32_t packed = color.pack();

		push(queue, a, packed);
		push(queue, b, packed);
		push(queue, c, packed);
	}

	void DebugDraw::quad(const rawrbox::Vector3f& a, const rawrbox::Vector3f& b, const rawrbox::Vector3f& c, const rawrbox::Vector3f& d, const rawrbox::Colorf& color, bool depthTest) {
		triangle(a, b, c, color, depthTest);
		triangle(a, c, d, color, depthTest);
	}

	void DebugDraw::aabb(const rawrbox::Vector3f& min, const rawrbox::Vector3f& max, const rawrbox::Colorf& color, bool depthTest) {
		const rawrbox::Vector3f corners[8] = {
		    {min.x, min.y, min.z}, {max.x, min.y, min.z}, {max.x, min.y, max.z}, {min.x, min.y, max.z},
		    {min.x, max.y, min.z}, {max.x, max.y, min.z}, {max.x, max.y, max.z}, {min.x, max.y, max.z}};

		constexpr std::array<std::pair<int, int>, 12> EDGES = {{{0, 1}, {1, 2}, {2, 3}, {3, 0}, {4, 5}, {5, 6}, {6, 7}, {7, 4}, {0, 4}, {1, 5}, {2, 6}, {3, 7}}};

		const size_t queue = queueIndex(true, depthTest);
		const uint32_t packed = color.pack();

		for (const auto& [i, j] : EDGES) {
			push(queue, corners[i], packed);
			push(queue, corners[j], packed);
		}
	}

	void DebugDraw::bbox(const rawrbox::Vector3f& pos, const rawrbox::BBOXf& box, const rawrbox::Colorf& color, bool depthTest) {
		aabb(pos + box.min, pos + box.max, color, depthTest);
	}

	void DebugDraw::draw() {
		size_t total = 0;
		for (const auto& queue : _queues) {
			total += queue.size();
		}

		if (total == 0) return;
		if (rawrbox::MAIN_CAMERA == nullptr) RAWRBOX_CRITICAL("Main camera not initialized");

		auto* context = rawrbox::RENDERER->context();
		if (_pipelines[0] == nullptr) createPipelines();
		ensureBuffer(total);

		// Upload ----
		std::array<uint32_t, 4> offsets = {};
		{
			Diligent::MapHelper<rawrbox::DebugVertex> data(context, _buffer, Diligent::MAP_WRITE, Diligent::MAP_FLAG_DISCARD);
			if (data == nullptr) RAWRBOX_CRITICAL("Failed to map debug draw buffer");
			rawrbox::DebugVertex* dst = data;

			size_t offset = 0;
			for (size_t i = 0; i < _queues.size(); i++) {
				offsets[i] = static_cast<uint32_t>(offset);
				if (_queues[i].empty()) continue;

				std::memcpy(dst + offset, _queues[i].data(), _queues[i].size() * sizeof(rawrbox::DebugVertex));
				offset += _queues[i].size();
			}
		}
		// ---------------------------------

		rawrbox::MAIN_CAMERA->setModelTransform({});

		std::array<Diligent::IBuffer*, 1> buffers = {_buffer};
		context->SetVertexBuffers(0, 1, buffers.data(), nullptr, Diligent::RESOURCE_STATE_TRANSITION_MODE_VERIFY, Diligent::SET_VERTEX_BUFFERS_FLAG_RESET);

		for (size_t i = 0; i < _queues.size(); i++) {
			if (_queues[i].empty()) continue;

			context->SetPipelineState(_pipelines[i]);

			Diligent::DrawAttribs attrs;
			attrs.NumVertices = static_cast<uint32_t>(_queues[i].size());
			attrs.StartVertexLocation = offsets[i];

			context->Draw(attrs);
		}

		for (auto& queue : _queues)
			queue.clear();
	}
} // namespace rawrbox
