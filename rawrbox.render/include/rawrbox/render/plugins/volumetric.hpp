#pragma once

#include <rawrbox/math/matrix4x4.hpp>
#include <rawrbox/math/vector4.hpp>
#include <rawrbox/render/lights/manager.hpp>
#include <rawrbox/render/plugins/base.hpp>
#include <rawrbox/render/render_config.hpp>
#include <rawrbox/render/textures/render.hpp>

#include <RefCntAutoPtr.hpp>

#include <Buffer.h>
#include <PipelineState.h>

#include <array>
#include <memory>
#include <vector>

// Raymarched light volumes
// Based on : https://gdcvault.com/play/1023002/Low-Complexity-High-Fidelity-INSIDE

namespace rawrbox {
	struct VolumetricSettings {
		// DEFAULT SETTINGS ----
		uint32_t divisions = 2U;
		uint32_t steps = 6U;
		uint32_t levels = 8U;

		float blend = 0.2F; // lower -> smoother but more ghosting (temporal)
				    // -------------
	};

	struct VolumetricConstants {
		std::array<rawrbox::Vector4f, 4> prevClip = {};
		rawrbox::Vector4u settings = {}; // x = lights, y = depth | scatter << 16, z = history (0 = none) | current << 16, w = frame
	};

	struct VolumetricLightData {
		rawrbox::LightDataVertex light = {};
		rawrbox::Vector4f params = {}; // x = density, y = distance behind the light, z = distance to the beam start
	};

	struct VolumetricLightSettings {
		float density = 0.03F;

		// SPOT LIGHT ----
		float width = 0.F;
		float offset = 0.F;
		// --------------
	};

	struct VolumetricLight {
		std::weak_ptr<rawrbox::LightBase> light = {};
		rawrbox::VolumetricLightSettings settings = {};
	};

	class VolumetricPlugin : public rawrbox::RenderPlugin {
	protected:
		rawrbox::VolumetricSettings _settings = {};

		std::vector<rawrbox::VolumetricLight> _lights = {};
		std::vector<rawrbox::VolumetricLightData> _data = {};

		std::unique_ptr<rawrbox::TextureRender> _scatter;

		// HISTORY (Temporal) ---
		std::array<std::unique_ptr<rawrbox::TextureRender>, 2> _history = {};
		uint32_t _historyIndex = 0;

		rawrbox::Matrix4x4 _prevViewProj = {};
		uint64_t _prevFrame = 0;
		uint32_t _jitterFrame = 0;
		// -----------

		// BUFFERS ---
		Diligent::RefCntAutoPtr<Diligent::IBuffer> _constants;
		Diligent::RefCntAutoPtr<Diligent::IBuffer> _lightBuffer;
		// -----------

		// PROGRAMS ---
		Diligent::IPipelineState* _marchProgram = nullptr;
		Diligent::IPipelineState* _resolveProgram = nullptr;
		Diligent::IPipelineState* _compositeProgram = nullptr;
		// -----------

	public:
		explicit VolumetricPlugin(const rawrbox::VolumetricSettings& settings = {});
		VolumetricPlugin(const VolumetricPlugin&) = delete;
		VolumetricPlugin(VolumetricPlugin&&) = delete;
		VolumetricPlugin& operator=(const VolumetricPlugin&) = delete;
		VolumetricPlugin& operator=(VolumetricPlugin&&) = delete;
		~VolumetricPlugin() override;

		// LIGHTS ---
		virtual void add(const rawrbox::LightBase& light, const rawrbox::VolumetricLightSettings& settings = {});
		virtual void remove(const rawrbox::LightBase& light);
		// -----------

		void initialize(const rawrbox::Vector2u& size) override;
		void upload() override;

		void signatures(std::vector<Diligent::PipelineResourceDesc>& sig) override;
		void bindStatic(Diligent::IPipelineResourceSignature& sig) override;

		void postRender(const rawrbox::CameraBase& camera) override;

		[[nodiscard]] short getPriority() const override;
		std::string getID() override;
	};
} // namespace rawrbox