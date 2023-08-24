#pragma once

#include <rawrbox/render/particles/emitter.hpp>
#include <rawrbox/render/texture/atlas.hpp>

namespace rawrbox {
	class ParticleSystem {
	protected:
		// Drawing ----
		std::vector<std::unique_ptr<rawrbox::Emitter>> _emitters = {};
		std::unique_ptr<rawrbox::MaterialParticle> _material = std::make_unique<rawrbox::MaterialParticle>();
		// ---

		// TEXTURE ---
		rawrbox::TextureAtlas* _atlas = nullptr;
		// ----

		uint32_t _totalParticles = 0;

		static int32_t particleSortFn(const void* _lhs, const void* _rhs);

	public:
		explicit ParticleSystem(rawrbox::TextureAtlas& spriteAtlas);
		ParticleSystem(const ParticleSystem&) = delete;
		ParticleSystem(ParticleSystem&&) = delete;
		ParticleSystem& operator=(const ParticleSystem&) = delete;
		ParticleSystem& operator=(ParticleSystem&&) = delete;
		virtual ~ParticleSystem() = default;

		// UTILS -----
		[[nodiscard]] virtual rawrbox::TextureAtlas* getTexture() const;
		virtual void upload();

		template <typename M = rawrbox::MaterialBase>
		void setMaterial() {
			this->_material = std::make_unique<M>();
			if ((this->_material->supports() & rawrbox::MaterialFlags::PARTICLE) == 0) throw std::runtime_error("[RawrBox-ParticleSystem] Invalid material! ParticleSystem only supports `particle` materials!");
		}

		virtual rawrbox::Emitter& addEmitter(rawrbox::Emitter em);
		virtual void removeEmitter(rawrbox::Emitter* em);
		[[nodiscard]] virtual rawrbox::Emitter& get(size_t indx) const;
		// -----

		virtual void update();
		virtual void draw();
	};
} // namespace rawrbox
