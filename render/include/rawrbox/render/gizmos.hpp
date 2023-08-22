#pragma once

#include <rawrbox/math/vector3.hpp>
#include <rawrbox/render/model/sprite.hpp>
#include <rawrbox/render/texture/image.hpp>

namespace rawrbox {
	class GIZMOS {
	protected:
		static std::unique_ptr<rawrbox::Sprite> _gizmos;
		static std::unordered_map<std::string, std::unique_ptr<rawrbox::TextureImage>> _textures;
		static uint32_t _ID;

	public:
		static void shutdown();

		// UTILS ----
		static uint32_t addGizmo(const std::string& type, const rawrbox::Vector3f& pos);
		static void removeGizmo(uint32_t id);
		static void updateGizmo(uint32_t id, const rawrbox::Vector3f& pos);
		// ------

		static void draw();
	};
} // namespace rawrbox
