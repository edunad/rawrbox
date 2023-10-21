#pragma once

namespace rawrbox {
	class PostProcessBase {
	protected:
	public:
		PostProcessBase() = default;
		PostProcessBase(PostProcessBase&&) = delete;
		PostProcessBase& operator=(PostProcessBase&&) = delete;
		PostProcessBase(const PostProcessBase&) = delete;
		PostProcessBase& operator=(const PostProcessBase&) = delete;

		virtual ~PostProcessBase() = default;

		virtual void upload() = 0;
		virtual void applyEffect() = 0;
	};
} // namespace rawrbox
