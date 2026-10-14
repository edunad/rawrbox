#pragma once

#include <rawrbox/math/vector2.hpp>
#include <rawrbox/render/lights/base.hpp>

namespace rawrbox {
	class RectLight : public rawrbox::LightBase {
	protected:
		rawrbox::Vector3f _right = {};
		rawrbox::Vector2f _size = {};

		virtual void orthonormalizeRight();

	public:
		RectLight(const rawrbox::Vector3f& pos, const rawrbox::Vector3f& direction, const rawrbox::Vector3f& right, const rawrbox::Colorf& color, const rawrbox::Vector2f& size, float radius);
		RectLight(const RectLight&) = delete;
		RectLight(RectLight&&) = delete;
		RectLight& operator=(const RectLight&) = delete;
		RectLight& operator=(RectLight&&) = delete;
		~RectLight() override = default;

		[[nodiscard]] rawrbox::LightType getType() const override;
		[[nodiscard]] rawrbox::Vector4f getData() const override; // xy = half size, zw = right axis

		void setDirection(const rawrbox::Vector3f& dir) override;

		[[nodiscard]] virtual const rawrbox::Vector3f& getRight() const;
		virtual void setRight(const rawrbox::Vector3f& right);

		[[nodiscard]] virtual const rawrbox::Vector2f& getSize() const;
		virtual void setSize(const rawrbox::Vector2f& size);
	};
} // namespace rawrbox