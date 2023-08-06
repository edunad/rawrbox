#include <rawrbox/math/matrix4x4.hpp>
#include <rawrbox/scripting/wrappers/math/matrix_wrapper.hpp>

namespace rawrbox {
	void MatrixWrapper::registerLua(sol::state& lua) {
		lua.new_usertype<rawrbox::Matrix4x4>("Matrix",
		    sol::constructors<rawrbox::Matrix4x4(), rawrbox::Matrix4x4(const float*), rawrbox::Matrix4x4(const std::array<float, 16>)>(),

		    "zero", &Matrix4x4::zero,
		    "identity", &Matrix4x4::identity,

		    "size", &Matrix4x4::size,

		    "transpose", sol::overload(sol::resolve<void(const float*)>(&Matrix4x4::transpose), sol::resolve<void(const std::array<float, 16>&)>(&Matrix4x4::transpose)),

		    "translate", &Matrix4x4::translate,
		    "scale", &Matrix4x4::scale,
		    "billboard", &Matrix4x4::billboard,
		    "rotate", &Matrix4x4::rotate,
		    "rotateX", &Matrix4x4::rotateX,
		    "rotateY", &Matrix4x4::rotateY,
		    "rotateZ", &Matrix4x4::rotateZ,
		    "rotateXYZ", &Matrix4x4::rotateXYZ,
		    "mtxSRT", &Matrix4x4::mtxSRT,

		    "mul", sol::overload(sol::resolve<void(const rawrbox::Matrix4x4&)>(&Matrix4x4::mul), sol::resolve<void(const rawrbox::Vector3f&)>(&Matrix4x4::mul)),
		    "add", sol::overload(sol::resolve<void(const rawrbox::Matrix4x4&)>(&Matrix4x4::add), sol::resolve<void(const rawrbox::Vector3f&)>(&Matrix4x4::add)),

		    "mulVec", sol::overload(sol::resolve<rawrbox::Vector3f(const rawrbox::Vector3f&) const>(&Matrix4x4::mulVec), sol::resolve<rawrbox::Vector4f(const rawrbox::Vector4f&) const>(&Matrix4x4::mulVec)),

		    "inverse", &Matrix4x4::inverse,
		    "lookAt", &Matrix4x4::lookAt,
		    "project", &Matrix4x4::project,

		    sol::meta_function::equal_to, &Matrix4x4::operator==,
		    sol::meta_function::index, &Matrix4x4::operator[],
		    sol::meta_function::addition, sol::overload(sol::resolve<rawrbox::Matrix4x4(rawrbox::Matrix4x4) const>(&Matrix4x4::operator+), sol::resolve<rawrbox::Matrix4x4(rawrbox::Vector3f) const>(&Matrix4x4::operator+)),
		    sol::meta_function::multiplication, sol::overload(sol::resolve<rawrbox::Matrix4x4(rawrbox::Matrix4x4) const>(&Matrix4x4::operator*), sol::resolve<rawrbox::Matrix4x4(rawrbox::Vector3f) const>(&Matrix4x4::operator*)));
	}
} // namespace rawrbox
