
#include <rawrbox/render/static.hpp>
#include <rawrbox/render/utils/debug_draw.hpp>
#include <rawrbox/render/utils/pipeline.hpp>

#include <bit>

namespace rawrbox {
	namespace {
		class DebugBatch : public rawrbox::ModelBase<rawrbox::MaterialUnlit> {
		public:
			void rebuild(const std::vector<rawrbox::Vector3f>& points, const rawrbox::Colorf& color) {
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

			void drawBatch(Diligent::IPipelineState* pipe) {
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
		};

		std::vector<std::unique_ptr<DebugBatch>>& batchPool() {
			static std::vector<std::unique_ptr<DebugBatch>> pool = {};
			return pool;
		}

		std::array<Diligent::IPipelineState*, 4>& pipeCache() {
			static std::array<Diligent::IPipelineState*, 4> cache = {};
			return cache;
		}

		Diligent::IPipelineState* overlayPipeline(bool line, bool onTop) {
			const size_t idx = (onTop ? 2U : 0U) | (line ? 1U : 0U);

			auto& cache = pipeCache();
			if (cache[idx] == nullptr) {
				const std::string name = std::string("Model::Unlit::") + (onTop ? "NoDepth" : "Overlay") + (line ? "::Line" : "");
				cache[idx] = rawrbox::PipelineUtils::getPipeline(name);
			}

			return cache[idx];
		}

		// Slots returned by evicted buckets. The DebugBatch object (and its GPU buffers) stays
		// alive for reuse — only the slot is recycled.
		std::vector<size_t>& freeSlots() {
			static std::vector<size_t> slots = {};
			return slots;
		}

		size_t acquireSlot() {
			auto& free = freeSlots();
			if (!free.empty()) {
				const size_t slot = free.back();
				free.pop_back();
				return slot;
			}

			batchPool().push_back(std::make_unique<DebugBatch>());
			return batchPool().size() - 1;
		}

		void hashBits(uint64_t& hash, uint64_t value) {
			static constexpr uint64_t FNV_PRIME = 1099511628211ULL;
			hash ^= value;
			hash *= FNV_PRIME;
		}

		void hashFloat(uint64_t& hash, float value) {
			hashBits(hash, std::bit_cast<uint32_t>(value));
		}

		uint64_t hashPoints(const std::vector<rawrbox::Vector3f>& points) {
			static constexpr uint64_t FNV_OFFSET = 14695981039346656037ULL;
			uint64_t hash = FNV_OFFSET;

			for (const auto& point : points) {
				hashFloat(hash, point.x);
				hashFloat(hash, point.y);
				hashFloat(hash, point.z);
			}

			return hash == 0 ? 1 : hash;
		}
	} // namespace

	// STATIC DATA ----
	std::map<DebugDraw::BucketKey, DebugDraw::Bucket> DebugDraw::_buckets = {};

	DebugDraw::BucketKey DebugDraw::_lastKey = {};
	DebugDraw::Bucket* DebugDraw::_lastBucket = nullptr;

	uint64_t DebugDraw::_frame = 0;
	DebugDraw::Stats DebugDraw::_stats = {};
	// ----------------

	const DebugDraw::Stats& DebugDraw::stats() { return _stats; }

	void DebugDraw::shutdown() {
		batchPool().clear();
		freeSlots().clear();

		pipeCache().fill(nullptr);
		_buckets.clear();

		_lastBucket = nullptr;
		_lastKey = {};

		_frame = 0;
	}

	DebugDraw::Bucket& DebugDraw::bucket(const rawrbox::Colorf& color, bool line, bool onTop) {
		BucketKey key = {};

		key.rgba = color.pack();
		key.line = line;
		key.onTop = onTop;

		if (_lastBucket != nullptr && _lastKey == key) return *_lastBucket;

		auto& bucket = _buckets[key];
		bucket.color = color;

		_lastKey = key;
		_lastBucket = &bucket;

		return bucket;
	}

	void DebugDraw::line(const rawrbox::Vector3f& a, const rawrbox::Vector3f& b, const rawrbox::Colorf& color, bool onTop) {
		auto& b0 = bucket(color, true, onTop);
		b0.points.push_back(a);
		b0.points.push_back(b);
	}

	void DebugDraw::triangle(const rawrbox::Vector3f& a, const rawrbox::Vector3f& b, const rawrbox::Vector3f& c, const rawrbox::Colorf& color, bool onTop) {
		auto& b0 = bucket(color, false, onTop);
		b0.points.push_back(a);
		b0.points.push_back(b);
		b0.points.push_back(c);
	}

	void DebugDraw::quad(const rawrbox::Vector3f& a, const rawrbox::Vector3f& b, const rawrbox::Vector3f& c, const rawrbox::Vector3f& d, const rawrbox::Colorf& color, bool onTop) {
		triangle(a, b, c, color, onTop);
		triangle(a, c, d, color, onTop);
	}

	void DebugDraw::aabb(const rawrbox::Vector3f& min, const rawrbox::Vector3f& max, const rawrbox::Colorf& color, bool onTop) {
		const rawrbox::Vector3f corners[8] = {
		    {min.x, min.y, min.z}, {max.x, min.y, min.z}, {max.x, min.y, max.z}, {min.x, min.y, max.z},
		    {min.x, max.y, min.z}, {max.x, max.y, min.z}, {max.x, max.y, max.z}, {min.x, max.y, max.z}};

		constexpr std::array<std::pair<int, int>, 12> EDGES = {{{0, 1}, {1, 2}, {2, 3}, {3, 0}, {4, 5}, {5, 6}, {6, 7}, {7, 4}, {0, 4}, {1, 5}, {2, 6}, {3, 7}}};
		for (const auto& [i, j] : EDGES) {
			line(corners[i], corners[j], color, onTop);
		}
	}

	void DebugDraw::drawBuckets() {
		auto& pool = batchPool();
		_frame++;

		_stats = {};
		_lastBucket = nullptr;

		for (auto it = _buckets.begin(); it != _buckets.end();) {
			auto& key = it->first;
			auto& bucket = it->second;

			// CLEANUP -----
			if (bucket.points.empty()) {
				if (bucket.batch != NO_BATCH && _frame - bucket.lastFrame > IDLE_REMOVE_FRAMES) {
					freeSlots().push_back(bucket.batch);
					it = _buckets.erase(it);
					continue;
				}

				++it;
				continue;
			}
			// --------------

			bucket.lastFrame = _frame;
			if (bucket.batch == NO_BATCH) bucket.batch = acquireSlot();

			auto& batch = *pool[bucket.batch];
			const uint64_t hash = hashPoints(bucket.points);
			if (hash != bucket.hash) {
				batch.rebuild(bucket.points, bucket.color);
				bucket.hash = hash;

				_stats.uploads++;
			}

			batch.drawBatch(overlayPipeline(key.line, key.onTop));

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
