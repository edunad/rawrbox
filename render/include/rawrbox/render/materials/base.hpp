#pragma once

#include <rawrbox/render/models/defs.hpp>
#include <rawrbox/render/models/mesh.hpp>

#include <Graphics/GraphicsEngine/interface/InputLayout.h>
#include <Graphics/GraphicsEngine/interface/PipelineState.h>
#include <fmt/format.h>

#include "Common/interface/BasicMath.hpp"

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

	struct MaterialUniforms {
		//  CAMERA -----
		rawrbox::Matrix4x4 _gModel;
		rawrbox::Matrix4x4 _gProj;
		rawrbox::Matrix4x4 _gView;
		rawrbox::Matrix4x4 _gWorldViewModel;
		//  --------

		// OTHER ----
		rawrbox::Colorf _gColorOverride;
		// std::array<rawrbox::Vector4f, 4> _gData;
		//  ----------
	};

	class MaterialBase {
	protected:
		Diligent::RefCntAutoPtr<Diligent::IBuffer> _uniforms;
		std::unordered_map<std::string, Diligent::RefCntAutoPtr<Diligent::IPipelineState>> _pipelines = {};

		Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> _SRB;

		virtual void bindUniforms(const rawrbox::Mesh& mesh);
		virtual void bindPipeline(const rawrbox::Mesh& mesh);

	public:
		MaterialBase() = default;
		MaterialBase(MaterialBase&&) = delete;
		MaterialBase& operator=(MaterialBase&&) = delete;
		MaterialBase(const MaterialBase&) = delete;
		MaterialBase& operator=(const MaterialBase&) = delete;
		virtual ~MaterialBase();

		virtual void init();

		virtual void bind(const rawrbox::Mesh& mesh);
		virtual void bind(Diligent::ITextureView* texture);

		[[nodiscard]] virtual uint32_t supports() const;
		[[nodiscard]] virtual const std::pair<std::vector<Diligent::LayoutElement>, uint32_t> vLayout() const;
	};
} // namespace rawrbox
