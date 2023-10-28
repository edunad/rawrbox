
#include <rawrbox/render/models/model.hpp>

#ifdef RAWRBOX_SCRIPTING
	#include <rawrbox/scripting/scripting.hpp>
#endif

namespace rawrbox {
	void Model::flattenMeshes(bool optimize, bool sort) {
		this->_mesh->clear();

		// Merge same meshes to reduce calls
		if (this->_canOptimize && optimize) {
			this->optimize();
		}
		// ----------------------

		// Flatten meshes for buffers
		for (auto& mesh : this->_meshes) {
			// Fix start index ----
			mesh->baseIndex = static_cast<uint16_t>(this->_mesh->indices.size());
			mesh->baseVertex = static_cast<uint16_t>(this->_mesh->vertices.size());
			// --------------------

			// Append vertices
			this->_mesh->vertices.insert(this->_mesh->vertices.end(), mesh->vertices.begin(), mesh->vertices.end());
			this->_mesh->indices.insert(this->_mesh->indices.end(), mesh->indices.begin(), mesh->indices.end());
			// -----------------
		}
		// --------

		// Sort alpha
		if (sort) {
			/*std::sort(this->_meshes.begin(), this->_meshes.end(), [](auto& a, auto& b) {
				return a->alphaBlend != b->alphaBlend;
			});*/
		}
		// --------
	}

	// ANIMATIONS ----
	void Model::animate(const rawrbox::Mesh& mesh) const {
		// VERTEX ANIMATION ----
		for (auto& anim : this->_animatedMeshes) {
			if (anim.second == nullptr) continue;
			this->readAnims(anim.second->matrix, anim.first);
		}
		// ------------

		// BONE ANIMATION ----
		if ((this->_material->supports() & rawrbox::MaterialFlags::BONES) != 0) {
			std::vector<rawrbox::Matrix4x4> boneTransforms = {};
			boneTransforms.resize(rawrbox::MAX_BONES_PER_MODEL);

			if (mesh.skeleton != nullptr) {
				auto calcs = std::unordered_map<uint8_t, rawrbox::Matrix4x4>();
				this->animateBones(calcs, *mesh.skeleton, *mesh.skeleton->rootBone, {});

				for (size_t i = 0; i < calcs.size(); i++) {
					boneTransforms[i] = calcs[static_cast<uint8_t>(i)];
				}
			}

			// this->_material->setUniformData("u_bones", boneTransforms);
		}
		// -----
	}

	void Model::animateBones(std::unordered_map<uint8_t, rawrbox::Matrix4x4>& calcs, const rawrbox::Skeleton& skeleton, const rawrbox::Bone& parentBone, const rawrbox::Matrix4x4& parentTransform) const {
		auto nodeTransform = parentBone.transformationMtx * parentBone.overrideMtx;
		this->readAnims(nodeTransform, parentBone.name);

		// store the result of our parent bone and our current node
		rawrbox::Matrix4x4 globalTransformation = parentTransform * nodeTransform;
		auto fnd = skeleton.boneMap.find(parentBone.name);
		if (fnd != skeleton.boneMap.end()) {
			calcs[fnd->second->boneId] = skeleton.invTransformationMtx * globalTransformation * fnd->second->offsetMtx;
		}

		for (auto& child : parentBone.children) {
			this->animateBones(calcs, skeleton, *child, globalTransformation);
		}
	}

	void Model::readAnims(rawrbox::Matrix4x4& nodeTransform, const std::string& nodeName) const {
		for (auto& anim : this->_playingAnimations) {
			auto animChannel = std::find_if(anim.data->frames.begin(), anim.data->frames.end(), [&](AnimationFrame& x) {
				return x.nodeName == nodeName;
			});

			if (animChannel != anim.data->frames.end()) {
				// figure out how "fast" the animation needs to play and the current playtime of the animation
				float ticksPerSecond = anim.data->ticksPerSecond != 0 ? anim.data->ticksPerSecond : 25.0F;
				float timeInTicks = anim.time * ticksPerSecond;
				timeInTicks = std::fmod(timeInTicks, anim.data->duration);

				// helper, select the next "frame" we should play depending on the time
				auto findFrameIndex = [&](auto& keys) {
					for (size_t i = 0; i + 1 < keys.size(); i++) {
						if (timeInTicks < keys[i + 1].first) {
							return i;
						}
					}

					return static_cast<size_t>(0);
				};

				// find all frames
				auto positionFrameIndex = findFrameIndex(animChannel->position);
				auto rotationFrameIndex = findFrameIndex(animChannel->rotation);
				auto scaleFrameIndex = findFrameIndex(animChannel->scale);

				auto currPos = animChannel->position[positionFrameIndex];
				auto nextPos = positionFrameIndex + 1 >= animChannel->position.size() ? animChannel->position.front() : animChannel->position[positionFrameIndex + 1];

				auto currRot = animChannel->rotation[rotationFrameIndex];
				auto nextRot = rotationFrameIndex + 1 >= animChannel->rotation.size() ? animChannel->rotation.front() : animChannel->rotation[rotationFrameIndex + 1];

				auto currScl = animChannel->scale[scaleFrameIndex];
				auto nextScl = scaleFrameIndex + 1 >= animChannel->scale.size() ? animChannel->scale.front() : animChannel->scale[scaleFrameIndex + 1];

				// Easing ----
				Vector3f position = nextPos.second;
				Vector4f rotation = nextRot.second;
				Vector3f scale = nextScl.second;

				float t = rawrbox::EasingUtils::ease(animChannel->stateEnd, timeInTicks);

				position = rawrbox::AnimUtils::lerpVector3(t, currPos, nextPos);
				rotation = rawrbox::AnimUtils::lerpRotation(t, currRot, nextRot);
				scale = rawrbox::AnimUtils::lerpVector3(t, currScl, nextScl);
				//   ----

				rawrbox::Matrix4x4 mt = {};
				mt.translate(position);

				rawrbox::Matrix4x4 ms = {};
				ms.scale(scale);

				rawrbox::Matrix4x4 mr = {};
				mr.rotate(rotation);

				nodeTransform = mt * mr * ms;
			}
		}
	}
	// -----------

	void Model::preDraw() {
		for (auto& anim : this->_playingAnimations) {
			float timeToAdd = rawrbox::DELTA_TIME * anim.speed;
			float time = anim.time + timeToAdd;
			float totalDur = anim.data->duration;

			if (!((time > totalDur || time < -totalDur) && !anim.loop)) {
				anim.time += timeToAdd;
			}
		}
	}

	void Model::postDraw() {
		for (auto it2 = this->_playingAnimations.begin(); it2 != this->_playingAnimations.end();) {
			if ((*it2).time >= (*it2).data->duration && !(*it2).loop) {
				it2 = this->_playingAnimations.erase(it2);
				continue;
			}

			++it2;
		}
	}
	// --------------

	void Model::updateLights() {
		// Update lights ---
		if ((this->_material->supports() & rawrbox::MaterialFlags::NORMALS) != 0) {
			for (auto& mesh : this->meshes()) {
				// auto p = rawrbox::MathUtils::applyRotation(meshPos + this->getPos(), this->getAngle()); // TODO

				for (auto light : mesh->lights) {
					if (light == nullptr) continue;
					light->setOffsetPos(mesh->getPos() + this->getPos());
				}
			}
		}
	}

	// BLEND SHAPES ---
	void Model::applyBlendShapes() {
		rawrbox::ModelBase::applyBlendShapes();

		this->flattenMeshes(false, false); // No need to optimize & sort, it's just vertex changes
		rawrbox::ModelBase::updateBuffers();
	}
	// --------------

#ifdef RAWRBOX_SCRIPTING
	void Model::initializeLua() {
		if (this->_luaWrapper.valid()) this->_luaWrapper.abandon();
		this->_luaWrapper = sol::make_object(rawrbox::SCRIPTING::getLUA(), rawrbox::ModelWrapper(this->shared_from_this()));
	}
#endif

	Model::~Model() {
		this->_meshes.clear();
		this->_animatedMeshes.clear();
		this->_animations.clear();
		this->_lights.clear();
	}

	void Model::setOptimizable(bool status) { this->_canOptimize = status; }
	void Model::optimize() {
#ifndef NDEBUG
		size_t old = this->_meshes.size();
#endif

		for (size_t i1 = 0; i1 < this->_meshes.size(); i1++) {
			auto& mesh1 = this->_meshes[i1];

			// figure out how big our buffers will get
			size_t reserveVertices = mesh1->vertices.size();
			size_t reserveIndices = mesh1->indices.size();

			for (size_t i2 = this->_meshes.size() - 1; i2 > i1; i2--) {
				auto& mesh2 = this->_meshes[i2];
				if (!mesh1->canOptimize(*mesh2)) continue;

				reserveVertices += mesh2->vertices.size();
				reserveIndices += mesh2->indices.size();
			}

			if (reserveVertices == mesh1->vertices.size()) continue;
			mesh1->vertices.reserve(reserveVertices);
			mesh1->indices.reserve(reserveIndices);

			// merge what it can
			for (size_t i2 = this->_meshes.size() - 1; i2 > i1; i2--) {
				auto& mesh2 = this->_meshes[i2];
				if (!mesh1->canOptimize(*mesh2)) continue;

				mesh1->merge(*mesh2);
				this->_meshes.erase(this->_meshes.begin() + i2);
			}
		}

#ifndef NDEBUG
		if (old != this->_meshes.size()) fmt::print("[RawrBox-Model] Optimized mesh for rendering (Before {} | After {})\n", old, this->_meshes.size());
#endif
	}

	void Model::updateBuffers() {
		this->flattenMeshes();
		rawrbox::ModelBase::updateBuffers();
	}

	// Animations ----
	bool Model::blendAnimation(const std::string& /*otherAnim*/, float /*blend*/) {
		throw std::runtime_error("TODO");
	}

	bool Model::playAnimation(const std::string& name, bool loop, float speed) {
		auto iter = this->_animations.find(name);
		if (iter == this->_animations.end()) throw std::runtime_error(fmt::format("[RawrBox-Model] Animation {} not found!", name));

		// Add it
		this->_playingAnimations.emplace_back(name,
		    loop,
		    speed,
		    0.0F,
		    &iter->second);

		return true;
	}

	bool Model::stopAnimation(const std::string& name) {
		for (size_t i = 0; i < this->_playingAnimations.size(); i++) {
			if (this->_playingAnimations[i].name != name) continue;
			this->_playingAnimations.erase(this->_playingAnimations.begin() + i);
			return true;
		}

		return false;
	}
	// --------------

	void Model::setPos(const rawrbox::Vector3f& pos) {
		rawrbox::ModelBase::setPos(pos);
		this->updateLights();
	}

	void Model::setAngle(const rawrbox::Vector4f& angle) {
		rawrbox::ModelBase::setAngle(angle);
		this->updateLights();
	}

	void Model::setEulerAngle(const rawrbox::Vector3f& angle) {
		rawrbox::ModelBase::setEulerAngle(angle);
		this->updateLights();
	}

	void Model::setScale(const rawrbox::Vector3f& size) {
		rawrbox::ModelBase::setScale(size);
		this->updateLights();
	}

	const rawrbox::BBOX& Model::getBBOX() const { return this->_bbox; }
	size_t Model::totalMeshes() const {
		return this->_meshes.size();
	}

	bool Model::empty() const {
		return this->_meshes.empty();
	}

	void Model::removeMeshByName(const std::string& id) {
		for (auto it2 = this->_meshes.begin(); it2 != this->_meshes.end();) {
			if ((*it2)->getName() == id) {
				it2 = this->_meshes.erase(it2);
				continue;
			}

			++it2;
		}

		if (this->isUploaded() && this->isDynamic()) this->updateBuffers(); // Already uploaded? And dynamic? Then update vertices
	}

	void Model::removeMesh(size_t index) {
		if (index >= this->_meshes.size()) return;
		this->_meshes.erase(this->_meshes.begin() + index);

		if (this->isUploaded() && this->isDynamic()) this->updateBuffers(); // Already uploaded? And dynamic? Then update vertices
	}

	rawrbox::Mesh* Model::addMesh(rawrbox::Mesh mesh) {
		this->_bbox.combine(mesh.getBBOX());
		mesh.owner = this;

		auto& a = this->_meshes.emplace_back(std::make_unique<rawrbox::Mesh>(mesh));
		if (this->isUploaded() && this->isDynamic()) this->updateBuffers(); // Already uploaded? And dynamic? Then update vertices

		return a.get();
	}

	rawrbox::Mesh* Model::getMeshByName(const std::string& id) {
		auto fnd = std::find_if(this->_meshes.begin(), this->_meshes.end(), [&id](auto& mesh) { return mesh->getName() == id; });
		if (fnd == this->_meshes.end()) return nullptr;

		return (*fnd).get();
	}

	rawrbox::Mesh* Model::getMesh(size_t id) {
		if (!this->hasMesh(id)) return nullptr;
		return this->_meshes[id].get();
	}

	bool Model::hasMesh(size_t id) {
		return id >= 0 && id < this->_meshes.size();
	}

	void Model::setCulling(Diligent::CULL_MODE cull, int id) {
		for (size_t i = 0; i < this->_meshes.size(); i++) {
			if (id != -1 && i != static_cast<size_t>(id)) continue;
			this->_meshes[i]->setCulling(cull);
		}
	}

	void Model::setWireframe(bool wireframe, int id) {
		for (size_t i = 0; i < this->_meshes.size(); i++) {
			if (id != -1 && i != static_cast<size_t>(id)) continue;
			this->_meshes[i]->setWireframe(wireframe);
		}
	}

	void Model::setColor(const rawrbox::Color& color, int id) {
		for (size_t i = 0; i < this->_meshes.size(); i++) {
			if (id != -1 && i != static_cast<size_t>(id)) continue;
			this->_meshes[i]->setColor(color);
		}
	}

	void Model::setRecieveDecals(bool recieve, int id) {
		for (size_t i = 0; i < this->_meshes.size(); i++) {
			if (id != -1 && i != static_cast<size_t>(id)) continue;
			this->_meshes[i]->setRecieveDecals(recieve);
		}
	}

	void Model::setTexture(rawrbox::TextureBase* tex, int id) {
		for (size_t i = 0; i < this->_meshes.size(); i++) {
			if (id != -1 && i != static_cast<size_t>(id)) continue;
			this->_meshes[i]->setTexture(tex);
		}
	}

	std::vector<std::unique_ptr<rawrbox::Mesh>>& Model::meshes() {
		return this->_meshes;
	}

	void Model::upload(bool dynamic) {
		this->flattenMeshes(); // Merge and optimize meshes for drawing
		ModelBase::upload(dynamic);
	}

	void Model::draw() {
		ModelBase::draw();
		this->preDraw();

		auto context = rawrbox::RENDERER->context();
		for (auto& mesh : this->_meshes) {
			// Process animations ---
			this->animate(*mesh);
			// ---

			// Bind materials uniforms & textures ----
			rawrbox::TRANSFORM = this->getMatrix() * mesh->getMatrix();
			this->_material->bind(*mesh);
			// -----------

			Diligent::DrawIndexedAttribs DrawAttrs;    // This is an indexed draw call
			DrawAttrs.IndexType = Diligent::VT_UINT16; // Index type
			DrawAttrs.FirstIndexLocation = mesh->baseIndex;
			DrawAttrs.BaseVertex = mesh->baseVertex;
			DrawAttrs.NumIndices = mesh->totalIndex;
			DrawAttrs.Flags = Diligent::DRAW_FLAG_VERIFY_ALL; // Verify the state of vertex and index buffers
			context->DrawIndexed(DrawAttrs);
		}

		this->postDraw();
	}
} // namespace rawrbox
