#pragma once

#include <rawrbox/math/vector3.hpp>
#include <rawrbox/render_temp/model/instanced.hpp>
#include <rawrbox/render_temp/texture/base.hpp>

namespace rawrbox {
	class DECALS {
	protected:
		static std::unique_ptr<rawrbox::InstancedModel> _model;

	public:
		static void setAtlasTexture(rawrbox::TextureBase* atlas);

		// UTILS -----
		static bool add(const rawrbox::Vector3f& pos, const rawrbox::Vector3f& scale = {1, 1, 1}, float direction = 0, const rawrbox::Colorf& color = rawrbox::Colors::White(), uint16_t atlasId = 0);
		static bool remove(size_t i);
		static const rawrbox::Instance& get(size_t i);
		static const rawrbox::Mesh& getTemplate();
		static size_t count();
		// -----

		static void init();
		static void shutdown();

		static void draw();
	};
} // namespace rawrbox