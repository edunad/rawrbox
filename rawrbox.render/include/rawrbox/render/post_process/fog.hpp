#pragma once

#include <rawrbox/render/post_process/base.hpp>

namespace rawrbox {
	enum class FOG_TYPE : uint32_t {
		LINEAR = 0,
		EXP = 1,
		EXP_SQUARED = 2
	};

	class PostProcessFog : public rawrbox::PostProcessBase {
	public:
		PostProcessFog();
		PostProcessFog(PostProcessFog&&) = delete;
		PostProcessFog& operator=(PostProcessFog&&) = delete;
		PostProcessFog(const PostProcessFog&) = delete;
		PostProcessFog& operator=(const PostProcessFog&) = delete;
		~PostProcessFog() override = default;

		virtual void setColor(const rawrbox::Colorf& color);
		virtual void setType(rawrbox::FOG_TYPE end);
		virtual void setEnd(float end);
		virtual void setDensity(float density);

		void init() override;
	};
} // namespace rawrbox
