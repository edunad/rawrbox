#pragma once
#include <rawrbox/math/vector3.hpp>
#include <rawrbox/render/camera/base.hpp>
#include <rawrbox/render/model/material/particle_unlit.hpp>
#include <rawrbox/render/particles/emitter.hpp>

#include <bgfx/bgfx.h>

#include <array>
#include <cstdlib>
#include <memory>
#include <stdexcept>

#define BGFX_STATE_DEFAULT_PARTICLE (0 | BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_DEPTH_TEST_LESS | BGFX_STATE_CULL_CW | BGFX_STATE_BLEND_NORMAL)

namespace rawrbox {
	template <typename M = rawrbox::MaterialParticleUnlit>
	class ParticleSystem {
	protected:
		// Drawing ----
		std::vector<std::shared_ptr<rawrbox::Emitter>> _emitters = {};
		std::unique_ptr<M> _material = std::make_unique<M>();
		// ---

		// TEXTURE ---
		std::shared_ptr<rawrbox::TextureBase> _texture = nullptr;
		// ----

		uint32_t totalParticles = 0;

	public:
		ParticleSystem() = default;
		virtual ~ParticleSystem() {
			this->_texture = nullptr;
			this->_material = nullptr;
			this->_emitters.clear();
		}

		ParticleSystem(ParticleSystem&&) = delete;
		ParticleSystem& operator=(ParticleSystem&&) = delete;
		ParticleSystem(const ParticleSystem&) = delete;
		ParticleSystem& operator=(const ParticleSystem&) = delete;

		static int32_t particleSortFn(const void* _lhs, const void* _rhs) {
			const rawrbox::ParticleSort& lhs = *(const rawrbox::ParticleSort*)_lhs;
			const rawrbox::ParticleSort& rhs = *(const rawrbox::ParticleSort*)_rhs;
			return lhs.dist > rhs.dist ? -1 : 1;
		}

		// UTILS -----
		virtual void setTexture(std::shared_ptr<rawrbox::TextureBase> texture) { this->_texture = texture; }
		[[nodiscard]] virtual std::shared_ptr<rawrbox::TextureBase> getTexture() const { return this->_texture; }

		void upload() {
			this->_material->upload();
			this->_material->registerUniforms();
		}

		void addEmitter(std::shared_ptr<rawrbox::Emitter> em) { this->_emitters.push_back(std::move(em)); }
		std::shared_ptr<rawrbox::Emitter> get(size_t indx) {
			if (indx >= this->_emitters.size()) throw std::runtime_error(fmt::format("[RawrBox-ParticleEngine] Emitter {} not found!", indx));
			return this->_emitters[indx];
		}
		// -----

		void update(float deltaTime) {
			totalParticles = 0;
			for (auto& em : this->_emitters) {
				em->update(deltaTime);
				totalParticles += static_cast<uint32_t>(em->totalParticles());
			}
		}

		void draw(std::shared_ptr<rawrbox::CameraBase> cam) {
			if (this->_emitters.empty() || totalParticles <= 0) return;

			int vertCount = 4; // Plane
			int indxCount = 6;

			const uint32_t numVertices = bgfx::getAvailTransientVertexBuffer(totalParticles * vertCount, M::vertexBufferType::vLayout());
			const uint32_t numIndices = bgfx::getAvailTransientIndexBuffer(totalParticles * indxCount);
			const uint32_t max = bx::uint32_min(numVertices / vertCount, numIndices / indxCount);

			this->_material->preProcess(cam->getPos());

			// BX_WARN(this->totalParticles == max, "Truncating transient buffer for particles to maximum available (requested %d, available %d).", this->totalParticles, max);
			if (max <= 0) return;
			bgfx::TransientVertexBuffer tvb = {};
			bgfx::TransientIndexBuffer tib = {};

			bgfx::allocTransientBuffers(&tvb, M::vertexBufferType::vLayout(), max * vertCount, &tib, max * indxCount);

			std::vector<rawrbox::ParticleSort> particleSort{max};
			uint32_t pos = 0;

			if (this->_texture != nullptr) this->_material->process(this->_texture->getHandle());

			auto* vertices = (typename M::vertexBufferType*)tvb.data;
			auto* indices = (uint16_t*)tib.data;

			for (auto& em : this->_emitters)
				pos += em->template draw<M>(cam->getViewMtx(), cam->getPos(), pos, max, particleSort.data(), vertices);

			std::qsort(particleSort.data(), max, sizeof(ParticleSort), particleSortFn);

			for (uint32_t ii = 0; ii < max; ++ii) {
				auto tri = [](uint16_t*& dest, uint16_t a, uint16_t b, uint16_t c) { *dest++ = a; *dest++ = b; *dest++ = c; };

				const uint16_t index = uint16_t(particleSort[ii].idx) * 4;
				uint16_t* dest = &indices[ii * 6];

				tri(dest, index + 0, index + 1, index + 2);
				tri(dest, index + 0, index + 3, index + 1);
			}

			bgfx::setState(BGFX_STATE_DEFAULT_PARTICLE, 0);

			bgfx::setVertexBuffer(0, &tvb);
			bgfx::setIndexBuffer(&tib);

			this->_material->postProcess();
		}
	};
} // namespace rawrbox
