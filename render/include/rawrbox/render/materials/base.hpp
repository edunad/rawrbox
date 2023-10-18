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
		// Diligent::float4x4 _gWorldViewModel;
		//  --------
	};

	class MaterialBase {
	protected:
		Diligent::RefCntAutoPtr<Diligent::IBuffer> _uniforms;
		Diligent::RefCntAutoPtr<Diligent::IPipelineState> _pipeline;
		Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> _SRB;

		virtual void setupUniforms();

	public:
		MaterialBase() = default;
		MaterialBase(MaterialBase&&) = delete;
		MaterialBase& operator=(MaterialBase&&) = delete;
		MaterialBase(const MaterialBase&) = delete;
		MaterialBase& operator=(const MaterialBase&) = delete;
		virtual ~MaterialBase();

		// virtual void registerUniform(const std::string& name, bgfx::UniformType::Enum type, uint16_t num = 0);
		//[[nodiscard]] virtual bgfx::UniformHandle& getUniform(const std::string& name);

		// SET ----
		virtual void setUniformData(const std::string& id, const std::vector<rawrbox::Matrix4x4>& data);
		// -----

		virtual void bind(const rawrbox::Mesh& mesh);
		virtual void bind(Diligent::ITextureView* texture);

		virtual void postProcess();
		virtual void upload();

		[[nodiscard]] virtual uint32_t supports() const;
		[[nodiscard]] virtual const std::vector<Diligent::LayoutElement> vLayout() const;
	};
} // namespace rawrbox
