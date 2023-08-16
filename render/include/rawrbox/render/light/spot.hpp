#pragma once
#include <rawrbox/render/light/base.hpp>

namespace rawrbox {
	class SpotLight : public rawrbox::LightBase {
	protected:
		float _innerCone;
		float _outerCone;

	public:
		SpotLight(const rawrbox::Vector3f& posMatrix, const rawrbox::Vector3f& direction, const rawrbox::Colorf& color, float innerCone, float outerCone, float power) : rawrbox::LightBase(posMatrix, color, power), _innerCone(innerCone), _outerCone(outerCone) {
			this->_direction = direction.normalized();
		};

		SpotLight(const SpotLight&) = delete;
		SpotLight(SpotLight&&) = delete;
		SpotLight& operator=(const SpotLight&) = delete;
		SpotLight& operator=(SpotLight&&) = delete;
		~SpotLight() override = default;

		[[nodiscard]] rawrbox::LightType getType() const override { return LightType::SPOT; };
		[[nodiscard]] const rawrbox::Vector4f getData() const override { return {_innerCone, _outerCone, 0, 0}; }

		[[nodiscard]] float getInnerCone() const { return _innerCone; };
		[[nodiscard]] float getOuterCone() const { return _outerCone; };
	};
} // namespace rawrbox
