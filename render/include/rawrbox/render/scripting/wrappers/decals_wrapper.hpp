#pragma once

#include <rawrbox/render/decals/manager.hpp>
#include <rawrbox/render/scripting/wrappers/model/instance_wrapper.hpp>

#include <sol/sol.hpp>

namespace rawrbox {
	class DecalsWrapper {
	public:
		DecalsWrapper() = default;
		DecalsWrapper(const DecalsWrapper&) = default;
		DecalsWrapper(DecalsWrapper&&) = default;
		DecalsWrapper& operator=(const DecalsWrapper&) = default;
		DecalsWrapper& operator=(DecalsWrapper&&) = default;
		virtual ~DecalsWrapper() = default;

		// UTILS -----
		virtual bool add(const rawrbox::Vector3f& pos, const sol::optional<rawrbox::Vector3f> scale, const sol::optional<float> direction, const sol::optional<rawrbox::Colori> color, const sol::optional<uint16_t> atlasId);
		virtual bool remove(size_t i);
		virtual const rawrbox::InstanceWrapper get(size_t i);
		virtual size_t count();
		// -----

		static void registerLua(sol::state& lua);
	};
} // namespace rawrbox
