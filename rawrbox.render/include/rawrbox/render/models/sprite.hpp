#pragma once
#include <rawrbox/render/materials/unlit.hpp>
#include <rawrbox/render/models/model.hpp>

namespace rawrbox {

	template <typename M = rawrbox::MaterialUnlit>
	class Sprite : public rawrbox::Model<M> {
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

		[[nodiscard]] virtual bool xAxisEnabled() const { return this->_xAxis; }
		virtual void lockXAxix(bool locked) { this->_xAxis = !locked; }
		[[nodiscard]] virtual bool yAxisEnabled() const { return this->_yAxis; }
		virtual void lockYAxix(bool locked) { this->_yAxis = !locked; }
		[[nodiscard]] virtual bool zAxisEnabled() const { return this->_zAxis; }
		virtual void lockZAxix(bool locked) { this->_zAxis = !locked; }

		rawrbox::Mesh<typename M::vertexBufferType>* addMesh(rawrbox::Mesh<typename M::vertexBufferType> mesh) override {
			mesh.setOptimizable(false);
			mesh.addData("billboard_mode", {this->_xAxis ? 1.F : 0.F, this->_yAxis ? 1.F : 0.F, this->_zAxis ? 1.F : 0.F, 0.F});

			return Model<M>::addMesh(mesh);
		}
	};
} // namespace rawrbox
