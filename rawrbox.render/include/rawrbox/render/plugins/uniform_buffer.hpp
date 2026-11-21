#pragma once

#include <rawrbox/render/plugins/base.hpp>
#include <rawrbox/render/static.hpp>
#include <rawrbox/render/utils/uniform.hpp>
#include <rawrbox/utils/logger.hpp>

#include <RefCntAutoPtr.hpp>

#include <Buffer.h>
#include <fmt/format.h>

#include <string_view>
#include <type_traits>
#include <vector>

namespace rawrbox {
	template <typename T>
		requires(std::is_trivially_copyable_v<T> && std::is_default_constructible_v<T>)
	class UniformBufferPlugin : public rawrbox::RenderPlugin {
	protected:
		Diligent::RefCntAutoPtr<Diligent::IBuffer> _buffer = {};
		T _uniforms = {};
		
		bool _dirty = false;

		Diligent::SHADER_TYPE _stage = Diligent::SHADER_TYPE_UNKNOWN;
		std::string_view _constants = {};

	public:
		UniformBufferPlugin(Diligent::SHADER_TYPE stage, std::string_view constants) : _stage(stage), _constants(constants) {}
		UniformBufferPlugin(const UniformBufferPlugin&) = delete;
		UniformBufferPlugin(UniformBufferPlugin&&) = delete;
		UniformBufferPlugin& operator=(const UniformBufferPlugin&) = delete;
		UniformBufferPlugin& operator=(UniformBufferPlugin&&) = delete;
		~UniformBufferPlugin() override { RAWRBOX_DESTROY(this->_buffer); }

		void signatures(std::vector<Diligent::PipelineResourceDesc>& sig) override {
			sig.emplace_back(this->_stage, this->_constants.data(), 1, Diligent::SHADER_RESOURCE_TYPE_CONSTANT_BUFFER, Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC, Diligent::PIPELINE_RESOURCE_FLAG_NO_DYNAMIC_BUFFERS);
		}

		void bindStatic(Diligent::IPipelineResourceSignature& sig) override {
			sig.GetStaticVariableByName(this->_stage, this->_constants.data())->Set(this->_buffer);
		}

		void initialize(const rawrbox::Vector2u& /*renderSize*/) override {
			if (this->_buffer != nullptr) RAWRBOX_CRITICAL("{} uniforms already initialized", this->getID());

			this->_buffer = rawrbox::UniformUtils::create(fmt::format("{}::Uniforms", this->getID()).c_str(), &this->_uniforms, sizeof(T));
			this->_dirty = false;
		}

		void preRender(const rawrbox::CameraBase& /*camera*/) override {
			if (!this->_dirty) return;

			this->_dirty = false;
			rawrbox::UniformUtils::update(this->_buffer, &this->_uniforms, sizeof(T));
		}
	};
} // namespace rawrbox