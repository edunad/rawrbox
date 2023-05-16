#pragma once

#include <rawrbox/render/model/light/base.hpp>

#include <memory>

namespace rawrbox {

	class LIGHTS {
	protected:
		static std::vector<std::shared_ptr<rawrbox::LightBase>> _lights;

	public:
		static bool fullbright;

		static void destroy();
		static void setEnabled(bool fb);

		// Light utils ----
		static void addLight(std::shared_ptr<rawrbox::LightBase> light);
		static void removeLight(std::shared_ptr<rawrbox::LightBase> light);
		static std::shared_ptr<rawrbox::LightBase> getLight(size_t indx);

		static size_t count();
		// ---------
	};
} // namespace rawrbox
