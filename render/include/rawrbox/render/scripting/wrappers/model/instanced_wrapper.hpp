#pragma once

#include <rawrbox/render/model/instance.hpp>
#include <rawrbox/render/scripting/wrappers/model/base_wrapper.hpp>
#include <rawrbox/render/scripting/wrappers/model/instance_wrapper.hpp>

#include <sol/sol.hpp>

namespace rawrbox {
	class InstancedModelWrapper : public rawrbox::ModelBaseWrapper {

	public:
		InstancedModelWrapper(const std::shared_ptr<rawrbox::ModelBase>& ref);
		InstancedModelWrapper(const InstancedModelWrapper&) = default;
		InstancedModelWrapper(InstancedModelWrapper&&) = default;
		InstancedModelWrapper& operator=(const InstancedModelWrapper&) = default;
		InstancedModelWrapper& operator=(InstancedModelWrapper&&) = default;
		~InstancedModelWrapper() override = default;

		// UTILS ----
		virtual void addInstance(const rawrbox::Instance& instance);
		virtual void removeInstance(size_t i = 0);
		virtual rawrbox::Instance& getInstance(size_t i = 0);
		// --------

		[[nodiscard]] virtual size_t count() const;

		// UPLOAD ---
		virtual void updateInstance();
		virtual void setAutoUpload(bool set);
		// -----

		static void registerLua(sol::state& lua);
	};
} // namespace rawrbox
