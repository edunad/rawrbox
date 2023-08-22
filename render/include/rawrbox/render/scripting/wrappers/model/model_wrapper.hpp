#pragma once

#include <rawrbox/math/matrix4x4.hpp>
#include <rawrbox/math/vector3.hpp>
#include <rawrbox/math/vector4.hpp>
#include <rawrbox/render/materials/base.hpp>
#include <rawrbox/render/scripting/wrappers/model/base_wrapper.hpp>

#include <sol/sol.hpp>

namespace rawrbox {
	class ModelWrapper : public rawrbox::ModelBaseWrapper {

	public:
		ModelWrapper(const std::shared_ptr<rawrbox::ModelBase>& ref);
		ModelWrapper(const ModelWrapper&) = default;
		ModelWrapper(ModelWrapper&&) = default;
		ModelWrapper& operator=(const ModelWrapper&) = default;
		ModelWrapper& operator=(ModelWrapper&&) = default;
		~ModelWrapper() override = default;

		// ANIMS ---
		virtual void playAnimation(const std::string& name, sol::optional<bool> loop, sol::optional<float> speed);
		virtual void stopAnimation(const std::string& name);
		// ---

		// UTILS ---
		virtual void setOptimizable(bool optimize);
		virtual void optimize();

		void setPos(const rawrbox::Vector3f& pos) override;
		void setAngle(const rawrbox::Vector4f& angle) override;
		void setEulerAngle(const rawrbox::Vector3f& angle) override;
		void setScale(const rawrbox::Vector3f& size) override;

		[[nodiscard]] virtual const rawrbox::BBOX getBBOX() const;
		[[nodiscard]] virtual size_t totalMeshes() const;
		[[nodiscard]] virtual bool empty() const;

		virtual void removeMeshByName(const std::string& id);
		virtual void removeMesh(size_t index);

		virtual rawrbox::Mesh* getMeshByName(const std::string& id);
		virtual rawrbox::Mesh* getMesh(sol::optional<size_t> id);
		// ---

		static void registerLua(sol::state& lua);
	};
} // namespace rawrbox
