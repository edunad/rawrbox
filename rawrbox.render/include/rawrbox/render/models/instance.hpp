#pragma once

#include <rawrbox/math/color.hpp>
#include <rawrbox/math/matrix4x4.hpp>
#include <rawrbox/math/utils/pack.hpp>

namespace rawrbox {
	struct Instance {
		rawrbox::Matrix4x4 matrix = {};
		uint32_t color = 0xFFFFFFFF;
		rawrbox::Vector4f extraData = {}; // AtlasID, etc..

		Instance() = default;
		Instance(const rawrbox::Matrix4x4& mat, const rawrbox::Colorf& col = rawrbox::Colors::White(), uint16_t atlasId = 0, uint32_t id = 0) : matrix(mat), color(col.pack()) {
			this->setAtlasId(atlasId);
			if (id != 0) this->setId(id);
		}

		[[nodiscard]] rawrbox::Colorf getColor() const { return rawrbox::Color::RGBAHex(color); }
		void setColor(const rawrbox::Colorf& cl) { color = cl.pack(); }

		[[nodiscard]] const rawrbox::Matrix4x4& getMatrix() const { return matrix; }
		void setMatrix(const rawrbox::Matrix4x4& mtrx) { matrix = mtrx; }

		[[nodiscard]] uint16_t getAtlasId() const { return static_cast<uint16_t>(extraData.w); }
		void setAtlasId(uint16_t id) { extraData.w = static_cast<float>(id); }

		[[nodiscard]] uint32_t getId() const {
			return rawrbox::PackUtils::toABGR(extraData.x, extraData.y, extraData.z, 1.F);
		}

		void setId(uint32_t id) {
			auto pack = rawrbox::PackUtils::fromABGR(0xFF000000 | id);

			extraData.x = pack[0];
			extraData.y = pack[1];
			extraData.z = pack[2];
		}
	};
} // namespace rawrbox
