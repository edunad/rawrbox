#pragma once

#include <rawrbox/utils/logger.hpp>

#include <RefCntAutoPtr.hpp>
#include <RenderStateCache.hpp>
#include <ShaderMacroHelper.hpp>

#include <Buffer.h>
#include <PipelineState.h>
#include <Shader.h>
#include <ShaderResourceVariable.h>

#include <unordered_map>
#include <vector>
#include <filesystem>

namespace rawrbox {
	struct PipeUniforms {
		Diligent::SHADER_TYPE type = Diligent::SHADER_TYPE_VERTEX;
		Diligent::IDeviceObject* uniform = nullptr;

		std::string name = "Constants";
	};

	struct PipePass {
		Diligent::IRenderPass* pass = nullptr;
		uint8_t index = 0;
	};

	struct PipeBlending {
		Diligent::BLEND_FACTOR src = Diligent::BLEND_FACTOR_ONE;
		Diligent::BLEND_FACTOR dest = Diligent::BLEND_FACTOR_INV_SRC_ALPHA;
	};

	struct PipeSampler {
		Diligent::SHADER_TYPE type = Diligent::SHADER_TYPE_VERTEX;
		std::string name = "Constants";
	};

	struct PipeSettings {
		Diligent::PRIMITIVE_TOPOLOGY topology = Diligent::PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		Diligent::CULL_MODE cull = Diligent::CULL_MODE_FRONT;
		Diligent::FILL_MODE fill = Diligent::FILL_MODE_SOLID;

		Diligent::COMPARISON_FUNCTION depth = Diligent::COMPARISON_FUNC_LESS;
		bool depthWrite = true;
		bool depthFormat = true;
		bool scissors = false;
		std::vector<rawrbox::PipeSampler> immutableSamplers = {};

		uint8_t renderTargets = 1;
		rawrbox::PipePass renderPass = {};
		rawrbox::PipeBlending blending = {};

		Diligent::ShaderMacroHelper macros = {};
		std::vector<Diligent::IPipelineResourceSignature*> signatures = {};

		std::string pVS;
		std::string pPS;
		std::string pGS;

		std::string bind;

		std::vector<rawrbox::PipeUniforms> uniforms = {};
		std::vector<Diligent::LayoutElement> layout = {};
		std::vector<Diligent::ShaderResourceVariableDesc> resources = {};

		Diligent::SHADER_RESOURCE_VARIABLE_TYPE resourceType = Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC;
	};

	struct PipeComputeSettings {
		std::string pCS;
		std::string bind;

		std::vector<rawrbox::PipeUniforms> uniforms = {};
		std::vector<Diligent::ShaderResourceVariableDesc> resources = {};
		std::vector<Diligent::IPipelineResourceSignature*> signatures = {};

		Diligent::SHADER_RESOURCE_VARIABLE_TYPE resourceType = Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC;
		Diligent::ShaderMacroHelper macros = {};
	};

	class PipelineUtils {
	protected:
		static std::unordered_map<std::string, Diligent::RefCntAutoPtr<Diligent::IPipelineState>> _pipelines;
		static std::unordered_map<std::string, Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding>> _binds;
		static std::unordered_map<std::string, Diligent::RefCntAutoPtr<Diligent::IShader>> _shaders;
		static std::unordered_map<uint32_t, Diligent::RefCntAutoPtr<Diligent::ISampler>> _samplers;
		static std::unordered_map<Diligent::SHADER_TYPE, Diligent::ShaderMacroHelper> _globalMacros;

		static Diligent::RefCntAutoPtr<Diligent::IRenderStateCache> _stateCache;

		static std::filesystem::path _stateCachePath;

		static std::unique_ptr<rawrbox::Logger> _logger;

	public:
		static Diligent::ISampler* defaultSampler;
		static bool initialized;

		// ---
		static void init(Diligent::IRenderDevice& device);
		static void shutdown();

		static Diligent::ISampler* registerSampler(uint32_t id, Diligent::SamplerDesc desc);
		static Diligent::IShader* compileShader(const std::string& name, Diligent::SHADER_TYPE type, const Diligent::ShaderMacroHelper& macros = {});
		static std::vector<Diligent::ImmutableSamplerDesc> compileSamplers(const std::vector<rawrbox::PipeSampler>& samplers);

		static Diligent::IPipelineState* createComputePipeline(const std::string& name, rawrbox::PipeComputeSettings settings);
		static Diligent::IPipelineState* createPipeline(const std::string& name, rawrbox::PipeSettings settings);

		template <typename T = std::string>
		static void registerGlobalMacro(const Diligent::SHADER_TYPE type, const std::string& macro, const T& val) {
			_globalMacros[type].Add(macro.c_str(), val);
		}

		[[nodiscard]] static Diligent::ISampler* getSampler(uint32_t id);
		[[nodiscard]] static Diligent::IShaderResourceBinding* getBind(const std::string& bindName);
		[[nodiscard]] static Diligent::IPipelineState* getPipeline(const std::string& pipe);
	};
} // namespace rawrbox
