#include <rawrbox/render/static.hpp>
#include <rawrbox/render/utils/barrier.hpp>

namespace rawrbox {
	// BARRIER ----
	std::unordered_map<Diligent::IDeviceObject*, Diligent::StateTransitionDesc> BarrierUtils::_barrierCache = {};
	//  -------------

	void BarrierUtils::barrier(const std::vector<Diligent::StateTransitionDesc>& resources) {
		if (std::this_thread::get_id() != rawrbox::RENDER_THREAD_ID) throw rawrbox::Logger::err("BarrierUtils", "Barriers can only be processed on the main render thread");

		std::vector<Diligent::StateTransitionDesc> states = {};
		for (auto barrier : resources) {
			auto fnd = _barrierCache.find(barrier.pResource);
			if (fnd != _barrierCache.end() && fnd->second.NewState == barrier.NewState) continue; //  Was already executed

			states.emplace_back(barrier);
			_barrierCache[barrier.pResource] = barrier;
		}

		if (!states.empty()) rawrbox::RENDERER->context()->TransitionResourceStates(static_cast<uint32_t>(states.size()), states.data());
	}

	void BarrierUtils::clearBarrierCache() {
		_barrierCache.clear();
	}
} // namespace rawrbox
