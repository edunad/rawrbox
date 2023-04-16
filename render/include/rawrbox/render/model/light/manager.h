#pragma once

#include <rawrbox/render/model/light/base.hpp>

namespace rawrBox {
	class LightManager {
		std::vector<std::shared_ptr<rawrBox::LightBase>> _lights;

	public:
		static bool FULLBRIGHT;

		// Light utils ----
		void addLight(const std::shared_ptr<rawrBox::LightBase>& light);
		void removeLight(size_t indx);
		std::shared_ptr<rawrBox::LightBase> getLight(size_t indx);
		// ---------

		void clear();
		size_t count();

		static LightManager& getInstance() {
			static LightManager i;
			return i;
		}
	};
} // namespace rawrBox
