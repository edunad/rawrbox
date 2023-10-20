#pragma once

#include <Common/interface/RefCntAutoPtr.hpp>

#include <Graphics/GraphicsEngine/interface/Buffer.h>
#include <Graphics/GraphicsEngine/interface/PipelineState.h>
#include <Graphics/GraphicsEngine/interface/Shader.h>
#include <Graphics/GraphicsEngine/interface/ShaderResourceVariable.h>

#include <unordered_map>
#include <vector>

namespace rawrbox {
	struct PipeSettings {
		Diligent::PRIMITIVE_TOPOLOGY topology = Diligent::PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		Diligent::CULL_MODE cull = Diligent::CULL_MODE_BACK;
		Diligent::FILL_MODE fill = Diligent::FILL_MODE_SOLID;

		Diligent::COMPARISON_FUNCTION depth = Diligent::COMPARISON_FUNC_LESS;
		bool depthWrite = true;

		bool scissors = false;
		uint8_t renderTargets = 1;

		std::string pVS = "";
		std::string pPS = "";
		std::string pGS = "";

		std::vector<Diligent::LayoutElement> layout = {};
		std::vector<Diligent::ShaderResourceVariableDesc> resources = {};
	};

	class PipelineUtils {
		static std::unordered_map<std::string, Diligent::RefCntAutoPtr<Diligent::IPipelineState>> _pipelines;
		static std::unordered_map<std::string, Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding>> _binds;
		static std::unordered_map<std::string, Diligent::RefCntAutoPtr<Diligent::IShader>> _shaders;

	public:
		// ---
		static Diligent::IShader* compileShader(const std::string& name, Diligent::SHADER_TYPE type);
		static Diligent::IPipelineState* createPipelines(const std::string& name, const std::string& bindName, const rawrbox::PipeSettings settings, Diligent::IBuffer* uniforms = nullptr);
		[[nodiscard]] static Diligent::IShaderResourceBinding* getBind(const std::string& bindName);
	};
} // namespace rawrbox
