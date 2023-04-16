
#include <rawrbox/render/model/light/manager.h>

namespace rawrBox {
	bool LightManager::FULLBRIGHT = false;

	// Light utils ----
	void LightManager::addLight(const std::shared_ptr<rawrBox::LightBase>& light) {
		if (light == nullptr) return;

		light->indx = static_cast<uint32_t>(this->_lights.size());
		this->_lights.push_back(std::move(light));
	}

	void LightManager::removeLight(size_t indx) {
		if (indx < 0 || indx >= this->_lights.size()) return;
		this->_lights.erase(this->_lights.begin() + indx);
	}

	std::shared_ptr<rawrBox::LightBase> LightManager::getLight(size_t indx) {
		if (indx < 0 || indx >= this->_lights.size()) return nullptr;
		return this->_lights[indx];
	}
	// ---------

	void LightManager::clear() {
		this->_lights.clear();
	}

	size_t LightManager::count() {
		return this->_lights.size();
	}

} // namespace rawrBox
