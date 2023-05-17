
#include <rawrbox/render/model/base.hpp>
#include <rawrbox/render/model/light/base.hpp>
#include <rawrbox/render/model/light/manager.hpp>

#ifdef RAWRBOX_DEBUG
	#ifndef RAWRBOX_TESTING
		#include <rawrbox/debug/gizmos.hpp>
	#endif
#endif

#include <fmt/format.h>

namespace rawrbox {
	std::vector<std::shared_ptr<rawrbox::LightBase>> LIGHTS::_lights = {};
	bool LIGHTS::fullbright = false;

	void LIGHTS::destroy() { _lights.clear(); }
	void LIGHTS::setEnabled(bool fb) { fullbright = fb; }

	// Light utils ----
	void LIGHTS::addLight(std::shared_ptr<rawrbox::LightBase> light) {
		if (light == nullptr || _lights.size() >= rawrbox::MAX_LIGHTS) return;
		light->setId(++rawrbox::LIGHT_ID);

#ifdef RAWRBOX_DEBUG
	#ifndef RAWRBOX_TESTING
		rawrbox::GIZMOS::addLight(light.get());
	#endif
#endif
		_lights.push_back(std::move(light));
	}

	void LIGHTS::removeLight(std::shared_ptr<rawrbox::LightBase> light) {
		removeLight(light.get());
	}

	void LIGHTS::removeLight(rawrbox::LightBase* light) {
		if (_lights.empty()) return;

#ifdef RAWRBOX_DEBUG
	#ifndef RAWRBOX_TESTING
		rawrbox::GIZMOS::removeLight(light);
	#endif
#endif

		for (size_t i = 0; i < _lights.size(); i++) {
			if (_lights[i].get() == light) {
				_lights.erase(_lights.begin() + i);
				return;
			}
		}
	}

	std::shared_ptr<rawrbox::LightBase> LIGHTS::getLight(size_t indx) {
		if (indx < 0 || indx >= _lights.size()) return nullptr;
		return _lights[indx];
	}

	size_t LIGHTS::count() {
		return _lights.size();
	}
	// ---------

} // namespace rawrbox
