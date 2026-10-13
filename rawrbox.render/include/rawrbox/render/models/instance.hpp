#pragma once

#include <rawrbox/math/color.hpp>
#include <rawrbox/math/matrix4x4.hpp>
#include <rawrbox/math/utils/pack.hpp>

#include <array>

namespace rawrbox {
	struct Instance {
		std::array<rawrbox::Vector4f, 3> transform = {
		    rawrbox::Vector4f{1.F, 0.F, 0.F, 0.F},
		    rawrbox::Vector4f{0.F, 1.F, 0.F, 0.F},
		    rawrbox::Vector4f{0.F, 0.F, 1.F, 0.F}};

		rawrbox::Vector4u data = {0xFFFFFFFF, 0, 0, 0}; // Color, slice, gpu id, ??

		Instance() = default;
		Instance(const rawrbox::Matrix4x4& mat, const rawrbox::Colorf& col = rawrbox::Colors::White(), uint16_t slice = 0, uint32_t id = 0) : data(col.pack(), 0, 0, 0) {
			this->setMatrix(mat);
			this->setSlice(slice);
			if (id != 0) this->setId(id);
		}

		[[nodiscard]] rawrbox::Colorf getColor() const { return rawrbox::Color::RGBAHex(data.x); }
		void setColor(const rawrbox::Colorf& cl) { data.x = cl.pack(); }

		[[nodiscard]] rawrbox::Matrix4x4 getMatrix() const {
			return rawrbox::Matrix4x4{std::array<float, 16>{
			    this->transform[0].x, this->transform[1].x, this->transform[2].x, 0.F,
			    this->transform[0].y, this->transform[1].y, this->transform[2].y, 0.F,
			    this->transform[0].z, this->transform[1].z, this->transform[2].z, 0.F,
			    this->transform[0].w, this->transform[1].w, this->transform[2].w, 1.F}};
		}

		void setMatrix(const rawrbox::Matrix4x4& mtrx) {
			for (size_t i = 0; i < this->transform.size(); i++) {
				this->transform[i] = {mtrx[i], mtrx[4 + i], mtrx[8 + i], mtrx[12 + i]};
			}
		}

		[[nodiscard]] uint32_t getSlice() const { return data.y; }
		void setSlice(const uint32_t slice) { data.y = slice; }

		[[nodiscard]] uint32_t getId() const { return data.z; }
		void setId(const uint32_t id) { data.z = (id << 8) | 0xFF; }
	};
} // namespace rawrbox