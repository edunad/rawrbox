#include <rawrbox/render/scripting/wrappers/decals_wrapper.hpp>

namespace rawrbox {

	// UTILS -----
	bool DecalsWrapper::add(const rawrbox::Vector3f& pos, const sol::optional<rawrbox::Vector3f> scale, const sol::optional<float> direction, const sol::optional<rawrbox::Colori> color, const sol::optional<uint16_t> atlasId) {
		return rawrbox::DECALS::add(pos, scale.value_or(rawrbox::Vector3f::one()), direction.value_or(0.F), color.value_or(rawrbox::Colorsi::White()).cast<float>(), atlasId.value_or(0));
	}

	bool DecalsWrapper::remove(size_t i) {
		return rawrbox::DECALS::remove(i);
	}

	const rawrbox::Instance& DecalsWrapper::get(size_t i) {
		return rawrbox::DECALS::get(i);
	}

	size_t DecalsWrapper::count() {
		return rawrbox::DECALS::count();
	}
	// ----------------

	void DecalsWrapper::registerLua(sol::state& lua) {
		lua.new_usertype<DecalsWrapper>("Decals",
		    sol::no_constructor,

		    // UTILS ----
		    "add", &DecalsWrapper::add,
		    "remove", &DecalsWrapper::remove,
		    "get", &DecalsWrapper::get,
		    "count", &DecalsWrapper::count
		    // ---------
		);
	}
} // namespace rawrbox
