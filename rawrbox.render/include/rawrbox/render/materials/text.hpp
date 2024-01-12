#pragma once

#include <rawrbox/render/materials/unlit.hpp>

namespace rawrbox {

	class MaterialText3D : public rawrbox::MaterialUnlit {
		static bool _build;

	public:
		using vertexBufferType = rawrbox::VertexData;

		MaterialText3D() = default;
		MaterialText3D(const MaterialText3D&) = delete;
		MaterialText3D(MaterialText3D&&) = delete;
		MaterialText3D& operator=(const MaterialText3D&) = delete;
		MaterialText3D& operator=(MaterialText3D&&) = delete;
		~MaterialText3D() override = default;

		void init() override;
	};

} // namespace rawrbox
