#include <rawrbox/render/textures/manager.hpp>

#include <fmt/format.h>

namespace rawrbox {
	// PROTECTED ---
	std::vector<Diligent::IDeviceObject*> TextureManager::_textureHandles = {};
	// --------------

	size_t TextureManager::registerTexture(rawrbox::TextureBase* texture) {
		auto* pTextureSRV = texture->getHandle(); // Get shader resource view from the texture
		if (pTextureSRV == nullptr) throw std::runtime_error(fmt::format("[RawrBox-TextureManager] Failed to register texture '{}'! Texture view is null, not uploaded?", texture->getName()));

		// Check if it's already registered --
		for (size_t slot = 0; slot < _textureHandles.size(); slot++) {
			if (_textureHandles[slot] == pTextureSRV) {
				fmt::print("[RawrBox-TextureManager] Registering bindless texture '{}' to slot '{}'\n", texture->getName(), slot);
				return slot;
			}
		}
		//-------------------------

		// First find a empty slot -----------
		for (size_t slot = 0; slot < _textureHandles.size(); slot++) {
			if (_textureHandles[slot] == nullptr) {
				_textureHandles[slot] = pTextureSRV;
				fmt::print("[RawrBox-TextureManager] Registering bindless texture '{}' to slot '{}'\n", texture->getName(), slot);
				return slot;
			}
		}
		// ----------------------------

		// No slot ---
		size_t slot = _textureHandles.size();
		_textureHandles.push_back(pTextureSRV);
		fmt::print("[RawrBox-TextureManager] Registering bindless texture '{}' to slot '{}'\n", texture->getName(), slot);
		//

		return slot;
	}

	void TextureManager::unregisterTexture(size_t indx) {
		if (indx >= _textureHandles.size()) throw std::runtime_error(fmt::format("[RawrBox-TextureManager] Index '{}' not found!", indx));
		_textureHandles[indx] = nullptr;

		fmt::print("[RawrBox-TextureManager] Un-registering bindless texture slot '{}'\n", indx);
	}

	const std::vector<Diligent::IDeviceObject*> TextureManager::getHandles() { return _textureHandles; }
} // namespace rawrbox
