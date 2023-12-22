#pragma once

#include <rawrbox/render/materials/unlit.hpp>

namespace rawrbox {

	class MaterialInstanced : public rawrbox::MaterialUnlit {
		static Diligent::RefCntAutoPtr<Diligent::IBuffer> _uniforms;
		static bool _built;

	public:
		using vertexBufferType = rawrbox::VertexData;

	public:
		MaterialInstanced() = default;
		MaterialInstanced(MaterialInstanced&&) = delete;
		MaterialInstanced& operator=(MaterialInstanced&&) = delete;
		MaterialInstanced(const MaterialInstanced&) = delete;
		MaterialInstanced& operator=(const MaterialInstanced&) = delete;
		~MaterialInstanced() override = default;

		void init() override;

		template <typename T = rawrbox::VertexData>
		void bindUniforms(const rawrbox::Mesh<T>& mesh) {
			auto context = rawrbox::RENDERER->context();

			// SETUP UNIFORMS ----------------------------
			Diligent::MapHelper<rawrbox::MaterialBaseUniforms> CBConstants(context, this->_uniforms, Diligent::MAP_WRITE, Diligent::MAP_FLAG_DISCARD);
			this->bindBaseUniforms<T, rawrbox::MaterialBaseUniforms>(mesh, CBConstants);
			// ------------
		}
	};

} // namespace rawrbox
