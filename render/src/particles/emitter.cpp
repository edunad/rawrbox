
#include <rawrbox/engine/static.hpp>
#include <rawrbox/render/particles/emitter.hpp>

namespace rawrbox {
	Emitter::Emitter(rawrbox::EmitterSettings settings) : _settings(settings), _id(++rawrbox::EMITTER_ID), _timer(this->_settings.preHeat ? 1.F : 0.F){};
	Emitter::~Emitter() { this->clear(); }
	Emitter::Emitter(const Emitter& other) : _settings(other._settings), _id(other._id), _particles(other._particles), _pos(other._pos), _timer(other._timer) {}
	Emitter::Emitter(Emitter&& other) noexcept : _particles(std::move(other._particles)), _settings(other._settings), _id(other._id), _pos(other._pos), _timer(other._timer) {}

	void Emitter::spawnParticle() {
		// Calculate the next particle spawn time
		if (this->_settings.particlesPerSecond <= 0) return;

		rawrbox::Matrix4x4 mtx = {};
		mtx.rotate(this->_settings.angle);
		mtx.translate(this->_pos);
		mtx.scale({1.F, 1.F, 1.F});

		auto ppS = !this->_preHeated && this->_settings.preHeat ? this->_settings.maxParticles : this->_settings.particlesPerSecond;
		const float timePerParticle = !this->_preHeated && this->_settings.preHeat ? ppS : 1.0F / ppS;

		this->_timer += rawrbox::DELTA_TIME;
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
				case EmitterShape::SPHERE:
					pos = bx::randUnitSphere(&this->_rng);
					break;
				case EmitterShape::HEMISPHERE:
					pos = bx::randUnitHemisphere(&this->_rng, {0, 1.0F, 0});
					break;
				case EmitterShape::CIRCLE:
					pos = bx::randUnitCircle(&this->_rng);
					break;
				case EmitterShape::DISC:
					pos = bx::mul(bx::randUnitCircle(&this->_rng), bx::frnd(&this->_rng));
					break;
				default:
				case EmitterShape::RECT:
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
			particle.texture = static_cast<uint32_t>(bx::lerp(this->_settings.texture.x, this->_settings.texture.y, bx::frnd(&this->_rng)));

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

			time += timePerParticle; // When to spawn the next particle
			this->_particles.push_back(particle);
		}

		if (!this->_preHeated) this->_preHeated = true; // Done pre-heating
	}

	// UTILS -----
	void Emitter::clear() {
		this->_particles.clear();
		this->_rng.reset();
	}

	const size_t Emitter::id() const { return this->_id; }
	const size_t Emitter::totalParticles() const { return this->_particles.size(); }
	const rawrbox::EmitterSettings& Emitter::getSettings() const { return this->_settings; }
	const rawrbox::Vector3f& Emitter::getPos() const { return this->_pos; }

	void Emitter::setPos(const rawrbox::Vector3f& pos) {
		this->_pos = pos;
	}
	// ------

	void Emitter::update() {
		for (auto it2 = this->_particles.begin(); it2 != this->_particles.end();) {
			(*it2).life += rawrbox::DELTA_TIME / (*it2).lifeSpan;

			if ((*it2).life > 1.F) {
				it2 = this->_particles.erase(it2);
				continue;
			}

			++it2;
		}

		this->spawnParticle();
	}

} // namespace rawrbox
