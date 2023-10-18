#pragma once
#include <rawrbox/math/matrix4x4.hpp>

#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace rawrbox {

	struct Skeleton;
	struct Bone {
	private:
		rawrbox::Vector3f _scale = {1, 1, 1};
		rawrbox::Vector3f _pos = {};
		rawrbox::Vector4f _angle = {};

	public:
		std::string name = "";
		uint8_t boneId = 0;

		// Rendering ---
		rawrbox::Matrix4x4 transformationMtx = {};
		rawrbox::Matrix4x4 offsetMtx = {};
		rawrbox::Matrix4x4 overrideMtx = {};
		// ----

		// Lookup ----
		rawrbox::Skeleton* owner = nullptr;
		rawrbox::Bone* parent = nullptr;

		std::vector<std::unique_ptr<rawrbox::Bone>> children = {};
		// ----

		Bone(std::string id) : name(std::move(id)) {}

		[[nodiscard]] const rawrbox::Vector3f& getPos() const { return this->_pos; }
		void setPos(const rawrbox::Vector3f& pos) {
			this->_pos = pos;
			this->overrideMtx.mtxSRT(this->_scale, this->_angle, this->_pos);
		}

		[[nodiscard]] const rawrbox::Vector4f& getAngle() const { return this->_angle; }
		void setAngle(const rawrbox::Vector4f& ang) {
			this->_angle = ang;
			this->overrideMtx.mtxSRT(this->_scale, this->_angle, this->_pos);
		}

		void setEulerAngle(const rawrbox::Vector3f& ang) {
			this->_angle = rawrbox::Vector4f::toQuat(ang);
			this->overrideMtx.mtxSRT(this->_scale, this->_angle, this->_pos);
		}

		[[nodiscard]] const rawrbox::Vector3f& getScale() const { return this->_scale; }
		void setScale(const rawrbox::Vector3f& scale) {
			this->_scale = scale;
			this->overrideMtx.mtxSRT(this->_scale, this->_angle, this->_pos);
		}
	};

	struct Skeleton {
		std::string name = "Armature";
		std::unique_ptr<rawrbox::Bone> rootBone = nullptr;

		std::unordered_map<std::string, rawrbox::Bone*> boneMap = {}; // Map for quick lookup
		rawrbox::Matrix4x4 invTransformationMtx = {};

		[[nodiscard]] rawrbox::Bone* getBone(const std::string& boneName) const {
			auto fnd = boneMap.find(boneName);
			if (fnd == boneMap.end()) return nullptr;
			return fnd->second;
		}

		[[nodiscard]] rawrbox::Bone* getBone(size_t indx) const {
			for (auto& i : this->boneMap) {
				if (i.second->boneId == indx) return i.second;
			}

			return nullptr;
		}

		explicit Skeleton(std::string _name) : name(std::move(_name)) {}
	};
} // namespace rawrbox
