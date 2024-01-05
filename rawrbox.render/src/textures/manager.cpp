#include <rawrbox/render/static.hpp>
#include <rawrbox/render/textures/manager.hpp>

#include <fmt/format.h>

namespace rawrbox {
	// PROTECTED ---
	std::vector<Diligent::IDeviceObject*> TextureManager::_textureHandles = {};
	// --------------

	// PUBLIC -------
	rawrbox::Event<> TextureManager::onUpdate = {};
	// --------------

	uint32_t TextureManager::registerTexture(rawrbox::TextureBase* texture) {
		auto* pTextureSRV = texture->getHandle(); // Get shader resource view from the texture
		if (pTextureSRV == nullptr) throw std::runtime_error(fmt::format("[RawrBox-TextureManager] Failed to register texture '{}'! Texture view is null, not uploaded?", texture->getName()));

		// Check if it's already registered --
		for (size_t slot = 0; slot < _textureHandles.size(); slot++) {
			if (_textureHandles[slot] == pTextureSRV) {
				return static_cast<uint32_t>(slot);
			}
		}
		//-------------------------

		// First find a empty slot -----------
		for (size_t slot = 0; slot < _textureHandles.size(); slot++) {
			if (_textureHandles[slot] == nullptr) {
				_textureHandles[slot] = pTextureSRV;
				fmt::print("[RawrBox-TextureManager] Re-using slot '{}' for bindless texture '{}'\n", slot, texture->getName());
				return static_cast<uint32_t>(slot);
			}
		}
		// ----------------------------

		// No slot ---
		auto slot = static_cast<uint32_t>(_textureHandles.size());
		if (slot == static_cast<uint32_t>(rawrbox::RENDERER->MAX_TEXTURES / 1.2F)) {
			fmt::print("[RawrBox-TextureManager] Aproaching max texture limit of '{}'\n", rawrbox::RENDERER->MAX_TEXTURES);
			return slot;
		}

		if (slot >= rawrbox::RENDERER->MAX_TEXTURES) {
			throw std::runtime_error(fmt::format("[RawrBox-TextureManager] Max texture limit reached! Cannot allocate texture, remove some unecessary textures or increase MAX_TEXTURES on renderer\n", rawrbox::RENDERER->MAX_TEXTURES));
		}

		_textureHandles.push_back(pTextureSRV);
		fmt::print("[RawrBox-TextureManager] Registering bindless texture '{}' to slot '{}'\n", texture->getName(), slot);
		// -----

		onUpdate();
		return slot;
	}

	void TextureManager::unregisterTexture(uint32_t indx) {
		if (indx >= _textureHandles.size()) throw std::runtime_error(fmt::format("[RawrBox-TextureManager] Index '{}' not found!", indx));
		_textureHandles[indx] = nullptr;
		fmt::print("[RawrBox-TextureManager] Un-registering bindless texture slot '{}'\n", indx);

		onUpdate();
	}

	const std::vector<Diligent::IDeviceObject*>& TextureManager::getHandles() { return _textureHandles; }
	size_t TextureManager::total() { return _textureHandles.size(); }
} // namespace rawrbox
