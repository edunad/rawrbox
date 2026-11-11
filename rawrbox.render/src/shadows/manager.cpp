#include <rawrbox/render/shadows/manager.hpp>

#include <algorithm>

namespace rawrbox {
	// PRIVATE ----
	std::vector<rawrbox::ShadowCasterEntry> SHADOWS::_casters = {};
	std::vector<rawrbox::BBOX> SHADOWS::_invalid = {};

	uint32_t SHADOWS::_revision = 0;
	// ------------

	void SHADOWS::add(rawrbox::ShadowCaster& caster) {
		SHADOWS::_casters.push_back({&caster});
		SHADOWS::_revision++;
	}

	void SHADOWS::remove(rawrbox::ShadowCaster& caster) {
		const auto fnd = std::ranges::find(SHADOWS::_casters, &caster, &rawrbox::ShadowCasterEntry::caster);

		if (fnd == SHADOWS::_casters.end()) return;
		if (fnd->cached) SHADOWS::_invalid.push_back(fnd->bounds);

		*fnd = SHADOWS::_casters.back();
		SHADOWS::_casters.pop_back();
		SHADOWS::_revision++;
	}

	void SHADOWS::update() {
		for (auto& entry : SHADOWS::_casters) {
			const bool casts = entry.caster->canCastShadows();

			// Update if changed to dynamic ---
			if (entry.cached && (!casts || entry.caster->getShadowMode() != rawrbox::ShadowMode::STATIC)) {
				SHADOWS::_invalid.push_back(entry.bounds);
				entry.cached = false;
			}
			// -----------

			if (!casts) continue;
			const auto bounds = entry.caster->getShadowBounds();

			if (entry.caster->getShadowMode() == rawrbox::ShadowMode::STATIC) {
				if (!entry.cached) {
					SHADOWS::_invalid.push_back(bounds);
					entry.cached = true;
				} else if (entry.bounds != bounds) {
					SHADOWS::_invalid.push_back(entry.bounds);
					SHADOWS::_invalid.push_back(bounds);
				}
			}

			entry.bounds = bounds;
		}
	}

	void SHADOWS::addInvalid(const rawrbox::BBOX& bounds) { SHADOWS::_invalid.push_back(bounds); }
	void SHADOWS::clearInvalid() { SHADOWS::_invalid.clear(); }

	const std::vector<rawrbox::BBOX>& SHADOWS::getInvalid() { return SHADOWS::_invalid; }
	const std::vector<rawrbox::ShadowCasterEntry>& SHADOWS::getCasters() { return SHADOWS::_casters; }

	uint32_t SHADOWS::revision() { return SHADOWS::_revision; }
} // namespace rawrbox