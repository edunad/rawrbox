#pragma once

#include <rawrbox/render/model/defs.hpp>
#include <rawrbox/render/model/mesh.hpp>

#include <bgfx/bgfx.h>
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

	class MaterialBase {
	protected:
		bgfx::ProgramHandle _program = BGFX_INVALID_HANDLE;

		std::unordered_map<std::string, bgfx::UniformHandle> _uniforms = {};
		virtual void setupUniforms();

	public:
		MaterialBase() = default;
		MaterialBase(MaterialBase&&) = delete;
		MaterialBase& operator=(MaterialBase&&) = delete;
		MaterialBase(const MaterialBase&) = delete;
		MaterialBase& operator=(const MaterialBase&) = delete;
		virtual ~MaterialBase();

		virtual void registerUniform(const std::string& name, bgfx::UniformType::Enum type, uint16_t num = 0);

		// SET ----
		virtual void setUniformData(const std::string& id, const std::vector<rawrbox::Matrix4x4>& data);
		// -----

		virtual void process(const rawrbox::Mesh& mesh);
		virtual void process(const bgfx::TextureHandle& texture);

		virtual void postProcess();
		virtual void upload();

		[[nodiscard]] virtual uint32_t supports() const;
		[[nodiscard]] virtual const bgfx::VertexLayout vLayout() const;
	};
} // namespace rawrbox
