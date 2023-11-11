#pragma once

#include <Common/interface/RefCntAutoPtr.hpp>
#include <Graphics/GraphicsTools/interface/ShaderMacroHelper.hpp>

#include <Graphics/GraphicsEngine/interface/Buffer.h>
#include <Graphics/GraphicsEngine/interface/PipelineState.h>
#include <Graphics/GraphicsEngine/interface/Shader.h>
#include <Graphics/GraphicsEngine/interface/ShaderResourceVariable.h>

#include <unordered_map>
#include <vector>

namespace rawrbox {
	struct PipeUniforms {
		Diligent::SHADER_TYPE type = Diligent::SHADER_TYPE_VERTEX;
		Diligent::IBuffer* uniform = nullptr;

		std::string name = "Constants";
	};

	struct PipePass {
		Diligent::IRenderPass* pass = nullptr;
		uint32_t index = 0;
	};

	struct PipeBlending {
		Diligent::BLEND_FACTOR src = Diligent::BLEND_FACTOR_ONE;
		Diligent::BLEND_FACTOR dest = Diligent::BLEND_FACTOR_INV_SRC_ALPHA;
	};

	struct PipeSettings {
		Diligent::PRIMITIVE_TOPOLOGY topology = Diligent::PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		Diligent::CULL_MODE cull = Diligent::CULL_MODE_FRONT;
		Diligent::FILL_MODE fill = Diligent::FILL_MODE_SOLID;

		Diligent::COMPARISON_FUNCTION depth = Diligent::COMPARISON_FUNC_LESS;
		bool depthWrite = true;
		bool depthFormat = true;
		bool scissors = false;
		std::vector<bool> immutableSamplers = {};

		uint8_t renderTargets = 1;
		rawrbox::PipePass renderPass = {};
		rawrbox::PipeBlending blending = {};
		Diligent::ShaderMacroHelper macros = {};

		std::string pVS = "";
		std::string pPS = "";
		std::string pGS = "";

		std::vector<rawrbox::PipeUniforms> uniforms = {};
		std::vector<Diligent::LayoutElement> layout = {};
		std::vector<Diligent::ShaderResourceVariableDesc> resources = {};
	};

	struct PipeComputeSettings {
		std::string pCS = "";

		std::vector<rawrbox::PipeUniforms> uniforms = {};
		std::vector<Diligent::ShaderResourceVariableDesc> resources = {};

		Diligent::ShaderMacroHelper macros = {};
	};

	class PipelineUtils {
		static std::unordered_map<std::string, Diligent::RefCntAutoPtr<Diligent::IPipelineState>> _pipelines;
		static std::unordered_map<std::string, Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding>> _binds;
		static std::unordered_map<std::string, Diligent::RefCntAutoPtr<Diligent::IShader>> _shaders;
		static std::unordered_map<uint32_t, Diligent::RefCntAutoPtr<Diligent::ISampler>> _samplers;

	public:
		static Diligent::ISampler* defaultSampler;
		static bool initialized;

		// ---
		static void init();

		static Diligent::ISampler* registerSampler(uint32_t id, Diligent::SamplerDesc type);
		static Diligent::IShader* compileShader(const std::string& name, Diligent::SHADER_TYPE type, Diligent::ShaderMacroArray macros = {});

		static Diligent::IPipelineState* createComputePipeline(const std::string& name, const std::string& bindName, const rawrbox::PipeComputeSettings settings);
		static Diligent::IPipelineState* createPipeline(const std::string& name, const std::string& bindName, const rawrbox::PipeSettings settings);

		[[nodiscard]] static Diligent::ISampler* getSampler(uint32_t id);
		[[nodiscard]] static Diligent::IShaderResourceBinding* getBind(const std::string& bindName);
		[[nodiscard]] static Diligent::IPipelineState* getPipeline(const std::string& pipe);
	};
} // namespace rawrbox
