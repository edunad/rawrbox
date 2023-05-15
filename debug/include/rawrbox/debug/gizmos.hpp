#pragma once

#include <rawrbox/math/vector3.hpp>
#include <rawrbox/render/model/sprite.hpp>
#include <rawrbox/render/texture/image.hpp>

#include <array>
#include <cmath>
#include <memory>
#include <string>
#include <unordered_map>

namespace rawrbox {

#ifdef RAWRBOX_BASS
	class SoundInstance;
#endif

	class Emitter;
	class LightBase;
	class Gizmos {
	protected:
		std::shared_ptr<rawrbox::Sprite<>> _gizmo_lights = std::make_shared<rawrbox::Sprite<>>();
		std::shared_ptr<rawrbox::Sprite<>> _gizmo_sounds = std::make_shared<rawrbox::Sprite<>>();
		std::shared_ptr<rawrbox::Sprite<>> _gizmo_emitters = std::make_shared<rawrbox::Sprite<>>();

		std::unordered_map<std::string, std::shared_ptr<rawrbox::TextureImage>> _textures = {};

	public:
		void shutdown();
		void upload();

		// UTILS ----
		void addLight(rawrbox::LightBase* l);
		void removeLight(rawrbox::LightBase* l);

		void addEmitter(rawrbox::Emitter* l);
		void removeEmitter(rawrbox::Emitter* l);

#ifdef RAWRBOX_BASS
		void addSound(rawrbox::SoundInstance* l);
		void removeSound(rawrbox::SoundInstance* l);
#endif
		// ------
		void updateGizmo(const std::string& id, const rawrbox::Vector3f& pos);
		void draw();
	};
} // namespace rawrbox
