#pragma once

#include <rawrbox/render/model/light/base.hpp>

#include <memory>

namespace rawrBox {

	class LightManager {
	protected:
		std::vector<std::shared_ptr<rawrBox::LightBase>> _lights;

	public:
		bool fullbright = false;

		virtual void destroy();
		virtual void setEnabled(bool fb);

		// Light utils ----
		virtual void addLight(std::shared_ptr<rawrBox::LightBase> light);
		virtual void removeLight(std::shared_ptr<rawrBox::LightBase> light);
		virtual std::shared_ptr<rawrBox::LightBase> getLight(size_t indx);

		virtual size_t count();
		// ---------

		static LightManager& get() {
			static LightManager i;
			return i;
		}

		LightManager() = default;
		LightManager(LightManager&&) = delete;
		LightManager& operator=(LightManager&&) = delete;
		LightManager(const LightManager&) = delete;
		LightManager& operator=(const LightManager&) = delete;
		virtual ~LightManager() = default;
	};
} // namespace rawrBox
