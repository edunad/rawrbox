#pragma once

#include <rawrbox/math/color.hpp>
#include <rawrbox/math/vector3.hpp>
#include <rawrbox/render/model/material/particle_unlit.hpp>
#include <rawrbox/render/particles/particle.hpp>
#include <rawrbox/render/static.hpp>
#include <rawrbox/render/texture/base.hpp>
#include <rawrbox/utils/pack.hpp>

#include <algorithm>
#include <cstdint>
#include <stdexcept>

#include "rawrbox/math/utils/math.hpp"
#include "rawrbox/utils/time.hpp"

#ifdef RAWRBOX_DEBUG
	#ifndef RAWRBOX_TESTING
		#include <rawrbox/debug/gizmos.hpp>
	#endif
#endif

#include <bgfx/bgfx.h>
#include <bx/bounds.h>
#include <bx/easing.h>
#include <bx/math.h>
#include <bx/rng.h>

#include <array>
#include <memory>
#include <vector>

// Adapted from https://github.com/bkaradzic/bgfx/blob/master/examples/32-particles/particles.cpp
namespace rawrbox {
	enum EmitterShape {
		SPHERE = 0,
		HEMISPHERE,
		CIRCLE,
		DISC,
		RECT,
	};

	enum EmitterDirection {
		UP = 0,
		OUTWARD,
	};

	struct EmitterSettings {
		EmitterShape shape = EmitterShape::RECT;
		EmitterDirection direction = EmitterDirection::UP;

		// OFFSETS ---
		rawrbox::Vector2f offsetStart = {0.F, 0.F}; // Random between these 2 values
		rawrbox::Vector2f offsetEnd = {1.F, 1.F};   // Random between these 2 values
		// ---

		// SCALE ---
		rawrbox::Vector2f scaleStart = {0.F, 0.F}; // Random between these 2 values
		rawrbox::Vector2f scaleEnd = {0.5F, 0.5F}; // Random between these 2 values
		// ---

		// BLEND ---
		rawrbox::Vector2f blendStart = {0.F, 0.F}; // Random between these 2 values
		rawrbox::Vector2f blendEnd = {0.2F, 0.2F}; // Random between these 2 values
		// ---

		// BLEND ---
		rawrbox::Vector2f rotationStart = {0.F, 0.F}; // Random between these 2 values
		rawrbox::Vector2f rotationEnd = {90.F, 90.F}; // Random between these 2 values
		// ---

		/*
		   particle life time ------------------>
	       0 ---> 1 ----> 2 ----> 3 ----> 4 ----> 5
		*/
		std::array<uint32_t, 5> rgba = {0x00FFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0x00FFFFFF}; // Default: Transparent to white to transparent

		std::array<float, 2> lifeSpan = {2.F, 2.F}; // Random between these 2 values
		float gravityScale = 0.F;

		uint32_t particlesPerSecond = 2;
		uint32_t maxParticles = 100;

		bool preHeat = false;

		bx::Easing::Enum easePos = bx::Easing::Linear;
		bx::Easing::Enum easeRgba = bx::Easing::Linear;
		bx::Easing::Enum easeBlend = bx::Easing::Linear;
		bx::Easing::Enum easeScale = bx::Easing::Linear;
		bx::Easing::Enum easeRotation = bx::Easing::Linear;

		EmitterSettings() = default;
	};

	class Emitter {
	protected:
		// SETTINGS ----
		EmitterSettings _settings = {};
		// -----

		rawrbox::Vector3f _pos = {0, 0, 0};
		float _timer = .0F;
		bool _preHeated = false;

		size_t _id = 0;
		bx::RngMwc _rng;

		// PARTICLES ----
		std::vector<rawrbox::Particle> _particles = {};
		// -------

		void spawnParticle(float deltaTime) {
			// Calculate the next particle spawn time
			if (this->_settings.particlesPerSecond <= 0) return;

			std::array<float, 16> mtx = {};
			bx::mtxSRT(mtx.data(), 1.0F, 1.0F, 1.0F, 0, 0, 0, this->getPos().x, this->getPos().y, this->getPos().z);

			auto ppS = !this->_preHeated && this->_settings.preHeat ? this->_settings.maxParticles : this->_settings.particlesPerSecond;
			const float timePerParticle = !this->_preHeated && this->_settings.preHeat ? ppS : 1.0F / ppS;

			this->_timer += deltaTime;
			const auto numParticles = static_cast<uint32_t>(this->_timer / timePerParticle);
			this->_timer -= numParticles * timePerParticle;
			// -------

			// Spawn ---
			float time = 0.F;
			for (uint32_t ii = 0; ii < numParticles && this->_particles.size() < this->_settings.maxParticles; ++ii) {
				Particle particle = {};

				// Randomize position -----
				bx::Vec3 pos(bx::InitNone);
				switch (this->_settings.shape) {
					case SPHERE:
						pos = bx::randUnitSphere(&this->_rng);
						break;
					case HEMISPHERE:
						pos = bx::randUnitHemisphere(&this->_rng, {0, 1.0F, 0});
						break;
					case CIRCLE:
						pos = bx::randUnitCircle(&this->_rng);
						break;
					case DISC:
						pos = bx::mul(bx::randUnitCircle(&this->_rng), bx::frnd(&this->_rng));
						break;
					default:
					case RECT:
						pos = {
						    bx::frndh(&this->_rng),
						    bx::frndh(&this->_rng),
						    bx::frndh(&this->_rng),
						};
						break;
				}
				// ------

				// Calculate direction -----
				bx::Vec3 dir(bx::InitNone);
				switch (this->_settings.direction) {
					default:
					case EmitterDirection::UP:
						dir = {0, 1.0F, 0};
						break;

					case EmitterDirection::OUTWARD:
						dir = bx::normalize(pos);
						break;
				}
				// --------

				// ----

				particle.life = time;
				particle.lifeSpan = bx::lerp(this->_settings.lifeSpan[0], this->_settings.lifeSpan[1], bx::frnd(&this->_rng));

				// Copy color settings ---
				particle.rgba = this->_settings.rgba;
				// -----

				// Set pos settings ---
				const float startOffset = bx::lerp(this->_settings.offsetStart.x, this->_settings.offsetStart.y, bx::frnd(&this->_rng));
				const bx::Vec3 start = bx::mul(pos, startOffset);

				const float endOffset = bx::lerp(this->_settings.offsetEnd.x, this->_settings.offsetEnd.y, bx::frnd(&this->_rng));
				const bx::Vec3 tmp1 = bx::mul(dir, endOffset);
				const bx::Vec3 end = bx::add(tmp1, start);

				const bx::Vec3 gravity = {0.0F, -9.81F * this->_settings.gravityScale * bx::square(particle.lifeSpan), 0.0F};

				particle.posStart = bx::mul(start, mtx.data());
				particle.posEnd[0] = bx::mul(end, mtx.data());
				particle.posEnd[1] = bx::add(particle.posEnd[0], gravity);
				// ----

				// Rotation settings ----
				particle.rotationStart = bx::lerp(this->_settings.rotationStart.x, this->_settings.rotationStart.y, bx::frnd(&this->_rng));
				particle.rotationEnd = bx::lerp(this->_settings.rotationEnd.x, this->_settings.rotationEnd.y, bx::frnd(&this->_rng));
				// -------

				// Scale settings ----
				particle.scaleStart = bx::lerp(this->_settings.scaleStart.x, this->_settings.scaleStart.y, bx::frnd(&this->_rng));
				particle.scaleEnd = bx::lerp(this->_settings.scaleEnd.x, this->_settings.scaleEnd.y, bx::frnd(&this->_rng));
				// -------

				// BLEND settings ----
				particle.blendStart = bx::lerp(this->_settings.blendStart.x, this->_settings.blendStart.y, bx::frnd(&this->_rng));
				particle.blendEnd = bx::lerp(this->_settings.blendEnd.x, this->_settings.blendEnd.y, bx::frnd(&this->_rng));
				// -------------

				time += timePerParticle;
				this->_particles.push_back(particle);
			}

			this->_preHeated = true;
		}

		template <typename M = rawrbox::MaterialParticleUnlit>
		void write_vertex(typename M::vertexBufferType*& dest, typename M::vertexBufferType vertex)
			requires(supportsBlend<typename M::vertexBufferType>)
		{
			*dest = vertex;
			++dest;
		}

	public:
		explicit Emitter(EmitterSettings settings = {}) : _settings(settings), _timer(this->_settings.preHeat ? 1.F : 0.F), _id(++rawrbox::EMITTER_ID) {
#ifdef RAWRBOX_DEBUG
	#ifndef RAWRBOX_TESTING
			rawrbox::GIZMOS::get().addEmitter(this);
	#endif
#endif
		};

		virtual ~Emitter() {
			this->clear();

#ifdef RAWRBOX_DEBUG
	#ifndef RAWRBOX_TESTING
			rawrbox::GIZMOS::get().removeEmitter(this);
	#endif
#endif
		};

		Emitter(Emitter&&) = delete;
		Emitter& operator=(Emitter&&) = delete;
		Emitter(const Emitter&) = delete;
		Emitter& operator=(const Emitter&) = delete;

		virtual void clear() {
			this->_particles.clear();
			this->_rng.reset();
		}

		[[nodiscard]] virtual const size_t id() const { return this->_id; }
		[[nodiscard]] virtual const size_t totalParticles() const { return this->_particles.size(); }

		[[nodiscard]] virtual const rawrbox::Vector3f& getPos() const { return this->_pos; }
		virtual void setPos(const rawrbox::Vector3f& pos) {
			this->_pos = pos;

#ifdef RAWRBOX_DEBUG
	#ifndef RAWRBOX_TESTING
			rawrbox::GIZMOS::get().updateGizmo(fmt::format("Emitter-{}", this->_id), pos);
	#endif
#endif
		}

		[[nodiscard]] virtual const EmitterSettings& getSettings() const { return this->_settings; }

		void update(float deltaTime) {
			for (auto it2 = this->_particles.begin(); it2 != this->_particles.end();) {
				(*it2).life += deltaTime / (*it2).lifeSpan;

				if ((*it2).life > 1.F) {
					it2 = this->_particles.erase(it2);
					continue;
				}

				++it2;
			}

			this->spawnParticle(deltaTime);
		}

		template <typename M = rawrbox::MaterialParticleUnlit>
		uint32_t draw(const std::array<float, 16>& viewMtx, const rawrbox::Vector3f& eye, uint32_t first, uint32_t max, rawrbox::ParticleSort* outSort, typename M::vertexBufferType* outVert)
			requires(supportsBlend<typename M::vertexBufferType>)
		{
			bx::EaseFn easeRgba = bx::getEaseFunc(this->_settings.easeRgba);
			bx::EaseFn easePos = bx::getEaseFunc(this->_settings.easePos);
			bx::EaseFn easeScale = bx::getEaseFunc(this->_settings.easeScale);
			bx::EaseFn easeBlend = bx::getEaseFunc(this->_settings.easeBlend);
			bx::EaseFn easeRotation = bx::getEaseFunc(this->_settings.easeRotation);

			uint32_t index = first;
			for (const auto& p : this->_particles) {
				if (index + 1 >= max) break;

				const float ttPos = easePos(p.life);
				const float ttScale = easeScale(p.life);
				const float ttBlend = easeBlend(p.life);
				const float ttRotation = easeRotation(p.life);
				const float ttRgba = std::clamp(easeRgba(p.life), 0.F, 1.F);

				float scale = bx::lerp(p.scaleStart, p.scaleEnd, ttScale);
				float rotation = bx::lerp(p.rotationStart, p.rotationEnd, ttRotation);
				float blend = bx::lerp(p.blendStart, p.blendEnd, ttBlend);

				// POSITION -----
				const bx::Vec3 p0 = bx::lerp({p.posStart.x, p.posStart.y, p.posStart.z}, {p.posEnd[0].x, p.posEnd[0].y, p.posEnd[0].z}, ttPos);
				const bx::Vec3 p1 = bx::lerp({p.posEnd[0].x, p.posEnd[0].y, p.posEnd[0].z}, {p.posEnd[1].x, p.posEnd[1].y, p.posEnd[1].z}, ttPos);
				const bx::Vec3 pf = bx::lerp(p0, p1, ttPos);

				auto rot = rawrbox::MathUtils::mtxQuaternion(bx::toRad(rotation), 0, 0, 0);
				auto rotatedView = rawrbox::MathUtils::mtxMul(rot, viewMtx);

				rawrbox::Vector3f udir = rawrbox::Vector3f(rotatedView[0], rotatedView[4], rotatedView[8]) * scale;
				rawrbox::Vector3f vdir = rawrbox::Vector3f(rotatedView[1], rotatedView[5], rotatedView[9]) * scale;

				const rawrbox::Vector3f pos = rawrbox::Vector3f(pf.x, pf.y, pf.z);
				// -------

				// SORTING --
				ParticleSort& sort = outSort[index];
				sort.dist = pos.distance(eye);
				sort.idx = index;
				// ----

				auto idx = static_cast<uint32_t>(ttRgba * 4);
				float ttmod = bx::mod(ttRgba, 0.25F) / 0.25F;

				uint32_t rgbaStart = p.rgba[idx];
				uint32_t rgbaEnd = p.rgba[idx + 1];

				// NOLINTBEGIN(cppcoreguidelines-pro-type-cstyle-cast)
				float rr = bx::lerp(((uint8_t*)&rgbaStart)[0], ((uint8_t*)&rgbaEnd)[0], ttmod) / 255.0F;
				float gg = bx::lerp(((uint8_t*)&rgbaStart)[1], ((uint8_t*)&rgbaEnd)[1], ttmod) / 255.0F;
				float bb = bx::lerp(((uint8_t*)&rgbaStart)[2], ((uint8_t*)&rgbaEnd)[2], ttmod) / 255.0F;
				float aa = bx::lerp(((uint8_t*)&rgbaStart)[3], ((uint8_t*)&rgbaEnd)[3], ttmod) / 255.0F;
				// NOLINTEND(cppcoreguidelines-pro-type-cstyle-cast)

				auto color = rawrbox::Colorf(rr, gg, bb, aa);

				typename M::vertexBufferType* vertex = &outVert[index * 4];
				if constexpr (supportsNormals<typename M::vertexBufferType>) {
					this->write_vertex(vertex, rawrbox::VertexLitBlendData(pos - udir - vdir, rawrbox::PackUtils::packNormal(1, 0, 0), 0, 0, 1, blend, color));
					this->write_vertex(vertex, rawrbox::VertexLitBlendData(pos + udir + vdir, rawrbox::PackUtils::packNormal(1, 0, 0), 0, 1, 0, blend, color));
					this->write_vertex(vertex, rawrbox::VertexLitBlendData(pos - udir + vdir, rawrbox::PackUtils::packNormal(1, 0, 0), 0, 0, 0, blend, color));
					this->write_vertex(vertex, rawrbox::VertexLitBlendData(pos + udir - vdir, rawrbox::PackUtils::packNormal(1, 0, 0), 0, 1, 1, blend, color));
				} else {
					this->write_vertex(vertex, rawrbox::VertexBlendData(pos - udir - vdir, 0, 1, blend, color));
					this->write_vertex(vertex, rawrbox::VertexBlendData(pos + udir + vdir, 1, 0, blend, color));
					this->write_vertex(vertex, rawrbox::VertexBlendData(pos - udir + vdir, 0, 0, blend, color));
					this->write_vertex(vertex, rawrbox::VertexBlendData(pos + udir - vdir, 1, 1, blend, color));
				}

				++index;
			}

			return static_cast<uint32_t>(this->_particles.size());
		}
	};
} // namespace rawrbox
