#include <rawrbox/math/matrix4x4.hpp>
#include <rawrbox/scripting/wrappers/math/matrix_wrapper.hpp>

namespace rawrbox {
	void MatrixWrapper::registerLua(lua_State* L) {
		luabridge::getGlobalNamespace(L)
		    .beginClass<rawrbox::Matrix4x4>("Matrix")
		    .addConstructor<void(), void(const std::array<float, 16>)>()

		    .addFunction("size", &rawrbox::Matrix4x4::size)

		    .addFunction("zero", &rawrbox::Matrix4x4::zero)
		    .addFunction("identity", &rawrbox::Matrix4x4::identity)

		    .addFunction("transpose",
			luabridge::overload<>(&rawrbox::Matrix4x4::transpose),
			luabridge::overload<const rawrbox::Matrix4x4&>(&rawrbox::Matrix4x4::transpose))

		    .addFunction("get", [](const rawrbox::Matrix4x4& self, size_t index) {
			    return self[index];
		    })

		    .addFunction("translate", &rawrbox::Matrix4x4::translate)
		    .addFunction("scale", &rawrbox::Matrix4x4::scale)

		    .addFunction("rotate", &rawrbox::Matrix4x4::rotate)
		    .addFunction("rotateX", &rawrbox::Matrix4x4::rotateX)
		    .addFunction("rotateY", &rawrbox::Matrix4x4::rotateY)
		    .addFunction("rotateZ", &rawrbox::Matrix4x4::rotateZ)
		    .addFunction("rotateXYZ", &rawrbox::Matrix4x4::rotateXYZ)

		    .addFunction("ortho", &rawrbox::Matrix4x4::ortho)
		    .addFunction("proj", &rawrbox::Matrix4x4::proj)
		    .addFunction("billboard", &rawrbox::Matrix4x4::billboard)

		    .addFunction("SRT", &rawrbox::Matrix4x4::SRT)
		    .addFunction("inverse", &rawrbox::Matrix4x4::inverse)
		    .addFunction("lookAt", &rawrbox::Matrix4x4::lookAt)

		    .addStaticFunction("mtxTranspose", &rawrbox::Matrix4x4::mtxTranspose)
		    .addStaticFunction("mtxInverse", &rawrbox::Matrix4x4::mtxInverse)
		    .addStaticFunction("mtxSRT", &rawrbox::Matrix4x4::mtxSRT)
		    .addStaticFunction("mtxLookAt", &rawrbox::Matrix4x4::mtxLookAt)
		    .addStaticFunction("mtxOrtho", &rawrbox::Matrix4x4::mtxOrtho)
		    .addStaticFunction("mtxProj", &rawrbox::Matrix4x4::mtxProj)
		    .addStaticFunction("mtxProject", &rawrbox::Matrix4x4::mtxProject)

		    .addFunction("__mul", luabridge::overload<rawrbox::Vector3f>(&Matrix4x4::operator*), luabridge::overload<Matrix4x4>(&Matrix4x4::operator*))
		    .addFunction("__add", luabridge::overload<rawrbox::Vector3f>(&Matrix4x4::operator+), luabridge::overload<Matrix4x4>(&Matrix4x4::operator+))
		    .addFunction("__div", &rawrbox::Matrix4x4::operator/)
		    .addFunction("__eq", &rawrbox::Matrix4x4::operator==)
		    .addFunction("__ne", &rawrbox::Matrix4x4::operator!=)

		    .endClass();
	}
} // namespace rawrbox
