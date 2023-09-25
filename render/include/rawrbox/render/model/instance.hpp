#pragma once

#include <rawrbox/math/color.hpp>
#include <rawrbox/math/matrix4x4.hpp>
#include <rawrbox/utils/pack.hpp>

#include <bgfx/bgfx.h>

namespace rawrbox {
	struct Instance {
		rawrbox::Matrix4x4 matrix = {};
		rawrbox::Colorf color = rawrbox::Colors::White();
		rawrbox::Vector4f extraData = {}; // AtlasID, etc..

		Instance() = default;
		Instance(const rawrbox::Matrix4x4& mat, const rawrbox::Colorf& col = rawrbox::Colors::White(), uint16_t atlasId = 0, uint32_t id = 0) : matrix(mat), color(col) {
			this->setAtlasId(atlasId);
			if (id != 0) this->setId(id);
		}

		rawrbox::Colorf getColor() { return color; }
		void setColor(const rawrbox::Colorf& cl) { color = cl; }

		const rawrbox::Matrix4x4& getMatrix() { return matrix; }
		void setMatrix(const rawrbox::Matrix4x4& mtrx) { matrix = mtrx; }

		uint16_t getAtlasId() { return static_cast<uint16_t>(extraData.x); }
		void setAtlasId(uint16_t id) { extraData.x = static_cast<float>(id); }

		uint32_t getId() {
			return rawrbox::PackUtils::toABGR(extraData.y, extraData.z, extraData.w, 1.F);
		}

		void setId(uint32_t id) {
			auto pack = rawrbox::PackUtils::fromRGBA(0xFF000000 | id);

			extraData.y = pack[1];
			extraData.z = pack[2];
			extraData.w = pack[3];
		}

		static bgfx::VertexLayout vLayout() {
			static bgfx::VertexLayout l;
			l.begin()
			    .add(bgfx::Attrib::TexCoord0, 4, bgfx::AttribType::Float) // Position
			    .add(bgfx::Attrib::TexCoord1, 4, bgfx::AttribType::Float)
			    .add(bgfx::Attrib::TexCoord2, 4, bgfx::AttribType::Float)
			    .add(bgfx::Attrib::TexCoord3, 4, bgfx::AttribType::Float)
			    .add(bgfx::Attrib::TexCoord4, 4, bgfx::AttribType::Float) // Color
			    .add(bgfx::Attrib::TexCoord5, 4, bgfx::AttribType::Float) // Atlas & GPU picking
			    .end();
			return l;
		};
	};
} // namespace rawrbox
