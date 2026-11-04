#include <rawrbox/render/utils/uniform_cache.hpp>

namespace rawrbox {
	std::array<uint64_t, RB_RENDER_MAX_CONTEXTS> UniformCache::_generation = {};

	void UniformCache::reset() {
		++_generation[rawrbox::RenderThreadPool::contextIndex()];
	}

	void UniformCache::clear() {
		if (rawrbox::RenderThreadPool::isRecording())
			RAWRBOX_CRITICAL("'UniformCache::clear' cannot be called inside 'parallelRender'");
		for (auto& generation : _generation)
			++generation;
	}
} // namespace rawrbox