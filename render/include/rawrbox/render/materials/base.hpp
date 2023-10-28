#pragma once

#include <rawrbox/render/models/defs.hpp>
#include <rawrbox/render/models/mesh.hpp>

#include <Graphics/GraphicsEngine/interface/InputLayout.h>
#include <Graphics/GraphicsEngine/interface/PipelineState.h>
#include <fmt/format.h>

namespace rawrbox {
	constexpr auto MAX_DATA = 4;

	// FLAGS ------
	// NOLINTBEGIN{unused-const-variable}
	namespace MaterialFlags {
		const uint32_t NONE = 0;
		const uint32_t NORMALS = 1 << 1;
		const uint32_t BONES = 1 << 2;
		const uint32_t INSTANCED = 1 << 3;
		const uint32_t TEXT = 1 << 4;
		const uint32_t PARTICLE = 1 << 5;
	}; // namespace MaterialFlags

	// NOLINTEND{unused-const-variable}
	//  --------------------

	struct MaterialBaseUniforms {
		//  CAMERA -----
		rawrbox::Matrix4x4 _gModel;
		rawrbox::Matrix4x4 _gViewProj;
		rawrbox::Matrix4x4 _gInvView;
		rawrbox::Matrix4x4 _gWorldViewModel;
		rawrbox::Vector4f _gScreenSize;
		//  --------

		// OTHER ----
		rawrbox::Colorf _gColorOverride;
		rawrbox::Vector4f _gTextureFlags;
		std::array<rawrbox::Vector4f, 4> _gData;
		//  ----------
	};

	class MaterialBase {
		static Diligent::RefCntAutoPtr<Diligent::IBuffer> _uniforms;
		std::vector<rawrbox::VertexData> _temp = {};

	protected:
		Diligent::IPipelineState* _base = nullptr;
		Diligent::IPipelineState* _base_alpha = nullptr;
		Diligent::IPipelineState* _line = nullptr;
		Diligent::IPipelineState* _cullback = nullptr;
		Diligent::IPipelineState* _cullback_alpha = nullptr;
		Diligent::IPipelineState* _wireframe = nullptr;
		Diligent::IPipelineState* _cullnone = nullptr;
		Diligent::IPipelineState* _cullnone_alpha = nullptr;

		Diligent::IShaderResourceBinding* _bind = nullptr;

		template <typename T = MaterialBaseUniforms>
		void bindBaseUniforms(const rawrbox::Mesh& mesh, Diligent::MapHelper<T>& helper) {
			auto renderer = rawrbox::RENDERER;

			auto size = renderer->getSize().cast<float>();
			auto tTransform = rawrbox::TRANSFORM.transpose();
			auto tProj = renderer->camera()->getProjMtx().transpose();
			auto tView = renderer->camera()->getViewMtx().transpose();
			auto tInvView = renderer->camera()->getViewMtx();
			tInvView.inverse();

			auto tWorldView = renderer->camera()->getProjViewMtx().transpose();

			std::array<rawrbox::Vector4f, MAX_DATA>
			    data = {rawrbox::Vector4f{0.F, 0.F, 0.F, 0.F}, {0.F, 0.F, 0.F, 0.F}, {0.F, 0.F, 0.F, 0.F}, {0.F, 0.F, 0.F, 0.F}};
			if (mesh.hasData("billboard_mode")) {
				data[0] = mesh.getData("billboard_mode").data();
			}

			if (mesh.hasData("vertex_snap")) {
				data[1] = mesh.getData("vertex_snap").data();
			}

			if (mesh.hasData("displacement_strength")) {
				data[2] = mesh.getData("displacement_strength").data();
			}

			if (mesh.hasData("mask")) {
				data[3] = mesh.getData("mask").data();
			}

			*helper = {
			    // CAMERA -------
			    tTransform,
			    tView * tProj,
			    tInvView,
			    tTransform * tWorldView,
			    size,
			    // --------------
			    mesh.color,
			    mesh.texture == nullptr ? rawrbox::Vector4f() : mesh.texture->getData(),
			    data};
			// ----------------------------
		}

		virtual void prepareMaterial();

		virtual void bindUniforms(const rawrbox::Mesh& mesh);
		virtual void bindPipeline(const rawrbox::Mesh& mesh);
		virtual void bindShaderResources();

	public:
		MaterialBase() = default;
		MaterialBase(MaterialBase&&) = delete;
		MaterialBase& operator=(MaterialBase&&) = delete;
		MaterialBase(const MaterialBase&) = delete;
		MaterialBase& operator=(const MaterialBase&) = delete;
		virtual ~MaterialBase() = default;

		static void init();
		virtual void bind(const rawrbox::Mesh& mesh);

		virtual void* convert(const std::vector<rawrbox::ModelVertexData>& v);

		[[nodiscard]] virtual uint32_t supports() const;
		[[nodiscard]] virtual const uint32_t vLayoutSize();
	};
} // namespace rawrbox
