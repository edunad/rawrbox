
#include <rawrbox/debug/gizmos.hpp>
#include <rawrbox/render/model/base.hpp>
#include <rawrbox/render/model/light/manager.hpp>

#include <fmt/format.h>

namespace rawrBox {
	void LightManager::destroy() {
		this->_lights.clear();
	}

	void LightManager::setEnabled(bool fb) { this->fullbright = fb; }

	// Light utils ----
	void LightManager::addLight(std::shared_ptr<rawrBox::LightBase> light) {
		if (light == nullptr || this->_lights.size() >= rawrBox::MAX_LIGHTS) return;

		GIZMOS::get().addLight(light);
		this->_lights.push_back(std::move(light));
	}

	void LightManager::removeLight(std::shared_ptr<rawrBox::LightBase> light) {
		if (this->_lights.empty()) return;

		for (size_t i = 0; i < this->_lights.size(); i++) {
			if (this->_lights[i] == light) {
				GIZMOS::get().removeLight(i);
				this->_lights.erase(this->_lights.begin() + i);
				return;
			}
		}
	}

	std::shared_ptr<rawrBox::LightBase> LightManager::getLight(size_t indx) {
		if (indx < 0 || indx >= this->_lights.size()) return nullptr;
		return this->_lights[indx];
	}

	size_t LightManager::count() {
		return this->_lights.size();
	}
	// ---------

} // namespace rawrBox
