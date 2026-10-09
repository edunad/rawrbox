#include <rawrbox/render/models/animations/blender.hpp>
#include <rawrbox/render/models/animations/sampler.hpp>
#include <rawrbox/render/scripting/wrappers/models/animation.hpp>

namespace rawrbox {
	void AnimationWrapper::registerLua(lua_State* L) {
		luabridge::getGlobalNamespace(L)
		    .beginClass<rawrbox::AnimationIKBase>("AnimationIKBase")
		    .addProperty("target", &rawrbox::AnimationIKBase::target)
		    .addProperty("weight", &rawrbox::AnimationIKBase::weight)
		    .addProperty("reached", +[](const rawrbox::AnimationIKBase* self) { return self->reached; })
		    .endClass()

		    .deriveClass<rawrbox::AnimationIKAim, rawrbox::AnimationIKBase>("AnimationIKAim")
		    .addConstructor<void()>()
		    .addProperty("forward", &rawrbox::AnimationIKAim::forward)
		    .addProperty("up", &rawrbox::AnimationIKAim::up)
		    .addProperty("offset", &rawrbox::AnimationIKAim::offset)
		    .endClass()

		    .deriveClass<rawrbox::AnimationIK, rawrbox::AnimationIKBase>("AnimationIK")
		    .addConstructor<void()>()
		    .addProperty("pole", &rawrbox::AnimationIK::pole)
		    .addProperty("midAxis", &rawrbox::AnimationIK::midAxis)
		    .addProperty("soften", &rawrbox::AnimationIK::soften)
		    .addProperty("twist", &rawrbox::AnimationIK::twist)
		    .endClass()

		    .beginClass<rawrbox::Animation>("Animation")
		    .addProperty("name", +[](const rawrbox::Animation* self) { return self->name; })
		    .addProperty("duration", +[](const rawrbox::Animation* self) { return self->duration; })
		    .addFunction("empty", &rawrbox::Animation::empty)
		    .endClass()

		    .beginClass<rawrbox::AnimationSampler>("AnimationSampler")
		    .addFunction("getDuration", &rawrbox::AnimationSampler::getDuration)
		    .addFunction("getIndex", &rawrbox::AnimationSampler::getIndex)
		    .addFunction("getAnimation", &rawrbox::AnimationSampler::getAnimation)

		    .addFunction("getTime", &rawrbox::AnimationSampler::getTime)
		    .addFunction("setTime", &rawrbox::AnimationSampler::setTime)

		    .addFunction("getLoop", &rawrbox::AnimationSampler::getLoop)
		    .addFunction("setLoop", &rawrbox::AnimationSampler::setLoop)

		    .addFunction("getSpeed", &rawrbox::AnimationSampler::getSpeed)
		    .addFunction("setSpeed", &rawrbox::AnimationSampler::setSpeed)

		    .addFunction("getWeight", &rawrbox::AnimationSampler::getWeight)
		    .addFunction("setWeight", &rawrbox::AnimationSampler::setWeight)

		    .addFunction("setJointWeight", &rawrbox::AnimationSampler::setJointWeight)
		    .addFunction("clearJointWeights", &rawrbox::AnimationSampler::clearJointWeights)
		    .endClass();
	}
} // namespace rawrbox
