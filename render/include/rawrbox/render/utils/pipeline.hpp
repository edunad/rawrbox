#pragma once

#include <Graphics/GraphicsEngine/interface/PipelineState.h>
#include <Graphics/GraphicsEngine/interface/ShaderResourceVariable.h>

#include <vector>

namespace rawrbox {
	struct PipeSettings {
		Diligent::PRIMITIVE_TOPOLOGY topology = Diligent::PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		Diligent::CULL_MODE cull = Diligent::CULL_MODE_BACK;

		Diligent::COMPARISON_FUNCTION depth = Diligent::COMPARISON_FUNC_LESS;
		bool depthWrite = true;

		bool scissors = false;
		uint8_t renderTargets = 1;

		std::string vsh = "";
		std::string psh = "";
		std::string gsh = "";

		std::vector<Diligent::LayoutElement> layout = {};
		std::vector<Diligent::ShaderResourceVariableDesc> resources = {};
	};

	class PipelineUtils {
	public:
		// ---
		static void createPipelines(const std::string& name, const rawrbox::PipeSettings settings, Diligent::IPipelineState** pipe);
	};
} // namespace rawrbox
