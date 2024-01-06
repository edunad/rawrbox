#pragma once

#include <rawrbox/render/textures/base.hpp>
#include <rawrbox/utils/event.hpp>

#include <RefCntAutoPtr.hpp>

#include <ShaderResourceBinding.h>

// https://github.com/DiligentGraphics/DiligentSamples/tree/master/Tutorials/Tutorial16_BindlessResources

namespace rawrbox {
	class TextureManager {
	protected:
		static std::vector<Diligent::IDeviceObject*> _textureHandles;

	public:
		static rawrbox::Event<> onUpdate;

		static uint32_t registerTexture(rawrbox::TextureBase* texture);
		static void unregisterTexture(uint32_t indx);

		static const std::vector<Diligent::IDeviceObject*>& getHandles();
		static size_t total();
	};
} // namespace rawrbox
