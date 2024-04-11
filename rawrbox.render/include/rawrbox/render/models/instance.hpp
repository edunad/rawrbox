#pragma once

#include <rawrbox/math/color.hpp>
#include <rawrbox/math/matrix4x4.hpp>
#include <rawrbox/math/utils/pack.hpp>

namespace rawrbox {
	struct Instance {
		rawrbox::Matrix4x4 matrix = {};
		rawrbox::Vector4u data = {0xFFFFFFFF, 0, 0, 0}; // Color, slice, gpu id, ??

		Instance() = default;
		Instance(const rawrbox::Matrix4x4& mat, const rawrbox::Colorf& col = rawrbox::Colors::White(), uint16_t slice = 0, uint32_t id = 0) : matrix(mat), data(col.pack(), 0, 0, 0) {
			this->setSlice(slice);
			if (id != 0) this->setId(id);
		}

		[[nodiscard]] rawrbox::Colorf getColor() const { return rawrbox::Color::RGBAHex(data.x); }
		void setColor(const rawrbox::Colorf& cl) { data.x = cl.pack(); }

		[[nodiscard]] const rawrbox::Matrix4x4& getMatrix() const { return matrix; }
		void setMatrix(const rawrbox::Matrix4x4& mtrx) { matrix = mtrx; }

		[[nodiscard]] uint32_t getSlice() const { return data.y; }
		void setSlice(uint32_t slice) { data.y = slice; }

		[[nodiscard]] uint32_t getId() const { return data.z; }
		void setId(uint32_t id) { data.z = (id << 8) | 0xFF; }
	};
} // namespace rawrbox
