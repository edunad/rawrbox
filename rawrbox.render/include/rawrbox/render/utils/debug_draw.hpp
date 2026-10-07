#pragma once

#include <rawrbox/math/bbox.hpp>
#include <rawrbox/math/color.hpp>
#include <rawrbox/math/vector3.hpp>

#include <RefCntAutoPtr.hpp>

#include <Buffer.h>
#include <PipelineState.h>

#include <array>
#include <cstdint>
#include <vector>

namespace rawrbox {
	struct DebugVertex {
		rawrbox::Vector3f pos = {};
		uint32_t color = 0; // Packed RGBA8
	};

	class DebugDraw {

	protected:
		static constexpr size_t MIN_BUFFER_VERTICES = 4096;

		static std::array<std::vector<rawrbox::DebugVertex>, 4> _queues;
		static std::array<Diligent::IPipelineState*, 4> _pipelines;

		static Diligent::RefCntAutoPtr<Diligent::IBuffer> _buffer;
		static size_t _bufferVertices;

		static size_t queueIndex(bool line, bool depthTest);
		static void push(size_t queue, const rawrbox::Vector3f& pos, uint32_t color);

		static void createPipelines();
		static void ensureBuffer(size_t vertices);

	public:
		static void shutdown();

		// UTILS ----
		static void line(const rawrbox::Vector3f& a, const rawrbox::Vector3f& b, const rawrbox::Colorf& color, bool depthTest = true);
		static void triangle(const rawrbox::Vector3f& a, const rawrbox::Vector3f& b, const rawrbox::Vector3f& c, const rawrbox::Colorf& color, bool depthTest = true);
		static void quad(const rawrbox::Vector3f& a, const rawrbox::Vector3f& b, const rawrbox::Vector3f& c, const rawrbox::Vector3f& d, const rawrbox::Colorf& color, bool depthTest = true);
		static void aabb(const rawrbox::Vector3f& min, const rawrbox::Vector3f& max, const rawrbox::Colorf& color, bool depthTest = true);
		static void bbox(const rawrbox::Vector3f& pos, const rawrbox::BBOXf& box, const rawrbox::Colorf& color, bool depthTest = true);
		// ----------

		// Call inside PASS_WORLD
		static void draw();
	};
} // namespace rawrbox
