#pragma once

#include <rawrbox/render/materials/unlit.hpp>
#include <rawrbox/render/models/base.hpp>

#include <map>
#include <vector>

namespace rawrbox {
	class DebugDraw {
	public:
		struct Stats {
			size_t buckets = 0;
			size_t points = 0;
			size_t uploads = 0;
			size_t drawCalls = 0;
		};

	protected:
		static constexpr size_t NO_BATCH = static_cast<size_t>(-1);
		static constexpr uint64_t IDLE_REMOVE_FRAMES = 600;

		struct BucketKey {
			bool onTop = false;
			bool line = false;

			uint32_t rgba = 0;
			auto operator<=>(const BucketKey&) const = default;
		};

		struct Bucket {
			rawrbox::Colorf color = {};
			std::vector<rawrbox::Vector3f> points = {};

			uint64_t hash = 0; // Check if rebuilding needs done

			size_t batch = NO_BATCH;
			uint64_t lastFrame = 0;
		};

		static std::map<BucketKey, Bucket> _buckets;

		static BucketKey _lastKey;
		static Bucket* _lastBucket;

		static uint64_t _frame;
		static Stats _stats;

		static Bucket& bucket(const rawrbox::Colorf& color, bool line, bool onTop);
		static void drawBuckets();

	public:
		static void shutdown();

		static void line(const rawrbox::Vector3f& a, const rawrbox::Vector3f& b, const rawrbox::Colorf& color, bool onTop = false);
		static void triangle(const rawrbox::Vector3f& a, const rawrbox::Vector3f& b, const rawrbox::Vector3f& c, const rawrbox::Colorf& color, bool onTop = false);
		static void quad(const rawrbox::Vector3f& a, const rawrbox::Vector3f& b, const rawrbox::Vector3f& c, const rawrbox::Vector3f& d, const rawrbox::Colorf& color, bool onTop = false);
		static void aabb(const rawrbox::Vector3f& min, const rawrbox::Vector3f& max, const rawrbox::Colorf& color, bool onTop = false);

		// Call inside PASS_WORLD
		static void draw();

		[[nodiscard]] static const Stats& stats();
	};
} // namespace rawrbox
