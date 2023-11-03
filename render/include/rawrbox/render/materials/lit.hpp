#pragma once

#include <rawrbox/render/materials/unlit.hpp>
#include <rawrbox/render/renderers/cluster.hpp>

namespace rawrbox {

	struct MaterialLitUniforms : public rawrbox::MaterialUnlitUniforms, public rawrbox::ClusterConstants {};

	class MaterialLit : public rawrbox::MaterialBase {
		static Diligent::RefCntAutoPtr<Diligent::IBuffer> _uniforms;

	protected:
		void prepareMaterial() override;

	public:
		using vertexBufferType = rawrbox::VertexNormData;

		MaterialLit() = default;
		MaterialLit(MaterialLit&&) = delete;
		MaterialLit& operator=(MaterialLit&&) = delete;
		MaterialLit(const MaterialLit&) = delete;
		MaterialLit& operator=(const MaterialLit&) = delete;
		~MaterialLit() override = default;

		static void init();

		template <typename T = rawrbox::VertexData>
		void bindUniforms(const rawrbox::Mesh<T>& mesh) {
			auto context = rawrbox::RENDERER->context();

			// SETUP UNIFORMS ----------------------------
			Diligent::MapHelper<rawrbox::MaterialLitUniforms> CBConstants(context, this->_uniforms, Diligent::MAP_WRITE, Diligent::MAP_FLAG_DISCARD);
			this->bindBaseUniforms<T, rawrbox::MaterialLitUniforms>(mesh, CBConstants);
			// ------------

			// Bind renderer uniforms ---------
			rawrbox::RENDERER->bindUniforms<rawrbox::MaterialLitUniforms>(CBConstants);
			// --------------------------------
		}
	};

} // namespace rawrbox
