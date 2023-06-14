#pragma once

#include <rawrbox/render/model/material/base.hpp>

namespace rawrbox {

	class MaterialText3DUnlit : public rawrbox::MaterialBase {
	public:
		using vertexBufferType = rawrbox::VertexData;
		MaterialText3DUnlit() = default;

		void upload() override;
	};

} // namespace rawrbox
