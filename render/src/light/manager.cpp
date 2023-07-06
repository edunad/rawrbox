
#include <rawrbox/render/light/manager.hpp>

#include <fmt/format.h>

namespace rawrbox {
	// PRIVATE ----
	std::vector<std::unique_ptr<rawrbox::LightBase>> LIGHTS::_lights = {};

	bgfx::DynamicVertexBufferHandle LIGHTS::_buffer = BGFX_INVALID_HANDLE;
	bgfx::UniformHandle LIGHTS::_u_lightSettings = BGFX_INVALID_HANDLE;
	bgfx::UniformHandle LIGHTS::_ambientLightIrradianceUniform = BGFX_INVALID_HANDLE;
	// -----

	// PUBLIC ----
	bool LIGHTS::fullbright = false;

	void LIGHTS::init() {
		_buffer = bgfx::createDynamicVertexBuffer(
		    1, LightDataVertex::vLayout(), BGFX_BUFFER_COMPUTE_READ | BGFX_BUFFER_ALLOW_RESIZE);

		_u_lightSettings = bgfx::createUniform("u_lightSettings", bgfx::UniformType::Vec4);
		_ambientLightIrradianceUniform = bgfx::createUniform("u_ambientLightIrradiance", bgfx::UniformType::Vec4);
	}

	void LIGHTS::shutdown() {
		RAWRBOX_DESTROY(_buffer);
		RAWRBOX_DESTROY(_u_lightSettings);
		RAWRBOX_DESTROY(_ambientLightIrradianceUniform);

		_lights.clear();
	}

	void LIGHTS::update() {
		if (!bgfx::isValid(_buffer)) throw std::runtime_error("[Rawrbox-LIGHT] Buffer not initialized! Did you call 'init' ?");
		if (_lights.empty()) return;

		// Update lights ---
		auto stride = rawrbox::LightDataVertex::vLayout().getStride();
		const bgfx::Memory* mem = bgfx::alloc(uint32_t(stride * _lights.size()));

		for (size_t i = 0; i < _lights.size(); i++) {
			auto& l = _lights[i];
			if (l->getType() != rawrbox::LightType::LIGHT_POINT || !l->isOn()) continue;
			auto light = std::bit_cast<rawrbox::LightDataVertex*>(mem->data + (i * stride));

			auto cl = l->getDiffuseColor() * 0.1F;
			auto pos = l->getWorldPos();

			float lightMax = std::fmaxf(std::fmaxf(cl.r, cl.g), cl.b);
			float radius =
			    (-l->getLinear() + std::sqrtf(l->getLinear() * l->getLinear() - 4 * l->getQuadratic() * (l->getConstant() - (256.0 / 5.0) * lightMax))) / (2 * l->getQuadratic());

			light->position = rawrbox::Vector3f(pos.x, pos.y, pos.z);
			light->intensity = rawrbox::Vector3f(cl.r, cl.g, cl.b);
			light->radius = radius;
		}

		bgfx::update(_buffer, 0, mem);
		// -------
	}

	void LIGHTS::bindUniforms() {
		if (!bgfx::isValid(_buffer)) throw std::runtime_error("[Rawrbox-LIGHT] Buffer not initialized! Did you call 'init' ?");

		std::array<float, 4> total = {fullbright ? 1.0F : 0.0F, static_cast<float>(rawrbox::LIGHTS::count())};
		bgfx::setUniform(_u_lightSettings, total.data());

		std::array<float, 4> ambient = {0.01F, 0.01F, 0.01F, 1.F};
		bgfx::setUniform(_ambientLightIrradianceUniform, ambient.data());

		bgfx::setBuffer(rawrbox::SAMPLE_LIGHTS_POINTLIGHTS, _buffer, bgfx::Access::Read);
	}

	void LIGHTS::setEnabled(bool fb) { fullbright = fb; }

	// Light utils ----
	void LIGHTS::removeLight(rawrbox::LightBase* light) {
		if (light == nullptr || _lights.empty()) return;
		for (size_t i = 0; i < _lights.size(); i++) {
			if (_lights[i].get() == light) {
				_lights.erase(_lights.begin() + i);
				return;
			}
		}

		update();
	}

	const rawrbox::LightBase& LIGHTS::getLight(size_t indx) {
		if (indx < 0 || indx >= _lights.size()) throw std::runtime_error(fmt::format("[RawrBox-LIGHTS] Could not get light {}", indx));
		return *_lights[indx];
	}

	size_t LIGHTS::count() {
		return _lights.size();
	}
	// ---------

} // namespace rawrbox
