#include <rawrbox/math/matrix4x4.hpp>
#include <rawrbox/scripting/wrappers/math/matrix_wrapper.hpp>

namespace rawrbox {
	void MatrixWrapper::registerLua(sol::state& lua) {
		lua.new_usertype<rawrbox::Matrix4x4>("Matrix",
		    sol::constructors<rawrbox::Matrix4x4(), rawrbox::Matrix4x4(const float*), rawrbox::Matrix4x4(const std::array<float, 16>)>(),

		    "zero", &Matrix4x4::zero,
		    "identity", &Matrix4x4::identity,

		    "size", &Matrix4x4::size,

		    //"transpose", &Matrix4x4::transpose,

		    "translate", &Matrix4x4::translate,
		    "scale", &Matrix4x4::scale,
		    "billboard", &Matrix4x4::billboard,
		    "rotate", &Matrix4x4::rotate,
		    "rotateX", &Matrix4x4::rotateX,
		    "rotateY", &Matrix4x4::rotateY,
		    "rotateZ", &Matrix4x4::rotateZ,
		    "rotateXYZ", &Matrix4x4::rotateXYZ,
		    "mtxSRT", &Matrix4x4::mtxSRT,
		    //"mul", &Matrix4x4::mul,
		    //"add", &Matrix4x4::add,
		    //"mulVec", &Matrix4x4::mulVec,
		    "inverse", &Matrix4x4::inverse,
		    "lookAt", &Matrix4x4::lookAt,
		    "project", &Matrix4x4::project);
	}
} // namespace rawrbox
