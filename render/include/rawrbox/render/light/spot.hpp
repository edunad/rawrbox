#pragma once
#include <rawrbox/render/light/base.hpp>

namespace rawrbox {
	class LightSpot : public rawrbox::LightBase {
	protected:
		float _innerCone;
		float _outerCone;

	public:
		LightSpot(const rawrbox::Vector3f& posMatrix, const rawrbox::Vector3f& direction, const rawrbox::Colorf& color, float innerCone, float outerCone, float power) : rawrbox::LightBase(posMatrix, color, power), _innerCone(innerCone), _outerCone(outerCone) {
			this->_direction = direction;
		};

		LightSpot(const LightSpot&) = default;
		LightSpot(LightSpot&&) = delete;
		LightSpot& operator=(const LightSpot&) = default;
		LightSpot& operator=(LightSpot&&) = delete;
		~LightSpot() override = default;

		[[nodiscard]] const rawrbox::LightType getType() const override { return LightType::LIGHT_SPOT; };
		[[nodiscard]] const rawrbox::Vector4f getData() const override { return {_innerCone, _outerCone, 0, 0}; }

		[[nodiscard]] const float getInnerCone() const { return _innerCone; };
		[[nodiscard]] const float getOuterCone() const { return _outerCone; };
	};
} // namespace rawrbox
