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

	class LIGHTS {
	protected:
		static std::vector<std::unique_ptr<rawrbox::LightBase>> _lights;

		static bgfx::DynamicVertexBufferHandle _buffer;
		static bgfx::UniformHandle _u_lightSettings;

		static bgfx::UniformHandle _u_ambientLight;

		static bgfx::UniformHandle _u_sunDirection;
		static bgfx::UniformHandle _u_sunColor;

		static rawrbox::Colorf _ambient;

		static rawrbox::Colorf _sun_color;
		static rawrbox::Vector3f _sun_direction;

	public:
		static bool fullbright;

		static void init();
		static void shutdown();
		static void update();

		static void bindUniforms();

		// UTILS ----
		static void setEnabled(bool fb);
		static void setSun(const rawrbox::Vector3f& dir, const rawrbox::Colorf& col);
		static void setAmbient(const rawrbox::Colorf& col);
		// -------

		// Light ----
		template <typename T = rawrbox::LightBase>
		static rawrbox::LightBase* addLight(T light) {
			if (_lights.size() >= rawrbox::MAX_LIGHTS) {
				fmt::print("[RawrBox-LIGHTS] Could not add light, max lights limit hit!\n");
				return nullptr;
			}

			light.setId(++rawrbox::LIGHT_ID);
			auto entry = _lights.emplace_back(std::make_unique<T>(light)).get();
			update();
			return entry;
		}

		static void removeLight(rawrbox::LightBase* light);
		// ---------

		// Light utils ----
		static const rawrbox::LightBase& getLight(size_t indx);
		static size_t count();
		// ---------
	};
} // namespace rawrbox
