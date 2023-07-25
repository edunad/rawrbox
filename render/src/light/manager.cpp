
#include <rawrbox/render/light/manager.hpp>

#include <fmt/format.h>

namespace rawrbox {
	// PRIVATE ----
	std::vector<std::unique_ptr<rawrbox::LightBase>> LIGHTS::_lights = {};

	bgfx::DynamicVertexBufferHandle LIGHTS::_buffer = BGFX_INVALID_HANDLE;
	bgfx::UniformHandle LIGHTS::_u_lightSettings = BGFX_INVALID_HANDLE;
	bgfx::UniformHandle LIGHTS::_u_ambientLight = BGFX_INVALID_HANDLE;

	bgfx::UniformHandle LIGHTS::_u_sunDirection = BGFX_INVALID_HANDLE;
	bgfx::UniformHandle LIGHTS::_u_sunColor = BGFX_INVALID_HANDLE;

	bgfx::UniformHandle LIGHTS::_u_fogColor = BGFX_INVALID_HANDLE;
	bgfx::UniformHandle LIGHTS::_u_fogSettings = BGFX_INVALID_HANDLE;

	// Ambient --
	rawrbox::Colorf LIGHTS::_ambient = {0.01F, 0.01F, 0.01F, 1.F};

	// Sun --
	rawrbox::Colorf LIGHTS::_sun_color = rawrbox::Colors::Transparent; // No sun by default
	rawrbox::Vector3f LIGHTS::_sun_direction = {0, 0, 0};

	// Fog --
	rawrbox::Colorf LIGHTS::_fog_color = {0.F, 0.F, 0.F, 0.F};
	float LIGHTS::_fog_density = -1.F;
	float LIGHTS::_fog_end = -1.F;
	rawrbox::FOG_TYPE LIGHTS::_fog_type = rawrbox::FOG_TYPE::FOG_EXP;
	// -----

	// PUBLIC ----
	bool LIGHTS::fullbright = false;
	// -------

	void LIGHTS::init() {
		_buffer = bgfx::createDynamicVertexBuffer(
		    1, LightDataVertex::vLayout(), BGFX_BUFFER_COMPUTE_READ | BGFX_BUFFER_ALLOW_RESIZE);

		_u_lightSettings = bgfx::createUniform("u_lightSettings", bgfx::UniformType::Vec4);
		_u_ambientLight = bgfx::createUniform("u_ambientLight", bgfx::UniformType::Vec4);

		_u_sunDirection = bgfx::createUniform("u_sunDirection", bgfx::UniformType::Vec4);
		_u_sunColor = bgfx::createUniform("u_sunColor", bgfx::UniformType::Vec4);

		_u_fogColor = bgfx::createUniform("u_fogColor", bgfx::UniformType::Vec4);
		_u_fogSettings = bgfx::createUniform("u_fogSettings", bgfx::UniformType::Vec4);
	}

	void LIGHTS::shutdown() {
		RAWRBOX_DESTROY(_buffer);
		RAWRBOX_DESTROY(_u_lightSettings);
		RAWRBOX_DESTROY(_u_ambientLight);
		RAWRBOX_DESTROY(_u_sunDirection);
		RAWRBOX_DESTROY(_u_sunColor);
		RAWRBOX_DESTROY(_u_fogColor);
		RAWRBOX_DESTROY(_u_fogSettings);

		_lights.clear();
	}

	void LIGHTS::update() {
		if (!bgfx::isValid(_buffer)) throw std::runtime_error("[Rawrbox-LIGHT] Buffer not initialized! Did you call 'init' ?");
		if (!rawrbox::__LIGHT_DIRTY__ || _lights.empty()) return;

		// Update lights ---
		auto stride = rawrbox::LightDataVertex::vLayout().getStride();
		const bgfx::Memory* mem = bgfx::alloc(uint32_t(stride * _lights.size()));

		for (size_t i = 0; i < _lights.size(); i++) {
			auto& l = _lights[i];
			if (!l->isOn()) continue;

			auto light = std::bit_cast<rawrbox::LightDataVertex*>(mem->data + (i * stride));

			auto cl = l->getColor();
			auto pos = l->getWorldPos();
			auto dir = l->getDirection();

			light->position = rawrbox::Vector3f(pos.x, pos.y, pos.z);
			light->intensity = rawrbox::Vector3f(cl.r, cl.g, cl.b);
			light->direction = rawrbox::Vector3f(dir.x, dir.y, dir.z);
			light->radius = l->getRadius();

			if (l->getType() == rawrbox::LightType::SPOT) {
				auto data = l->getData();
				light->innerCone = data.x;
				light->outerCone = data.y;
			} else {
				light->innerCone = 0.F;
				light->outerCone = 0.F;
			}
		}

		bgfx::update(_buffer, 0, mem);
		rawrbox::__LIGHT_DIRTY__ = false;
		// -------
	}

	void LIGHTS::bindUniforms() {
		if (!bgfx::isValid(_buffer)) throw std::runtime_error("[Rawrbox-LIGHT] Buffer not initialized! Did you call 'init' ?");
		update(); // Update all lights

		std::array<float, 4> total = {fullbright ? 1.0F : 0.0F, static_cast<float>(rawrbox::LIGHTS::count())}; // other light settings
		bgfx::setUniform(_u_lightSettings, total.data());

		bgfx::setUniform(_u_ambientLight, _ambient.data().data());

		bgfx::setUniform(_u_sunColor, _sun_color.data().data());
		bgfx::setUniform(_u_sunDirection, _sun_direction.data().data());

		// 0 = type
		std::array<float, 4> fogSettings = {static_cast<float>(_fog_type), _fog_end, _fog_density}; // other fog settings
		bgfx::setUniform(_u_fogSettings, fogSettings.data());
		bgfx::setUniform(_u_fogColor, _fog_color.data().data());

		bgfx::setBuffer(rawrbox::SAMPLE_LIGHTS, _buffer, bgfx::Access::Read);
	}

	// UTILS ----
	void LIGHTS::setEnabled(bool fb) { fullbright = fb; }
	// ----

	// SUN ---
	void LIGHTS::setSun(const rawrbox::Vector3f& dir, const rawrbox::Colorf& col) {
		_sun_direction = dir.normalized();
		_sun_color = col;
	}

	const rawrbox::Colorf& LIGHTS::getSunColor() { return _sun_color; }
	const rawrbox::Vector3f& LIGHTS::getSunDir() { return _sun_direction; }
	// -----

	// AMBIENT ----
	void LIGHTS::setAmbient(const rawrbox::Colorf& col) { _ambient = col; }
	const rawrbox::Colorf& LIGHTS::getAmbient() { return _ambient; }
	// ---------

	// FOG ----
	void LIGHTS::setFog(rawrbox::FOG_TYPE type, float end, float density, const rawrbox::Colorf& color) {
		_fog_type = type;
		_fog_color = color;
		_fog_density = density;
		_fog_end = end;
	}

	const rawrbox::FOG_TYPE LIGHTS::getFogType() { return _fog_type; }
	const rawrbox::Colorf& LIGHTS::getFogColor() { return _fog_color; }
	const float LIGHTS::getFogDensity() { return _fog_density; }
	const float LIGHTS::getFogEnd() { return _fog_end; }
	// ---------

	// Light ----
	void LIGHTS::removeLight(rawrbox::LightBase* light) {
		if (light == nullptr || _lights.empty()) return;
		for (size_t i = 0; i < _lights.size(); i++) {
			if (_lights[i].get() == light) {
				_lights.erase(_lights.begin() + i);
				return;
			}
		}

		rawrbox::__LIGHT_DIRTY__ = true;
	}

	rawrbox::LightBase* LIGHTS::getLight(size_t indx) {
		if (indx < 0 || indx >= _lights.size()) return nullptr;
		return _lights[indx].get();
	}

	size_t LIGHTS::count() {
		return _lights.size();
	}
	// ---------

} // namespace rawrbox
