#pragma once
#include <rawrbox/render/materials/base.hpp>
#include <rawrbox/render/models/model.hpp>

namespace rawrbox {

	class Sprite : public rawrbox::Model {
	protected:
		bool _xAxis = true;
		bool _yAxis = true;
		bool _zAxis = true;

	public:
		Sprite() = default;
		Sprite(const Sprite&) = delete;
		Sprite(Sprite&&) = delete;
		Sprite& operator=(const Sprite&) = delete;
		Sprite& operator=(Sprite&&) = delete;
		~Sprite() override = default;

		[[nodiscard]] virtual bool xAxisEnabled() const;
		virtual void lockXAxix(bool locked);
		[[nodiscard]] virtual bool yAxisEnabled() const;
		virtual void lockYAxix(bool locked);
		[[nodiscard]] virtual bool zAxisEnabled() const;
		virtual void lockZAxix(bool locked);

		rawrbox::Mesh* addMesh(rawrbox::Mesh mesh) override;
	};
} // namespace rawrbox
