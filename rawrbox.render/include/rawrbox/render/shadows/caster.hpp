#pragma once

#include <rawrbox/math/bbox.hpp>

#include <cstdint>
#include <functional>

namespace rawrbox {
	enum class ShadowMode : uint32_t {
		NONE = 0,
		STATIC,
		DYNAMIC,

		COUNT
	};

	struct ShadowSubmit {
		rawrbox::BBOX bounds = {};

		uint32_t indices = 0;
		uint32_t firstIndex = 0;
		uint32_t baseVertex = 0;

		uint32_t instances = 1;
		uint32_t view = 0;
	};

	struct ShadowDraw {
		static constexpr uint32_t ALL_VIEWS = 0xFFFFFFFFU;

		uint32_t views = 0;
		bool bind = true;

		std::function<void(uint32_t view)> setView = {};
		std::function<bool(const rawrbox::BBOX& bounds)> isVisible = {};
		std::function<void(const rawrbox::ShadowSubmit& submit)> submit = {};
	};

	class ShadowCaster {
	protected:
		rawrbox::ShadowMode _shadowMode = rawrbox::ShadowMode::NONE;

	public:
		ShadowCaster() = default;
		ShadowCaster(const ShadowCaster&) = delete;
		ShadowCaster(ShadowCaster&&) = delete;
		ShadowCaster& operator=(const ShadowCaster&) = delete;
		ShadowCaster& operator=(ShadowCaster&&) = delete;
		virtual ~ShadowCaster();

		virtual void setShadowMode(rawrbox::ShadowMode mode);
		[[nodiscard]] virtual rawrbox::ShadowMode getShadowMode() const;

		[[nodiscard]] virtual bool canCastShadows() const = 0;
		[[nodiscard]] virtual rawrbox::BBOX getShadowBounds() = 0;
		
		virtual void drawShadow(const rawrbox::ShadowDraw& draw) = 0;
	};
} // namespace rawrbox