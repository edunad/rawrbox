#pragma once

#include <rawrbox/render/passes/base.hpp>

namespace rawrbox {
	class ClusteredLightPass : rawrbox::RenderPass {
	protected:
	public:
		ClusteredLightPass(const ClusteredLightPass&) = default;
		ClusteredLightPass(ClusteredLightPass&&) = delete;
		ClusteredLightPass& operator=(const ClusteredLightPass&) = default;
		ClusteredLightPass& operator=(ClusteredLightPass&&) = delete;
		~ClusteredLightPass() override;

		[[nodiscard]] const std::string getID() const override;

		void initialize(const rawrbox::Vector2i& size) override;
		void resize(const rawrbox::Vector2i& size) override;
		void render(const rawrbox::DrawPass& pass) override;
	};
} // namespace rawrbox
