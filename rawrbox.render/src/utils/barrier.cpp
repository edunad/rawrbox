#include <rawrbox/render/static.hpp>
#include <rawrbox/render/utils/barrier.hpp>

#include <RefCntAutoPtr.hpp>

#include <Buffer.h>
#include <Texture.h>

namespace rawrbox {
	// BARRIER ----
	std::unique_ptr<rawrbox::Logger> BarrierUtils::_logger = std::make_unique<rawrbox::Logger>("RawrBox-BarrierUtils");
	//  -------------

	void BarrierUtils::barrier(const std::vector<Diligent::StateTransitionDesc>& resources) {
		if (std::this_thread::get_id() != rawrbox::RENDER_THREAD_ID) RAWRBOX_CRITICAL("Barriers can only be processed on the main render thread");

		std::vector<Diligent::StateTransitionDesc> states = {};
		for (const auto& barrier : resources) {
			Diligent::RESOURCE_STATE current = Diligent::RESOURCE_STATE_UNKNOWN;

			Diligent::RefCntAutoPtr<Diligent::ITexture> texture(barrier.pResource, Diligent::IID_Texture);
			if (texture != nullptr) {
				current = texture->GetState();
			} else {
				Diligent::RefCntAutoPtr<Diligent::IBuffer> buffer(barrier.pResource, Diligent::IID_Buffer);
				if (buffer != nullptr) current = buffer->GetState();
			}

			if (current == barrier.NewState) continue; //  Was already executed
			states.emplace_back(barrier);
		}

		if (!states.empty()) rawrbox::RENDERER->context()->TransitionResourceStates(static_cast<uint32_t>(states.size()), states.data());
	}
} // namespace rawrbox
