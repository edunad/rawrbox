#pragma once

#include <rawrbox/render/materials/instanced.hpp>
#include <rawrbox/render/models/instance.hpp>
#include <rawrbox/render/models/model.hpp>

#ifdef RAWRBOX_SCRIPTING
	#include <rawrbox/render/scripting/wrappers/model/instanced_wrapper.hpp>
	#include <sol/sol.hpp>
#endif

namespace rawrbox {
	class InstancedModel : public rawrbox::ModelBase {
	protected:
		// bgfx::DynamicVertexBufferHandle _dataBuffer = BGFX_INVALID_HANDLE;
		std::vector<rawrbox::Instance> _instances = {};
		bool _autoUpload = true;

		void updateBuffers() override;
#ifdef RAWRBOX_SCRIPTING
		void initializeLua() override;
#endif

	public:
		explicit InstancedModel(size_t instanceSize = 0);
		InstancedModel(const InstancedModel&) = delete;
		InstancedModel(InstancedModel&&) = delete;
		InstancedModel& operator=(const InstancedModel&) = delete;
		InstancedModel& operator=(InstancedModel&&) = delete;
		~InstancedModel() override;

		template <typename M = rawrbox::MaterialBase>
		void setMaterial() {
			this->_material = std::make_unique<M>();
			if ((this->_material->supports() & rawrbox::MaterialFlags::INSTANCED) == 0) throw std::runtime_error("[RawrBox-InstancedModel] Invalid material! InstancedModel only supports `instanced` materials!");
		}

		virtual void setAutoUpload(bool enabled);

		virtual void setTemplate(rawrbox::Mesh mesh);

		[[nodiscard]] virtual rawrbox::Mesh& getTemplate() const;

		virtual void addInstance(const rawrbox::Instance& instance);
		virtual void removeInstance(size_t i = 0);
		[[nodiscard]] rawrbox::Instance& getInstance(size_t i = 0);

		virtual std::vector<rawrbox::Instance>& instances();
		[[nodiscard]] virtual size_t count() const;

		void upload(bool /*dynamic*/ = false) override;

		virtual void updateInstance();

		void draw() override;
	};
} // namespace rawrbox
