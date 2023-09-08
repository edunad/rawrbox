#pragma once

#include <rawrbox/assimp/importer.hpp>
#include <rawrbox/render/model/model.hpp>

namespace rawrbox {

	class AssimpModel : public rawrbox::Model {
	protected:
		// INTERNAL -------
		void loadMeshes(const rawrbox::AssimpImporter& model);
		void loadAnimations(const rawrbox::AssimpImporter& model);
		void loadLights(const rawrbox::AssimpImporter& model);
		// -------------------

	public:
		AssimpModel() = default;
		AssimpModel(const AssimpModel&) = delete;
		AssimpModel(AssimpModel&&) = delete;
		AssimpModel& operator=(const AssimpModel&) = delete;
		AssimpModel& operator=(AssimpModel&&) = delete;
		~AssimpModel() override = default;

		void load(const rawrbox::AssimpImporter& model);
	};
} // namespace rawrbox
