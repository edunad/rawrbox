#pragma once

#include <rawrbox/render/model/light/base.hpp>
#include <rawrbox/render/model/sprite.hpp>
#include <rawrbox/render/texture/image.hpp>

#include <memory>

namespace rawrBox {

	class LightManager {
	protected:
		std::vector<std::shared_ptr<rawrBox::LightBase>> _lights;

		std::shared_ptr<rawrBox::TextureImage> _pointTexture;
		std::shared_ptr<rawrBox::TextureImage> _spotTexture;
		std::shared_ptr<rawrBox::TextureImage> _dirTexture;

		std::shared_ptr<rawrBox::Sprite> _debugMdl;

	public:
		bool fullbright = false;
		int32_t maxLights = 8;

		virtual void init(int32_t maxLights = 8);
		virtual void destroy();

		virtual void setEnabled(bool fb);

		// Light utils ----
		virtual void addLight(std::shared_ptr<rawrBox::LightBase> light);
		virtual void removeLight(std::shared_ptr<rawrBox::LightBase> light);
		virtual std::shared_ptr<rawrBox::LightBase> getLight(size_t indx);

		virtual size_t count();
		// ---------

		// DEBUG ----
		virtual void uploadDebug();
		virtual void drawDebug(const rawrBox::Vector3f& camPos);
		// ---

		static LightManager& getInstance() {
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
