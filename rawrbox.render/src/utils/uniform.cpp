#include <rawrbox/render/static.hpp>
#include <rawrbox/render/utils/barrier.hpp>
#include <rawrbox/render/utils/uniform.hpp>
#include <rawrbox/utils/logger.hpp>

namespace rawrbox {
	Diligent::RefCntAutoPtr<Diligent::IBuffer> UniformUtils::create(const char* name, const void* data, uint64_t size) {
		Diligent::BufferDesc desc;
		desc.Name = name;
		desc.Usage = Diligent::USAGE_DEFAULT;
		desc.BindFlags = Diligent::BIND_UNIFORM_BUFFER;
		desc.Size = size;

		const Diligent::BufferData initial(data, size);
		Diligent::RefCntAutoPtr<Diligent::IBuffer> buffer = {};
		rawrbox::RENDERER->device()->CreateBuffer(desc, &initial, &buffer);
		if (buffer == nullptr) RAWRBOX_CRITICAL("Failed to create the uniforms '{}'", name);

		rawrbox::BarrierUtils::barrier({{buffer, Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_CONSTANT_BUFFER, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE}});
		return buffer;
	}

	void UniformUtils::update(Diligent::IBuffer* buffer, const void* data, uint64_t size) {
		if (buffer == nullptr) RAWRBOX_CRITICAL("Invalid uniform buffer");

		rawrbox::BarrierUtils::barrier({{buffer, Diligent::RESOURCE_STATE_CONSTANT_BUFFER, Diligent::RESOURCE_STATE_COPY_DEST, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE}});
		rawrbox::RENDERER->context()->UpdateBuffer(buffer, 0, size, data, Diligent::RESOURCE_STATE_TRANSITION_MODE_VERIFY);
		rawrbox::BarrierUtils::barrier({{buffer, Diligent::RESOURCE_STATE_COPY_DEST, Diligent::RESOURCE_STATE_CONSTANT_BUFFER, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE}});
	}
} // namespace rawrbox
