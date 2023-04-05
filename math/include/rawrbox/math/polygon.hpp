#pragma once

#include <rawrbox/math/vector2.hpp>
#include <rawrbox/math/vector3.hpp>

#include <vector>

namespace rawrBox {
	template <class VerticeType>
	class Polygon_t {
	private:
		std::vector<VerticeType> verts;
	public:
		/* 2D isInside poly, z is ignored when using Vector3 */
		template<class T>
		static bool isInside(const T& p, const std::vector<T>& poly) {
			if (poly.size() < 3) return false;

			size_t i, j, c = 0;
			for (i = 0, j = poly.size() - 1; i < poly.size(); j = i++) {
				if (((poly[i].y > p.y) != (poly[j].y > p.y)) && (p.x < (poly[j].x - poly[i].x) * (p.y - poly[i].y) / (poly[j].y - poly[i].y) + poly[i].x)) {
					c = !c;
				}
			}

			return c != 0;
		}
	};

	using Polygon3d = Polygon_t<Vector3d>;
	using Polygon3f = Polygon_t<Vector3f>;
	using Polygon3i = Polygon_t<Vector3i>;

	using Polygon2d = Polygon_t<Vector2d>;
	using Polygon2f = Polygon_t<Vector2f>;
	using Polygon2i = Polygon_t<Vector2i>;

	using Polygon = Polygon2f;
}
