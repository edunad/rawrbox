#pragma once

#include <RefCntAutoPtr.hpp>
#include <RenderStateCache.hpp>
#include <ShaderMacroHelper.hpp>

#include <Buffer.h>
#include <PipelineState.h>
#include <Shader.h>
#include <ShaderResourceVariable.h>

#include <unordered_map>
#include <vector>

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
		Diligent::IPipelineResourceSignature* signature = nullptr;

		std::string pVS = "";
		std::string pPS = "";
		std::string pGS = "";

		std::vector<rawrbox::PipeUniforms> uniforms = {};
		std::vector<Diligent::LayoutElement> layout = {};
		std::vector<Diligent::ShaderResourceVariableDesc> resources = {};

		Diligent::SHADER_RESOURCE_VARIABLE_TYPE resourceType = Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC;
	};

	struct PipeComputeSettings {
		std::string pCS = "";

		std::vector<rawrbox::PipeUniforms> uniforms = {};
		std::vector<Diligent::ShaderResourceVariableDesc> resources = {};

		Diligent::IPipelineResourceSignature* signature = nullptr;

		Diligent::SHADER_RESOURCE_VARIABLE_TYPE resourceType = Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC;
		Diligent::ShaderMacroHelper macros = {};
	};

	struct PipeSignatureSettings {
		std::vector<Diligent::PipelineResourceDesc> desc = {};
		std::vector<rawrbox::PipeSampler> immutableSamplers = {};

		uint8_t bindingIndex = 0;
	};

	class PipelineUtils {
		static std::unordered_map<std::string, Diligent::RefCntAutoPtr<Diligent::IPipelineState>> _pipelines;
		static std::unordered_map<std::string, Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding>> _binds;
		static std::unordered_map<std::string, Diligent::RefCntAutoPtr<Diligent::IShader>> _shaders;
		static std::unordered_map<uint32_t, Diligent::RefCntAutoPtr<Diligent::ISampler>> _samplers;
		static std::unordered_map<std::string, Diligent::RefCntAutoPtr<Diligent::IPipelineResourceSignature>> _signatures;

		static Diligent::RefCntAutoPtr<Diligent::IRenderStateCache> _stateCache;

	public:
		static Diligent::ISampler* defaultSampler;
		static bool initialized;

		// ---
		static void init(Diligent::IRenderDevice& device);

		static Diligent::ISampler* registerSampler(uint32_t id, Diligent::SamplerDesc type);
		static Diligent::IShader* compileShader(const std::string& name, Diligent::SHADER_TYPE type, Diligent::ShaderMacroArray macros = {});
		static std::vector<Diligent::ImmutableSamplerDesc> compileSamplers(const std::vector<rawrbox::PipeSampler>& samplers);

		static Diligent::IPipelineState* createComputePipeline(const std::string& name, const std::string& bindName, rawrbox::PipeComputeSettings settings);
		static Diligent::IPipelineState* createPipeline(const std::string& name, const std::string& bindName, rawrbox::PipeSettings settings);
		static Diligent::IPipelineResourceSignature* createSignature(const std::string& name, const rawrbox::PipeSignatureSettings settings);

		[[nodiscard]] static Diligent::ISampler* getSampler(uint32_t id);
		[[nodiscard]] static Diligent::IShaderResourceBinding* getBind(const std::string& bindName);
		[[nodiscard]] static Diligent::IPipelineState* getPipeline(const std::string& pipe);
	};
} // namespace rawrbox
