#pragma once
#include <rawrbox/render/model/material/base.hpp>

namespace rawrbox {
	class MaterialParticle : public rawrbox::MaterialBase {
	public:
		using vertexBufferType = rawrbox::VertexBlendData;
		MaterialParticle() = default;

		void upload() override;
	};
} // namespace rawrbox
