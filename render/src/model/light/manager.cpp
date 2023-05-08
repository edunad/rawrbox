
#ifdef RAWRBOX_DEBUG
	#include <rawrbox/debug/gizmos.hpp>
#endif

#include <rawrbox/render/model/base.hpp>
#include <rawrbox/render/model/light/manager.hpp>

#include <fmt/format.h>

namespace rawrbox {
	void LightManager::destroy() {
		this->_lights.clear();
	}

	void LightManager::setEnabled(bool fb) { this->fullbright = fb; }

	// Light utils ----
	void LightManager::addLight(std::shared_ptr<rawrbox::LightBase> light) {
		if (light == nullptr || this->_lights.size() >= rawrbox::MAX_LIGHTS) return;
		this->_lights.push_back(std::move(light));
	}

	void LightManager::removeLight(std::shared_ptr<rawrbox::LightBase> light) {
		if (this->_lights.empty()) return;

		for (size_t i = 0; i < this->_lights.size(); i++) {
			if (this->_lights[i] == light) {
				this->_lights.erase(this->_lights.begin() + i);
				return;
			}
		}
	}

	std::shared_ptr<rawrbox::LightBase> LightManager::getLight(size_t indx) {
		if (indx < 0 || indx >= this->_lights.size()) return nullptr;
		return this->_lights[indx];
	}

	size_t LightManager::count() {
		return this->_lights.size();
	}
	// ---------

} // namespace rawrbox
