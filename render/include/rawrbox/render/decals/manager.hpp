#pragma once

#include <rawrbox/math/vector3.hpp>
#include <rawrbox/render/materials/decal.hpp>
#include <rawrbox/render/model/instanced.hpp>
#include <rawrbox/render/texture/base.hpp>

#include <vector>

// https://github.com/diharaw/deferred-decals/blob/master/src/main.cpp
namespace rawrbox {
	struct Decal {
		rawrbox::Vector3f pos = {};
		rawrbox::Vector3f dir = {0, 1, 0};

		uint16_t textureId = 0;
	};

	class DECALS {
	protected:
		static std::unique_ptr<rawrbox::InstancedModel<rawrbox::MaterialDecal>> _model;
		static std::vector<rawrbox::Decal> _decals;

	public:
		static void setAtlasTexture(rawrbox::TextureBase* atlas);
		static void addInstance(const rawrbox::Vector3f& pos, float direction = 0, const rawrbox::Colorf& color = rawrbox::Colors::White, uint16_t atlasId = 0);

		static void init();
		static void shutdown();

		static void draw();
	};
} // namespace rawrbox
