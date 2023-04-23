#pragma once

#include <rawrbox/render/model/mesh.hpp>
#include <rawrbox/render/shader_defines.h>
#include <rawrbox/render/static.h>
#include <rawrbox/render/util/uniforms.hpp>

#include <bgfx/bgfx.h>
#include <fmt/format.h>

#include <concepts>
#include <stdexcept>
#include <string>
#include <unordered_map>

namespace rawrBox {
	class MaterialBase {

	protected:
		bgfx::ProgramHandle _program = BGFX_INVALID_HANDLE;
		std::unordered_map<std::string, bgfx::UniformHandle> _uniforms;

	public:
		bgfx::VertexLayout vLayout;

		MaterialBase() {
			this->vLayout.begin()
			    .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
			    .add(bgfx::Attrib::Normal, 4, bgfx::AttribType::Uint8, true, true)
			    .add(bgfx::Attrib::Tangent, 4, bgfx::AttribType::Uint8, true, true)
			    .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
			    .add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true)
			    .end();

			// Default uniforms
			this->registerUniform("u_viewPos", bgfx::UniformType::Vec4, 3);
		};

		MaterialBase(MaterialBase&&) = delete;
		MaterialBase& operator=(MaterialBase&&) = delete;
		MaterialBase(const MaterialBase&) = delete;
		MaterialBase& operator=(const MaterialBase&) = delete;

		virtual ~MaterialBase() {
			RAWRBOX_DESTROY(this->_program);
			for (auto& un : this->_uniforms)
				RAWRBOX_DESTROY(un.second);

			this->_uniforms.clear();
		}

		virtual void registerUniform(const std::string& name, bgfx::UniformType::Enum type = bgfx::UniformType::Vec4, uint16_t num = (uint16_t)1U) {
			auto fnd = this->_uniforms.find(name);
			if (fnd != this->_uniforms.end()) throw std::runtime_error(fmt::format("[RawrBox-MaterialBase] Uniform '{}' already registered!", name));

			auto handle = bgfx::createUniform(name.c_str(), type, num);
			if (!bgfx::isValid(handle)) throw std::runtime_error(fmt::format("[RawrBox-MaterialBase] Failed to create uniform '{}'!", name));

			this->_uniforms[name] = handle;
		}

		virtual void buildShader(const bgfx::EmbeddedShader shaders[], const std::string& name) {
			bgfx::RendererType::Enum type = bgfx::getRendererType();
			bgfx::ShaderHandle vsh = bgfx::createEmbeddedShader(shaders, type, fmt::format("vs_{}", name).c_str());
			bgfx::ShaderHandle fsh = bgfx::createEmbeddedShader(shaders, type, fmt::format("fs_{}", name).c_str());

			this->_program = bgfx::createProgram(vsh, fsh, true);
			if (!bgfx::isValid(this->_program)) throw std::runtime_error("[RawrBox-MaterialBase] Failed to create shader");
		}

		template <typename T>
		void setUniform(const std::string& name, T& data) {
			auto fnd = this->_uniforms.find(name);
			if (fnd == this->_uniforms.end()) throw std::runtime_error(fmt::format("[RawrBox-MaterialBase] Uniform '{}' not found!", name));

			rawrBox::UniformUtils::setUniform(fnd->second, data);
		}

		bgfx::UniformHandle getUniform(const std::string& name) {
			auto fnd = this->_uniforms.find(name);
			if (fnd == this->_uniforms.end()) throw std::runtime_error(fmt::format("[RawrBox-MaterialBase] Uniform '{}' not found!", name));

			return fnd->second;
		}

		virtual void preProcess(){};
		virtual void process(std::shared_ptr<rawrBox::Mesh> mesh){};
		virtual void postProcess() {
			bgfx::submit(rawrBox::CURRENT_VIEW_ID, this->_program);
		}

		virtual void upload() = 0;
	};
} // namespace rawrBox
