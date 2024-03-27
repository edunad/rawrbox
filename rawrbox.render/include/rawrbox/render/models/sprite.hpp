#pragma once
#include <rawrbox/render/materials/unlit.hpp>
#include <rawrbox/render/models/model.hpp>

namespace rawrbox {

	template <typename M = rawrbox::MaterialUnlit>
		requires(std::derived_from<M, rawrbox::MaterialBase>)
	class Sprite : public rawrbox::Model<M> {
	protected:
		bool _xAxis = true;
		bool _yAxis = true;

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

		rawrbox::Mesh<typename M::vertexBufferType>* addMesh(rawrbox::Mesh<typename M::vertexBufferType> mesh) override {
			mesh.setOptimizable(false);

			uint32_t billboard = 0;
			if (this->_xAxis) billboard |= rawrbox::MeshBilldboard::X;
			if (this->_yAxis) billboard |= rawrbox::MeshBilldboard::Y;

			mesh.setBillboard(billboard);

			return Model<M>::addMesh(mesh);
		}
	};
} // namespace rawrbox
