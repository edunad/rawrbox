#include <rawrbox/render/static.hpp>
#include <rawrbox/render/utils/barrier.hpp>

namespace rawrbox {
	// BARRIER ----
	std::unordered_map<Diligent::IDeviceObject*, Diligent::StateTransitionDesc> BarrierUtils::_barrierQueue = {};
	std::unordered_map<Diligent::IDeviceObject*, Diligent::RESOURCE_STATE> BarrierUtils::_barrierCache = {};
	std::vector<std::function<void()>> BarrierUtils::_barrierCallback = {};
	// -------------

	void BarrierUtils::processBarriers() {
		auto threadID = std::this_thread::get_id();
		if (threadID != rawrbox::RENDER_THREAD_ID) throw rawrbox::Logger::err("BarrierUtils", "Barriers can only be processed on the main render thread");

		if (_barrierQueue.empty()) return;

		// bulk all the barriers ---
		std::vector<Diligent::StateTransitionDesc> states = {};
		states.reserve(_barrierQueue.size());

		for (auto& barrier : _barrierQueue) {
			auto fnd = _barrierCache.find(barrier.first);
			if (fnd != _barrierCache.end() && fnd->second == barrier.second.NewState) {
				continue; //  Was already executed
			}

			states.push_back(barrier.second);
			_barrierCache[barrier.first] = barrier.second.NewState; // Cache it so its not executed again this frame
		}
		// ------------

		// Transition (aka execute barriers) ---
		if (!states.empty()) rawrbox::RENDERER->context()->TransitionResourceStates(static_cast<uint32_t>(states.size()), states.data());
		// ------------

		// Call all the callbacks ---
		for (auto& callback : _barrierCallback) {
			callback();
		}
		// --------------------------

		_barrierQueue.clear();
		_barrierCallback.clear();
	}

	void BarrierUtils::clearBarrierCache() {
		_barrierCache.clear();
	}
} // namespace rawrbox
