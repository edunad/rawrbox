
#include <rawrbox/render/model/light/manager.hpp>

#include <fmt/format.h>

namespace rawrbox {
	// PRIVATE ----
	std::vector<std::unique_ptr<rawrbox::LightBase>> LIGHTS::_lights = {};
	// -----
	// PUBLIC ----
	bool LIGHTS::fullbright = false;

	void LIGHTS::shutdown() { _lights.clear(); }
	void LIGHTS::setEnabled(bool fb) { fullbright = fb; }

	// Light utils ----
	void LIGHTS::removeLight(rawrbox::LightBase* light) {
		if (light == nullptr || _lights.empty()) return;

		for (size_t i = 0; i < _lights.size(); i++) {
			if (_lights[i].get() == light) {
				_lights.erase(_lights.begin() + i);
				return;
			}
		}
	}

	const rawrbox::LightBase& LIGHTS::getLight(size_t indx) {
		if (indx < 0 || indx >= _lights.size()) throw std::runtime_error(fmt::format("[RawrBox-LIGHTS] Could not get light {}", indx));
		return *_lights[indx];
	}

	size_t LIGHTS::count() {
		return _lights.size();
	}
	// ---------

} // namespace rawrbox
