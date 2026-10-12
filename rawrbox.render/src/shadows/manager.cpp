#include <rawrbox/render/shadows/manager.hpp>

#include <algorithm>

namespace rawrbox {
	// PRIVATE ----
	std::vector<rawrbox::ShadowCasterEntry> SHADOWS::_casters = {};
	std::vector<rawrbox::BBOX> SHADOWS::_invalid = {};

	uint32_t SHADOWS::_revision = 0;
	// ------------

	void SHADOWS::add(rawrbox::ShadowCaster& caster) {
		_casters.push_back({&caster});
		_revision++;
	}

	void SHADOWS::remove(rawrbox::ShadowCaster& caster) {
		const auto fnd = std::ranges::find(_casters, &caster, &rawrbox::ShadowCasterEntry::caster);

		if (fnd == _casters.end()) return;
		if (fnd->cached) _invalid.push_back(fnd->bounds);

		*fnd = _casters.back();
		_casters.pop_back();
		_revision++;
	}

	void SHADOWS::update() {
		for (auto& entry : _casters) {
			const bool casts = entry.caster->canCastShadows();

			// Update if changed to dynamic ---
			if (entry.cached && (!casts || entry.caster->getShadowMode() != rawrbox::ShadowMode::STATIC)) {
				_invalid.push_back(entry.bounds);
				entry.cached = false;
			}
			// -----------

			if (!casts) continue;
			const auto bounds = entry.caster->getShadowBounds();

			if (entry.caster->getShadowMode() == rawrbox::ShadowMode::STATIC) {
				if (!entry.cached) {
					_invalid.push_back(bounds);
					entry.cached = true;
				} else if (entry.bounds != bounds) {
					_invalid.push_back(entry.bounds);
					_invalid.push_back(bounds);
				}
			}

			entry.bounds = bounds;
		}
	}

	void SHADOWS::addInvalid(const rawrbox::BBOX& bounds) { _invalid.push_back(bounds); }
	void SHADOWS::clearInvalid() { _invalid.clear(); }

	const std::vector<rawrbox::BBOX>& SHADOWS::getInvalid() { return _invalid; }
	const std::vector<rawrbox::ShadowCasterEntry>& SHADOWS::getCasters() { return _casters; }

	uint32_t SHADOWS::revision() { return _revision; }
} // namespace rawrbox