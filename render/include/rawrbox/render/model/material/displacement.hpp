#pragma once

#include <rawrbox/render/model/material/base.hpp>

namespace rawrbox {
	class MaterialDisplacement : public rawrbox::MaterialBase {
		bgfx::UniformHandle s_heightMap = BGFX_INVALID_HANDLE;

	public:
		using vertexBufferType = rawrbox::VertexData;

		MaterialDisplacement() = default;
		MaterialDisplacement(const MaterialDisplacement &) = delete;
		MaterialDisplacement(MaterialDisplacement &&) = delete;
		MaterialDisplacement &operator=(const MaterialDisplacement &) = delete;
		MaterialDisplacement &operator=(MaterialDisplacement &&) = delete;
		~MaterialDisplacement() override;

		template <typename T>
		void process(const rawrbox::Mesh<T> &mesh) {
			rawrbox::MaterialBase::process<T>(mesh);

			if (mesh.bumpTexture != nullptr && mesh.bumpTexture->valid() && !mesh.wireframe) {
				bgfx::setTexture(1, s_heightMap, mesh.bumpTexture->getHandle());
			} else {
				bgfx::setTexture(1, s_heightMap, rawrbox::BLACK_TEXTURE->getHandle());
			}
		}

		void registerUniforms() override;
		void upload() override;
	};

} // namespace rawrbox
