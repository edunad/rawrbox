#pragma once
#include <rawrbox/render/model/instance.hpp>

#include <sol/sol.hpp>

namespace rawrbox {
	class InstanceWrapper {
	public:
		rawrbox::Instance ref;

		InstanceWrapper() = default;
		InstanceWrapper(const rawrbox::Instance& ref);

		rawrbox::Colori getColor();
		void setColor(const rawrbox::Colori& color);

		const rawrbox::Matrix4x4& getMatrix();
		void setMatrix(const rawrbox::Matrix4x4& mtrx);

		const rawrbox::Vector4f& getExtraData();
		void setExtraData(const rawrbox::Vector4f& data);

		static void registerLua(sol::state& lua);
	};
} // namespace rawrbox
