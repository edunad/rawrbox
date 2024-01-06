#pragma once

#include <rawrbox/render/materials/unlit.hpp>

namespace rawrbox {
	struct MaterialTextUniforms {
		rawrbox::Vector4f _gBillboard;
	};

	class MaterialText3D : public rawrbox::MaterialUnlit {
		static bool _build;

	public:
		static Diligent::RefCntAutoPtr<Diligent::IBuffer> uniforms;

		using vertexBufferType = rawrbox::VertexData;

		MaterialText3D() = default;
		MaterialText3D(const MaterialText3D&) = delete;
		MaterialText3D(MaterialText3D&&) = delete;
		MaterialText3D& operator=(const MaterialText3D&) = delete;
		MaterialText3D& operator=(MaterialText3D&&) = delete;
		~MaterialText3D() override = default;

		void init() override;
		void createUniforms() override;

		template <typename T = rawrbox::VertexData>
		void bindUniforms(const rawrbox::Mesh<T>& mesh) {
			auto renderer = rawrbox::RENDERER;
			auto context = renderer->context();

			// SETUP UNIFORMS ----------------------------
			{
				Diligent::MapHelper<rawrbox::MaterialTextUniforms> CBConstants(context, this->uniforms, Diligent::MAP_WRITE, Diligent::MAP_FLAG_DISCARD);
				CBConstants->_gBillboard = mesh.getData("billboard_mode");
			}
			// --------

			// Bind ---
			rawrbox::PipelineUtils::signatureBind->GetVariableByName(Diligent::SHADER_TYPE_VERTEX, "Constants")->Set(this->_uniforms);
			rawrbox::PipelineUtils::signatureBind->GetVariableByName(Diligent::SHADER_TYPE_PIXEL, "Constants")->Set(this->_uniforms_pixel);
			// --------
		}
	};

} // namespace rawrbox
