#pragma once

#include <rawrbox/render/models/defs.hpp>
#include <rawrbox/render/models/mesh.hpp>

#include <Graphics/GraphicsEngine/interface/InputLayout.h>
#include <Graphics/GraphicsEngine/interface/PipelineState.h>
#include <fmt/format.h>

namespace rawrbox {
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

		static Diligent::IPipelineState* _base;
		static Diligent::IPipelineState* _line;
		static Diligent::IPipelineState* _cullback;
		static Diligent::IPipelineState* _wireframe;
		static Diligent::IPipelineState* _cullnone;

		static Diligent::IShaderResourceBinding* _bind;

	protected:
		virtual void bindUniforms(const rawrbox::Mesh& mesh);
		virtual void bindPipeline(const rawrbox::Mesh& mesh);

	public:
		MaterialBase() = default;
		MaterialBase(MaterialBase&&) = delete;
		MaterialBase& operator=(MaterialBase&&) = delete;
		MaterialBase(const MaterialBase&) = delete;
		MaterialBase& operator=(const MaterialBase&) = delete;
		virtual ~MaterialBase() = default;

		static std::vector<rawrbox::VertexData> convert(const std::vector<rawrbox::VertexNormBoneData>& v);
		static void init();

		virtual void bind(const rawrbox::Mesh& mesh);

		[[nodiscard]] virtual uint32_t supports() const;
		[[nodiscard]] static const std::pair<std::vector<Diligent::LayoutElement>, uint32_t> vLayout();
	};
} // namespace rawrbox
