#pragma once

#include <rawrbox/render/model/light/base.hpp>
#include <rawrbox/render/static.hpp>

#include <memory>
#include <vector>

namespace rawrbox {
	class LIGHTS {
	protected:
		static std::vector<std::unique_ptr<rawrbox::LightBase>> _lights;

	public:
		static bool fullbright;

		static void shutdown();
		static void setEnabled(bool fb);

		// Light ----
		template <typename T = rawrbox::LightBase>
		static rawrbox::LightBase* addLight(T light) {
			if (_lights.size() >= rawrbox::MAX_LIGHTS) {
				fmt::print("[RawrBox-LIGHTS] Could not add light, max lights limit hit!\n");
				return nullptr;
			}

			light.setId(++rawrbox::LIGHT_ID);
			return _lights.emplace_back(std::make_unique<T>(light)).get();
		}

		static void removeLight(rawrbox::LightBase* light);
		// ---------

		// Light utils ----
		static const rawrbox::LightBase& getLight(size_t indx);
		static size_t count();
		// ---------
	};
} // namespace rawrbox
