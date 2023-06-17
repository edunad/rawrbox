#pragma once

#include <rawrbox/math/vector4.hpp>
#include <rawrbox/render/camera/base.hpp>
#include <rawrbox/render/model/material/particle.hpp>
#include <rawrbox/render/particles/particle.hpp>
#include <rawrbox/render/utils/texture.hpp>

#include <bgfx/bgfx.h>
#include <bx/bounds.h>
#include <bx/easing.h>
#include <bx/math.h>
#include <bx/rng.h>

#include <vector>

// Adapted from https://github.com/bkaradzic/bgfx/blob/master/examples/32-particles/particles.cpp
namespace rawrbox {
	enum class EmitterShape {
		SPHERE = 0,
		HEMISPHERE,
		CIRCLE,
		DISC,
		RECT,
	};

	enum class EmitterDirection {
		UP = 0,
		OUTWARD,
	};

	struct EmitterSettings {
		EmitterShape shape = EmitterShape::RECT; // The random shape
		EmitterDirection direction = EmitterDirection::UP;

		rawrbox::Vector4f angle = {0, 0, 0, 0}; // In DEG
		rawrbox::Vector2f texture = {0, 0};     // Random between these 2 values

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

		// ROTATION ---
		rawrbox::Vector2f rotationStart = {0.F, 0.F}; // Random between these 2 values
		rawrbox::Vector2f rotationEnd = {90.F, 90.F}; // Random between these 2 values
		// ---

		/*
		   particle life time ------------------>
	       0 ---> 1 ----> 2 ----> 3 ----> 4 ----> 5
		*/
		std::array<uint32_t, 5> rgba = {0x00FFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0x00FFFFFF}; // Default: transparent -> white -> transparent

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

		size_t _id = 0;
		rawrbox::Vector3f _pos = {0, 0, 0};

		// PARTICLES ----
		float _timer = .0F;
		bool _preHeated = false;

		bx::RngMwc _rng = {};
		std::vector<rawrbox::Particle> _particles = {};
		// -------

		void spawnParticle();

		template <typename M = rawrbox::MaterialParticle>
		void write_vertex(typename M::vertexBufferType*& dest, typename M::vertexBufferType vertex)
			requires(supportsBlend<typename M::vertexBufferType>)
		{
			*dest = vertex;
			++dest;
		}

	public:
		explicit Emitter(EmitterSettings settings = {});

		// UTILS -----
		void clear();

		[[nodiscard]] const size_t id() const;
		[[nodiscard]] const size_t totalParticles() const;
		[[nodiscard]] const EmitterSettings& getSettings() const;

		[[nodiscard]] const rawrbox::Vector3f& getPos() const;
		void setPos(const rawrbox::Vector3f& pos);
		// ------

		void update();

		template <typename M = rawrbox::MaterialParticle>
		uint32_t draw(const rawrbox::CameraBase& camera, const rawrbox::Vector2i& atlasSize, uint32_t spriteSize, uint32_t first, uint32_t max, rawrbox::ParticleSort* outSort, typename M::vertexBufferType* outVert)
			requires(supportsBlend<typename M::vertexBufferType>)
		{
			bx::EaseFn easeRgba = bx::getEaseFunc(this->_settings.easeRgba);
			bx::EaseFn easePos = bx::getEaseFunc(this->_settings.easePos);
			bx::EaseFn easeScale = bx::getEaseFunc(this->_settings.easeScale);
			bx::EaseFn easeBlend = bx::getEaseFunc(this->_settings.easeBlend);
			// bx::EaseFn easeRotation = bx::getEaseFunc(this->_settings.easeRotation);

			uint32_t index = first;
			for (const auto& p : this->_particles) {
				if (index + 1 >= max) break;

				const float ttPos = easePos(p.life);
				const float ttScale = easeScale(p.life);
				const float ttBlend = easeBlend(p.life);
				// const float ttRotation = easeRotation(p.life);
				const float ttRgba = std::clamp(easeRgba(p.life), 0.F, 1.F);

				float scale = bx::lerp(p.scaleStart, p.scaleEnd, ttScale);
				// float rotation = bx::lerp(p.rotationStart, p.rotationEnd, ttRotation);
				float blend = bx::lerp(p.blendStart, p.blendEnd, ttBlend);

				// POSITION -----
				const bx::Vec3 p0 = bx::lerp({p.posStart.x, p.posStart.y, p.posStart.z}, {p.posEnd[0].x, p.posEnd[0].y, p.posEnd[0].z}, ttPos);
				const bx::Vec3 p1 = bx::lerp({p.posEnd[0].x, p.posEnd[0].y, p.posEnd[0].z}, {p.posEnd[1].x, p.posEnd[1].y, p.posEnd[1].z}, ttPos);
				const bx::Vec3 pf = bx::lerp(p0, p1, ttPos);

				rawrbox::Matrix4x4 rot = {};
				// rot.rotateZ(bx::toRad(rotation)); // TODO: FIX ME

				auto rotatedView = camera.getViewMtx() * rot;

				rawrbox::Vector3f udir = rawrbox::Vector3f(rotatedView[0], rotatedView[4], rotatedView[8]) * scale;
				rawrbox::Vector3f vdir = rawrbox::Vector3f(rotatedView[1], rotatedView[5], rotatedView[9]) * scale;

				const rawrbox::Vector3f pos = rawrbox::Vector3f(pf.x, pf.y, pf.z);
				// -------

				// SORTING --
				ParticleSort& sort = outSort[index];
				sort.dist = pos.distance(camera.getPos());
				sort.idx = index;
				// ----

				auto idx = static_cast<uint32_t>(ttRgba * 4);
				float ttmod = bx::mod(ttRgba, 0.25F) / 0.25F;

				uint32_t rgbaStart = p.rgba[idx];
				uint32_t rgbaEnd = idx + 1 >= p.rgba.size() ? 0x00FFFFFF : p.rgba[idx + 1];

				auto clStart = std::bit_cast<uint8_t*>(&rgbaStart);
				auto clEnd = std::bit_cast<uint8_t*>(&rgbaEnd);
				rawrbox::Colorf color = rawrbox::Colorf(
				    bx::lerp(clStart[0], clEnd[0], ttmod) / 255.F,
				    bx::lerp(clStart[1], clEnd[1], ttmod) / 255.F,
				    bx::lerp(clStart[2], clEnd[2], ttmod) / 255.F,
				    bx::lerp(clStart[3], clEnd[3], ttmod) / 255.F);

				auto uv = rawrbox::TextureUtils::atlasToUV(atlasSize, spriteSize, p.texture);
				typename M::vertexBufferType* vertex = &outVert[index * 4];

				this->write_vertex(vertex, rawrbox::VertexBlendData(pos - udir - vdir, {uv.x, uv.w, blend, 0.F}, color));
				this->write_vertex(vertex, rawrbox::VertexBlendData(pos + udir + vdir, {uv.z, uv.y, blend, 0.F}, color));
				this->write_vertex(vertex, rawrbox::VertexBlendData(pos - udir + vdir, {uv.x, uv.y, blend, 0.F}, color));
				this->write_vertex(vertex, rawrbox::VertexBlendData(pos + udir - vdir, {uv.z, uv.w, blend, 0.F}, color));

				++index;
			}

			return static_cast<uint32_t>(this->_particles.size());
		}
	};
} // namespace rawrbox
