
#include <rawrbox/render/static.hpp>
#include <rawrbox/render/utils/debug_draw.hpp>
#include <rawrbox/render/utils/pipeline.hpp>

#include <bit>

namespace rawrbox {
	// DEBUG BATCH ----
	void DebugBatch::rebuild(const std::vector<rawrbox::Vector3f>& points, const rawrbox::Colorf& color) {
		auto& verts = this->_mesh->vertices;
		auto& inds = this->_mesh->indices;

		verts.clear();
		verts.reserve(points.size());

		inds.clear();
		inds.reserve(points.size());

		for (size_t i = 0; i < points.size(); i++) {
			verts.emplace_back(points[i], rawrbox::Vector2f(0.5F, 0.5F));
			inds.push_back(static_cast<uint32_t>(i));
		}

		this->_mesh->setColor(color);
		this->_mesh->setTexture(rawrbox::WHITE_TEXTURE.get());

		if (!this->isUploaded()) {
			this->upload(rawrbox::UploadType::RESIZABLE_DYNAMIC);
		} else {
			this->updateBuffers();
		}
	}

	void DebugBatch::drawBatch(Diligent::IPipelineState* pipe) {
		if (this->_mesh->indices.empty() || pipe == nullptr) return;

		rawrbox::ModelBase<rawrbox::MaterialUnlit>::draw();

		auto* context = rawrbox::RENDERER->context();
		auto& mesh = *this->_mesh;

		context->SetPipelineState(pipe);

		this->_material->bindVertexUniforms(mesh);
		this->_material->bindVertexSkinnedUniforms(mesh);
		this->_material->bindPixelUniforms(mesh);

		rawrbox::MAIN_CAMERA->setModelTransform(mesh.getMatrix());

		Diligent::DrawIndexedAttribs attrs;
		attrs.IndexType = Diligent::VT_UINT32;
		attrs.FirstIndexLocation = 0;
		attrs.BaseVertex = 0;
		attrs.NumIndices = static_cast<uint32_t>(mesh.indices.size());
		attrs.Flags = Diligent::DRAW_FLAG_VERIFY_ALL;

		context->DrawIndexed(attrs);
	}
	// ----------------

	// STATIC DATA ----
	std::map<DebugDraw::BucketKey, DebugDraw::Bucket> DebugDraw::_buckets = {};

	DebugDraw::BucketKey DebugDraw::_lastKey = {};
	DebugDraw::Bucket* DebugDraw::_lastBucket = nullptr;

	uint64_t DebugDraw::_frame = 0;
	DebugDraw::Stats DebugDraw::_stats = {};

	std::vector<std::unique_ptr<rawrbox::DebugBatch>> DebugDraw::_pool = {};
	std::vector<size_t> DebugDraw::_free = {};
	std::array<Diligent::IPipelineState*, 4> DebugDraw::_pipeCache = {};
	// ----------------

	// PRIVATE ----
	Diligent::IPipelineState* DebugDraw::overlayPipeline(bool line, bool depthTest) {
		const size_t idx = (depthTest ? 0U : 2U) | (line ? 1U : 0U);

		if (_pipeCache[idx] == nullptr) {
			const std::string name = std::string("Model::Unlit::") + (depthTest ? "Overlay" : "NoDepth") + (line ? "::Line" : "");
			_pipeCache[idx] = rawrbox::PipelineUtils::getPipeline(name);
		}

		return _pipeCache[idx];
	}

	size_t DebugDraw::acquire() {
		if (!_free.empty()) {
			const size_t slot = _free.back();
			_free.pop_back();
			return slot;
		}

		_pool.push_back(std::make_unique<rawrbox::DebugBatch>());
		return _pool.size() - 1;
	}

	void DebugDraw::hashBits(uint64_t& hash, uint64_t value) {
		static constexpr uint64_t FNV_PRIME = 1099511628211ULL;
		hash ^= value;
		hash *= FNV_PRIME;
	}

	void DebugDraw::hashFloat(uint64_t& hash, float value) {
		hashBits(hash, std::bit_cast<uint32_t>(value));
	}

	uint64_t DebugDraw::hashPoints(const std::vector<rawrbox::Vector3f>& points) {
		static constexpr uint64_t FNV_OFFSET = 14695981039346656037ULL;
		uint64_t hash = FNV_OFFSET;

		for (const auto& point : points) {
			hashFloat(hash, point.x);
			hashFloat(hash, point.y);
			hashFloat(hash, point.z);
		}

		return hash == 0 ? 1 : hash;
	}
	//------------

	const DebugDraw::Stats& DebugDraw::stats() { return _stats; }

	void DebugDraw::shutdown() {
		_pool.clear();
		_free.clear();

		_pipeCache.fill(nullptr);
		_buckets.clear();

		_lastBucket = nullptr;
		_lastKey = {};

		_frame = 0;
	}

	DebugDraw::Bucket& DebugDraw::bucket(const rawrbox::Colorf& color, bool line, bool depthTest) {
		BucketKey key = {};

		key.rgba = color.pack();
		key.line = line;
		key.depthTest = depthTest;

		if (_lastBucket != nullptr && _lastKey == key) return *_lastBucket;

		auto& bucket = _buckets[key];
		bucket.color = color;

		_lastKey = key;
		_lastBucket = &bucket;

		return bucket;
	}

	void DebugDraw::line(const rawrbox::Vector3f& a, const rawrbox::Vector3f& b, const rawrbox::Colorf& color, bool depthTest) {
		auto& b0 = bucket(color, true, depthTest);
		b0.points.push_back(a);
		b0.points.push_back(b);
	}

	void DebugDraw::triangle(const rawrbox::Vector3f& a, const rawrbox::Vector3f& b, const rawrbox::Vector3f& c, const rawrbox::Colorf& color, bool depthTest) {
		auto& b0 = bucket(color, false, depthTest);

		b0.points.push_back(a);
		b0.points.push_back(b);
		b0.points.push_back(c);
	}

	void DebugDraw::quad(const rawrbox::Vector3f& a, const rawrbox::Vector3f& b, const rawrbox::Vector3f& c, const rawrbox::Vector3f& d, const rawrbox::Colorf& color, bool depthTest) {
		triangle(a, b, c, color, depthTest);
		triangle(a, c, d, color, depthTest);
	}

	void DebugDraw::aabb(const rawrbox::Vector3f& min, const rawrbox::Vector3f& max, const rawrbox::Colorf& color, bool depthTest) {
		const rawrbox::Vector3f corners[8] = {
		    {min.x, min.y, min.z}, {max.x, min.y, min.z}, {max.x, min.y, max.z}, {min.x, min.y, max.z},
		    {min.x, max.y, min.z}, {max.x, max.y, min.z}, {max.x, max.y, max.z}, {min.x, max.y, max.z}};

		constexpr std::array<std::pair<int, int>, 12> EDGES = {{{0, 1}, {1, 2}, {2, 3}, {3, 0}, {4, 5}, {5, 6}, {6, 7}, {7, 4}, {0, 4}, {1, 5}, {2, 6}, {3, 7}}};
		for (const auto& [i, j] : EDGES) {
			line(corners[i], corners[j], color, depthTest);
		}
	}

	void DebugDraw::bbox(const rawrbox::Vector3f& pos, const rawrbox::BBOXf& box, const rawrbox::Colorf& color, bool depthTest) {
		aabb(pos + box.min, pos + box.max, color, depthTest);
	}

	void DebugDraw::drawBuckets() {
		_frame++;

		_stats = {};
		_lastBucket = nullptr;

		for (auto it = _buckets.begin(); it != _buckets.end();) {
			auto& key = it->first;
			auto& bucket = it->second;

			// CLEANUP -----
			if (bucket.points.empty()) {
				if (bucket.batch != NO_BATCH && _frame - bucket.lastFrame > IDLE_REMOVE_FRAMES) {
					_free.push_back(bucket.batch);
					it = _buckets.erase(it);
					continue;
				}

				++it;
				continue;
			}
			// --------------

			bucket.lastFrame = _frame;
			if (bucket.batch == NO_BATCH) bucket.batch = acquire();

			auto& batch = *_pool[bucket.batch];
			const uint64_t hash = hashPoints(bucket.points);
			if (hash != bucket.hash) {
				batch.rebuild(bucket.points, bucket.color);
				bucket.hash = hash;

				_stats.uploads++;
			}

			batch.drawBatch(overlayPipeline(key.line, key.depthTest));

			_stats.buckets++;
			_stats.points += bucket.points.size();
			_stats.drawCalls++;

			bucket.points.clear();
			++it;
		}
	}

	void DebugDraw::draw() {
		DebugDraw::drawBuckets();
	}
} // namespace rawrbox
