#pragma once

#include <rawrbox/math/vector3.hpp>
#include <rawrbox/render/model/sprite.hpp>
#include <rawrbox/render/texture/image.hpp>

#include <array>
#include <cmath>
#include <memory>
#include <string>
#include <unordered_map>

namespace rawrBox {

	class LightBase;
	class SoundInstance;

	class GIZMOS {
	protected:
		std::shared_ptr<rawrBox::Sprite<>> _gizmo_lights = std::make_shared<rawrBox::Sprite<>>();
		std::shared_ptr<rawrBox::Sprite<>> _gizmo_sounds = std::make_shared<rawrBox::Sprite<>>();

		std::unordered_map<std::string, std::shared_ptr<rawrBox::TextureImage>> _textures = {};

	public:
		void shutdown();
		void upload();

		// UTILS ----
		void addLight(rawrBox::LightBase* l);
		void removeLight(rawrBox::LightBase* l);

#ifdef RAWRBOX_BASS
		void addSound(rawrBox::SoundInstance* l);
		void removeSound(rawrBox::SoundInstance* l);
#endif
		// ------
		void updateGizmo(const std::string& id, const rawrBox::Vector3f& pos);
		void draw();

		static GIZMOS& get() {
			static GIZMOS cl;
			return cl;
		}
	};
} // namespace rawrBox
