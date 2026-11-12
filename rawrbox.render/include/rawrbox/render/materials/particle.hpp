#pragma once

#include <rawrbox/render/materials/base.hpp>

#include <string>

namespace rawrbox {

	class MaterialParticle : public rawrbox::MaterialBase {
		static bool _built;

	protected:
		[[nodiscard]] virtual std::string getVertexShader() const;

	public:
		MaterialParticle() = default;
		MaterialParticle(MaterialParticle&&) = delete;
		MaterialParticle& operator=(MaterialParticle&&) = delete;
		MaterialParticle(const MaterialParticle&) = delete;
		MaterialParticle& operator=(const MaterialParticle&) = delete;
		~MaterialParticle() override = default;

		void init() override;
		void createPipelines(const std::string& id, const std::vector<Diligent::LayoutElement>& layout, const Diligent::ShaderMacroHelper& helper = {}) override;
	};
} // namespace rawrbox
