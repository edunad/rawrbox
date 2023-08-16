#pragma once

#include <rawrbox/render/light/base.hpp>
#include <rawrbox/render/static.hpp>

#include <memory>
#include <vector>

namespace rawrbox {

	struct LightDataVertex {
		rawrbox::Vector3f position = {};
		float outerCone = 0.F;

		rawrbox::Vector3f intensity = {};
		float radius = 0.F;

		rawrbox::Vector3f direction = {};
		float innerCone = 0.F;

		static bgfx::VertexLayout vLayout() {
			static bgfx::VertexLayout l;
			l.begin()
			    .add(bgfx::Attrib::TexCoord0, 4, bgfx::AttribType::Float) // Position
			    .add(bgfx::Attrib::TexCoord1, 4, bgfx::AttribType::Float) // Intensity
			    .add(bgfx::Attrib::TexCoord2, 4, bgfx::AttribType::Float) // Direction
			    .end();
			return l;
		};
	};

	enum class FOG_TYPE {
		FOG_LINEAR = 0,
		FOG_EXP = 1
	};

	class LIGHTS {
	protected:
		static std::vector<std::shared_ptr<rawrbox::LightBase>> _lights;

		static bgfx::DynamicVertexBufferHandle _buffer;
		static bgfx::UniformHandle _u_lightSettings;

		static bgfx::UniformHandle _u_ambientLight;

		static bgfx::UniformHandle _u_sunDirection;
		static bgfx::UniformHandle _u_sunColor;

		static bgfx::UniformHandle _u_fogColor;
		static bgfx::UniformHandle _u_fogSettings;

		static rawrbox::Colorf _ambient;

		static rawrbox::Colorf _sun_color;
		static rawrbox::Vector3f _sun_direction;

		static rawrbox::Colorf _fog_color;
		static rawrbox::FOG_TYPE _fog_type;
		static float _fog_density;
		static float _fog_end;

		static void update();

	public:
		static bool fullbright;

		static void init();
		static void shutdown();

		static void bindUniforms();

		// UTILS ----
		static void setEnabled(bool fb);

		// SUN
		static void setSun(const rawrbox::Vector3f& dir, const rawrbox::Colorf& col);
		static const rawrbox::Colorf& getSunColor();
		static const rawrbox::Vector3f& getSunDir();
		// ----

		// FOG
		static void setFog(rawrbox::FOG_TYPE type, float end, float density, const rawrbox::Colorf& col = rawrbox::Colors::Black());

		static rawrbox::FOG_TYPE getFogType();
		static const rawrbox::Colorf& getFogColor();
		static float getFogEnd();
		static float getFogDensity();
		// ----

		// AMBIENT
		static void setAmbient(const rawrbox::Colorf& col);
		static const rawrbox::Colorf& getAmbient();
		// -------

		// Light ----
		template <typename T = rawrbox::LightBase, typename... CallbackArgs>
		static rawrbox::LightBase* addLight(CallbackArgs&&... args) {
			auto light = _lights.emplace_back(std::make_shared<T>(std::forward<CallbackArgs>(args)...)).get();
			light->setId(++rawrbox::LIGHT_ID);
			rawrbox::__LIGHT_DIRTY__ = true;

			return light;
		}

		static void removeLight(rawrbox::LightBase* light);
		// ---------

		// Light utils ----
		static rawrbox::LightBase* getLight(size_t indx);
		static size_t count();
		// ---------
	};
} // namespace rawrbox
