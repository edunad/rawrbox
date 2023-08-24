#pragma once

#include <rawrbox/math/vector4.hpp>
#include <rawrbox/render/camera/base.hpp>
#include <rawrbox/render/materials/particle.hpp>
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

		virtual void spawnParticle();
		virtual void write_vertex(rawrbox::VertexData*& dest, rawrbox::VertexData vertex);

	public:
		explicit Emitter(EmitterSettings settings = {});
		Emitter(const Emitter&) = default;
		Emitter(Emitter&&) = delete;
		Emitter& operator=(const Emitter&) = default;
		Emitter& operator=(Emitter&&) = delete;
		virtual ~Emitter() = default;

		// UTILS -----
		virtual void clear();

		[[nodiscard]] virtual size_t id() const;
		[[nodiscard]] virtual size_t totalParticles() const;
		[[nodiscard]] virtual const EmitterSettings& getSettings() const;

		[[nodiscard]] virtual const rawrbox::Vector3f& getPos() const;
		virtual void setPos(const rawrbox::Vector3f& pos);
		// ------

		virtual void update();
		virtual uint32_t draw(uint32_t first, uint32_t max, rawrbox::ParticleSort* outSort, rawrbox::VertexData* outVert);
	};
} // namespace rawrbox
