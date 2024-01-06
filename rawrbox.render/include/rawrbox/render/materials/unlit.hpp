#pragma once

#include <rawrbox/render/decals/manager.hpp>
#include <rawrbox/render/materials/base.hpp>

namespace rawrbox {
	class MaterialUnlit : public rawrbox::MaterialBase {
		static bool _built;

	protected:
		static Diligent::RefCntAutoPtr<Diligent::IBuffer> _uniforms;
		static Diligent::RefCntAutoPtr<Diligent::IBuffer> _uniforms_pixel;

	public:
		using vertexBufferType = rawrbox::VertexData;

		MaterialUnlit() = default;
		MaterialUnlit(const MaterialUnlit&) = delete;
		MaterialUnlit(MaterialUnlit&&) = delete;
		MaterialUnlit& operator=(const MaterialUnlit&) = delete;
		MaterialUnlit& operator=(MaterialUnlit&&) = delete;
		~MaterialUnlit() override = default;

		void init() override;
		void createUniforms() override;
		void createPipelines(const std::string& id, const std::vector<Diligent::LayoutElement>& layout, Diligent::ShaderMacroHelper helper = {}) override;

		template <typename T = rawrbox::VertexData>
		void bindUniforms(const rawrbox::Mesh<T>& mesh) {
			auto context = rawrbox::RENDERER->context();

			// SETUP VERTEX UNIFORMS ----------------------------
			{
				Diligent::MapHelper<rawrbox::MaterialBaseUniforms> CBConstants(context, this->_uniforms, Diligent::MAP_WRITE, Diligent::MAP_FLAG_DISCARD);
				this->bindBaseUniforms<T, rawrbox::MaterialBaseUniforms>(mesh, CBConstants);
			}
			// -----------

			// SETUP PIXEL UNIFORMS ----------------------------
			{
				Diligent::MapHelper<rawrbox::MaterialBasePixelUniforms> CBConstants(context, this->_uniforms_pixel, Diligent::MAP_WRITE, Diligent::MAP_FLAG_DISCARD);
				this->bindBasePixelUniforms<T, rawrbox::MaterialBasePixelUniforms>(mesh, CBConstants);
			}
			// -----------

			// Bind ---
			rawrbox::PipelineUtils::signatureBind->GetVariableByName(Diligent::SHADER_TYPE_VERTEX, "Constants")->Set(this->_uniforms);
			rawrbox::PipelineUtils::signatureBind->GetVariableByName(Diligent::SHADER_TYPE_PIXEL, "Constants")->Set(this->_uniforms_pixel);
			// --------
		}
	};
} // namespace rawrbox
