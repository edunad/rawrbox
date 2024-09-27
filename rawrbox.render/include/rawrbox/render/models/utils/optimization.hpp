#pragma once

#include <rawrbox/render/models/vertex.hpp>

#include <meshoptimizer.h>

namespace rawrbox {

	class MeshOptimization {
	public:
		template <typename T = rawrbox::VertexData>
			requires(std::derived_from<T, rawrbox::VertexData>)
		static void optimize(std::vector<T>& verts, std::vector<uint32_t>& indices, float threshold = 1.05F) {
			size_t indiceSize = indices.size();
			std::vector<uint32_t> remap(indiceSize);

			// Calculate vertex map ---
			size_t newVerticeCount = meshopt_generateVertexRemap(&remap[0], &indices[0], indiceSize, &verts[0], indiceSize, sizeof(T));
			// ------

			// Optimize index & vertex buffers ---
			std::vector<uint32_t> resultIndices(indiceSize);
			std::vector<T> resultVertices(newVerticeCount);

			meshopt_remapIndexBuffer(&resultIndices[0], &indices[0], indiceSize, &remap[0]);
			meshopt_remapVertexBuffer(&resultVertices[0], &verts[0], indiceSize, sizeof(T), &remap[0]);
			// -------------

			// Vertex cache
			meshopt_optimizeVertexCache(&resultIndices[0], &resultIndices[0], indiceSize, newVerticeCount);

			// Overdraw
			meshopt_optimizeOverdraw(&resultIndices[0], &resultIndices[0], indiceSize, &resultVertices[0].position.x, newVerticeCount, sizeof(T), threshold);

			// Vertex fetch
			meshopt_optimizeVertexFetch(&resultVertices[0], &resultIndices[0], indiceSize, &resultVertices[0], newVerticeCount, sizeof(T));

			verts.swap(resultVertices);
			indices.swap(resultIndices);
		}

		template <typename T = rawrbox::VertexData>
			requires(std::derived_from<T, rawrbox::VertexData>)
		static void simplify(std::vector<T>& verts, std::vector<uint32_t>& indices, float complexity_threshold = 0.9F) {
			const size_t index_count = indices.size();
			const size_t vertex_count = verts.size();

			const size_t target_index_count = index_count * complexity_threshold;
			constexpr float target_error = 1e-2f;
			constexpr unsigned int options = 0;

			std::vector<uint32_t> lod_indices(index_count);
			float lod_error = 0.0f;

			lod_indices.resize(meshopt_simplify(lod_indices.data(), indices.data(), index_count,
			    reinterpret_cast<const float*>(verts.data()),
			    vertex_count, sizeof(T), target_index_count,
			    target_error, options, &lod_error));

			indices.swap(lod_indices);
		}
	};
} // namespace rawrbox
