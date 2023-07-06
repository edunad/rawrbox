#pragma once
#include <rawrbox/render/light/base.hpp>

namespace rawrbox {
	class LightDirectional : public rawrbox::LightBase {
	protected:
		rawrbox::Vector3f _direction;

	public:
		LightDirectional(rawrbox::Vector3f posMatrix, rawrbox::Vector3f dir, rawrbox::Colorf diffuse, rawrbox::Colorf specular) : rawrbox::LightBase(posMatrix, diffuse, specular), _direction(dir){};
		LightDirectional(const LightDirectional&) = default;
		LightDirectional(LightDirectional&&) = delete;
		LightDirectional& operator=(const LightDirectional&) = default;
		LightDirectional& operator=(LightDirectional&&) = delete;
		~LightDirectional() override = default;

		[[nodiscard]] const rawrbox::LightType getType() const override { return LightType::LIGHT_DIR; };
		[[nodiscard]] const rawrbox::Vector3f& getDirection() const { return _direction; };
	};
} // namespace rawrbox
