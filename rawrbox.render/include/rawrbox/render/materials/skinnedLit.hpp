#pragma once

#include <rawrbox/render/materials/lit.hpp>
#include <rawrbox/render/materials/skinned.hpp>

namespace rawrbox {
	class MaterialSkinnedLit : public rawrbox::MaterialLit {
		static bool _built;

		static Diligent::RefCntAutoPtr<Diligent::IBuffer> _uniforms;
		static Diligent::RefCntAutoPtr<Diligent::IBuffer> _uniforms_pixel;

	public:
		using vertexBufferType = rawrbox::VertexNormBoneData;

		MaterialSkinnedLit() = default;
		MaterialSkinnedLit(MaterialSkinnedLit&&) = delete;
		MaterialSkinnedLit& operator=(MaterialSkinnedLit&&) = delete;
		MaterialSkinnedLit(const MaterialSkinnedLit&) = delete;
		MaterialSkinnedLit& operator=(const MaterialSkinnedLit&) = delete;
		~MaterialSkinnedLit() override = default;

		void init() override;
		void createUniforms() override;

		template <typename T = rawrbox::VertexData>
		void bindUniforms(const rawrbox::Mesh<T>& mesh) {
			auto context = rawrbox::RENDERER->context();

			// SETUP UNIFORMS ----------------------------
			{
				Diligent::MapHelper<rawrbox::MaterialSkinnedUniforms> CBConstants(context, this->_uniforms, Diligent::MAP_WRITE, Diligent::MAP_FLAG_DISCARD);
				this->bindBaseUniforms<T, rawrbox::MaterialSkinnedUniforms>(mesh, CBConstants);

				CBConstants->g_bones = mesh.boneTransforms;
			}

			{
				Diligent::MapHelper<rawrbox::MaterialLitPixelUniforms> CBConstants(context, this->_uniforms_pixel, Diligent::MAP_WRITE, Diligent::MAP_FLAG_DISCARD);

				CBConstants->textureIDs = mesh.textures.getPixelIDs();
				CBConstants->litData = mesh.textures.getData();
			} // ------------

			// Bind ---
			rawrbox::PipelineUtils::signatureBind->GetVariableByName(Diligent::SHADER_TYPE_VERTEX, "Constants")->Set(this->_uniforms);
			rawrbox::PipelineUtils::signatureBind->GetVariableByName(Diligent::SHADER_TYPE_PIXEL, "Constants")->Set(this->_uniforms_pixel);
			// --------
		}
	};

} // namespace rawrbox
