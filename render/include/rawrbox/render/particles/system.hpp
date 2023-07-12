#pragma once

#include <rawrbox/render/particles/emitter.hpp>
#include <rawrbox/render/texture/atlas.hpp>

#define BGFX_STATE_DEFAULT_PARTICLE (0 | BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_WRITE_Z | BGFX_STATE_DEPTH_TEST_LESS | BGFX_STATE_CULL_CW | BGFX_STATE_BLEND_NORMAL)

namespace rawrbox {
	template <typename M = rawrbox::MaterialParticle>
	class ParticleSystem {
	protected:
		// Drawing ----
		std::vector<std::unique_ptr<rawrbox::Emitter>> _emitters = {};
		std::unique_ptr<M> _material = std::make_unique<M>();
		// ---

		// TEXTURE ---
		rawrbox::TextureAtlas* _atlas = nullptr;
		// ----

		uint32_t _totalParticles = 0;

	public:
		explicit ParticleSystem(rawrbox::TextureAtlas& spriteAtlas) : _atlas(&spriteAtlas){};

		static int32_t particleSortFn(const void* _lhs, const void* _rhs) {
			const rawrbox::ParticleSort& lhs = *std::bit_cast<const rawrbox::ParticleSort*>(_lhs);
			const rawrbox::ParticleSort& rhs = *std::bit_cast<const rawrbox::ParticleSort*>(_rhs);
			return lhs.dist > rhs.dist ? -1 : 1;
		}

		// UTILS -----
		[[nodiscard]] rawrbox::TextureAtlas* getTexture() const { return this->_atlas; }

		void upload() {
			this->_material->upload();
			this->_material->registerUniforms();
		}

		rawrbox::Emitter& addEmitter(rawrbox::Emitter em) {
			return *this->_emitters.emplace_back(std::make_unique<rawrbox::Emitter>(em));
		}

		void removeEmitter(rawrbox::Emitter* em) {
			auto iter = std::find_if(this->_emitters.begin(), this->_emitters.end(), [&](auto& e) { return em == e.get(); });
			if (iter == this->_emitters.end()) return;

			em->clear();
			this->_emitters.erase(iter);
		}

		[[nodiscard]] rawrbox::Emitter& get(size_t indx) const {
			if (indx >= this->_emitters.size()) throw std::runtime_error(fmt::format("[RawrBox-ParticleEngine] Emitter {} not found!", indx));
			return *this->_emitters[indx];
		}
		// -----

		void update() {
			this->_totalParticles = 0;

			for (auto& em : this->_emitters) {
				em->update();
				this->_totalParticles += static_cast<uint32_t>(em->totalParticles());
			}
		}

		void draw(const rawrbox::CameraBase& cam) {
			if (this->_emitters.empty() || this->_totalParticles <= 0) return;

			int vertCount = 4; // Plane
			int indxCount = 6;

			const auto layout = M::vLayout();
			const uint32_t numVertices = bgfx::getAvailTransientVertexBuffer(this->_totalParticles * vertCount, layout);
			const uint32_t numIndices = bgfx::getAvailTransientIndexBuffer(this->_totalParticles * indxCount);
			const uint32_t max = std::min(numVertices / vertCount, numIndices / indxCount);

			if (max != this->_totalParticles) fmt::print("[RawrBox-ParticleEngine] Truncating transient buffer for particles to maximum available (requested {}, available {}) \n", this->_totalParticles, max);
			if (max <= 0) return;

			bgfx::TransientVertexBuffer tvb = {};
			bgfx::TransientIndexBuffer tib = {};

			bgfx::allocTransientBuffers(&tvb, layout, max * vertCount, &tib, max * indxCount);

			std::vector<rawrbox::ParticleSort> particleSort{max};
			uint32_t pos = 0;

			this->_material->process(this->_atlas->getHandle());

			auto* vertices = std::bit_cast<rawrbox::VertexData*>(tvb.data);
			auto* indices = std::bit_cast<uint16_t*>(tib.data);

			for (auto& em : this->_emitters) {
				pos += em->draw(cam, pos, max, particleSort.data(), vertices);
			}

			std::qsort(particleSort.data(), max, sizeof(ParticleSort), particleSortFn);

			for (uint32_t ii = 0; ii < max; ++ii) {
				auto tri = [](uint16_t*& dest, uint16_t a, uint16_t b, uint16_t c) { *dest++ = a; *dest++ = b; *dest++ = c; };

				const uint16_t index = static_cast<uint16_t>(particleSort[ii].idx) * 4;
				uint16_t* dest = &indices[ii * 6];

				tri(dest, index + 0, index + 1, index + 2);
				tri(dest, index + 0, index + 3, index + 1);
			}

			bgfx::setVertexBuffer(0, &tvb);
			bgfx::setIndexBuffer(&tib);
			bgfx::setState(BGFX_STATE_DEFAULT_PARTICLE, 0);

			this->_material->postProcess();
			bgfx::discard();
		}
	};
} // namespace rawrbox
