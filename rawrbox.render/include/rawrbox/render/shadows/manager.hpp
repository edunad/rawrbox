#pragma once

#include <rawrbox/math/bbox.hpp>
#include <rawrbox/render/shadows/caster.hpp>

#include <vector>

namespace rawrbox {
	struct ShadowCasterEntry {
		rawrbox::ShadowCaster* caster = nullptr;
		rawrbox::BBOX bounds = {};

		bool cached = false;
	};

	class SHADOWS {
	protected:
		static std::vector<rawrbox::ShadowCasterEntry> _casters;
		static std::vector<rawrbox::BBOX> _invalid;

		static uint32_t _revision;

	public:
		static void add(rawrbox::ShadowCaster& caster);
		static void remove(rawrbox::ShadowCaster& caster);

		static void update();
		
		static void addInvalid(const rawrbox::BBOX& bounds);
		static void clearInvalid();

		[[nodiscard]] static const std::vector<rawrbox::BBOX>& getInvalid();
		[[nodiscard]] static const std::vector<rawrbox::ShadowCasterEntry>& getCasters();
		[[nodiscard]] static uint32_t revision();
	};
} // namespace rawrbox