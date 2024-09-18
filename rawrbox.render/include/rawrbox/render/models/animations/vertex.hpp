#pragma once

#include <rawrbox/render/models/animations/base.hpp>

#include <stdexcept>

namespace rawrbox {
	class AnimationVertexSampler : public rawrbox::AnimationSampler {
	private:
		ozz::animation::SamplingJob::Context _context;
		ozz::vector<ozz::math::SoaTransform> _output;

	public:
		AnimationVertexSampler(size_t index, ozz::animation::Animation* anim) : rawrbox::AnimationSampler(index, anim) {
			this->_context.Resize(anim->num_tracks());
			this->_output.resize(anim->num_soa_tracks());
		}

		// UTILS ----
		virtual const ozz::vector<ozz::math::SoaTransform>& getOutput() {
			ozz::animation::SamplingJob job;
			job.context = &this->_context;
			job.animation = this->_animation;
			job.ratio = this->_currentTime;
			job.output = ozz::make_span(this->_output);

			if (!job.Run()) throw std::runtime_error("Failed to run animation job");
			return this->_output;
		}
		// -------------
	};
} // namespace rawrbox
