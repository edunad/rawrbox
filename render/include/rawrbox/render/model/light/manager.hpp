#pragma once

#include <rawrbox/render/model/light/base.hpp>
#include <rawrbox/render/static.hpp>

#include <memory>
#include <vector>

namespace rawrbox {

	struct LightDataVertex {
		rawrbox::Vector3f position = {};
		float padding = 0.F;

		rawrbox::Vector3f intensity = {};
		float radius = 0.F;

		/*rawrbox::Colorf color = rawrbox::Colors::White;
		rawrbox::Vector4f paddingAndRadius = {};*/

		static bgfx::VertexLayout vLayout() {
			static bgfx::VertexLayout l;
			l.begin()
			    .add(bgfx::Attrib::TexCoord0, 4, bgfx::AttribType::Float) // Position
			    .add(bgfx::Attrib::TexCoord1, 4, bgfx::AttribType::Float) // Intensity
			    .end();
			return l;
		};
	};

	class LIGHTS {
	protected:
		static std::vector<std::unique_ptr<rawrbox::LightBase>> _lights;

		static bgfx::DynamicVertexBufferHandle _buffer;
		static bgfx::UniformHandle _u_lightSettings;
		static bgfx::UniformHandle _ambientLightIrradianceUniform;

	public:
		static bool fullbright;

		static void init();
		static void shutdown();

		static void update();
		static void bindUniforms();

		static void setEnabled(bool fb);

		// Light ----
		template <typename T = rawrbox::LightBase>
		static rawrbox::LightBase* addLight(T light) {
			if (_lights.size() >= rawrbox::MAX_LIGHTS) {
				fmt::print("[RawrBox-LIGHTS] Could not add light, max lights limit hit!\n");
				return nullptr;
			}

			light.setId(++rawrbox::LIGHT_ID);
			auto a = _lights.emplace_back(std::make_unique<T>(light)).get();
			update();
			return a;
		}

		static void removeLight(rawrbox::LightBase* light);
		// ---------

		// Light utils ----
		static const rawrbox::LightBase& getLight(size_t indx);
		static size_t count();
		// ---------
	};
} // namespace rawrbox
