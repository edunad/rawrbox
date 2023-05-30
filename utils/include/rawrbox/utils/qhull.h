
// NOLINTBEGIN(*)
#pragma once
/*
extern "C" {
#include <libqhull/libqhull.h>
}

#include <string>
#include <vector>

namespace rawrbox {
	struct QHullPoint {
		float x = 0.F;
		float y = 0.F;

		QHullPoint() = default;
	};

	class QHull {
	public:

			My god, this lib's documentation is a mess
			⠄⠄⠄⠄⠄⠄⠄⠄⠄⠄⠄⠄⠄⠄⠄⠄⣀⠄⣀⠄⠄⣀⣀⠄⠄⠄⠄⠄⠄⠄⢀⣀⣀⣀⣀⣀⣀⣀
			⠄⠄⠄⠄⠄⠄⠄⠄⠄⠄⠄⠄⣀⣴⣲⠟⠛⠛⠃⣀⠉⠁⠈⠙⢶⣤⣀⠄⠄⠄⣸⣿⣿⣿⣿⣿⣿⣿
			⠄⠄⠄⠄⠄⠄⠄⠄⠄⢀⣴⡿⡹⣕⠁⠄⣠⣾⣿⣿⣿⣿⣷⣦⣄⠽⣿⣷⡄⠄⢹⣿⣿⣿⣿⣿⣿⣿
			⠄⠄⠄⠄⠄⠄⠄⠄⣠⣿⣾⣴⣷⣿⡇⢠⣿⣿⣿⣿⣿⣿⣿⣿⣿⣷⡙⣿⣿⣆⣼⣿⣿⣿⣿⣿⣿⣿
			⠄⠄⠄⠄⠄⠄⠄⣴⣿⣿⢻⣿⣿⢹⠇⣿⣟⣯⣭⣍⡉⠹⣿⣿⣿⢿⠳⠜⢿⣿⣿⣿⣿⣿⣿⣿⣿⣿
			⠄⠄⠄⠄⠄⠄⢰⣿⣿⣿⣿⣿⡽⡾⢸⣿⠿⠃⠄⢤⡪⣠⣿⣿⣿⠄⠙⠛⠈⣿⣿⣿⣿⣿⣿⣿⣿⣿
			⠄⠄⠄⠄⠄⠄⣼⣿⣿⣿⣿⣿⣵⠇⣼⣿⣿⣿⣿⣿⣿⣿⣿⣿⣷⣮⣬⣇⢠⢾⣿⣿⣿⣿⣿⣿⣿⣿
			⠄⠄⠄⠄⠄⢰⣿⣿⣿⣿⣿⣿⡟⠄⣿⣿⣿⡿⣿⣿⠏⠟⠛⠛⠿⢻⣿⣿⣧⠈⣿⣿⣿⣿⣿⣿⣿⣿
			⠄⠄⠄⠄⠄⣿⣿⣿⡵⣿⣿⣿⠄⠄⣿⣿⣿⣿⡟⢁⣾⣶⣾⣾⣶⣷⠈⣿⡿⠄⣼⣿⣿⣿⣿⣿⣿⣿
			⠄⠄⠄⠄⣸⣿⣿⣿⣯⣿⣿⡟⠄⠠⣿⣿⣿⣿⣇⠏⠁⠄⠄⠄⠉⢻⢀⣿⢳⣶⣽⡿⣿⣿⣿⣿⣿⣿
			⠄⠄⠄⣰⣿⣿⣿⣿⣿⣿⣿⡇⠄⠄⢻⣿⣿⣿⣽⣿⠛⠉⠛⠳⠒⣾⣿⡟⢸⣿⣿⠇⢹⣿⣿⣿⣿⣿
			⠄⠄⣠⣿⣿⣿⣿⣿⣿⣿⣿⠄⠄⠄⠄⣿⣿⡟⢿⢿⠿⠿⢿⣷⣾⣿⡟⠄⢈⢧⣽⣷⣼⣿⣿⣿⣿⣿
			⠄⢀⣿⣿⣿⣿⣿⢻⣿⣿⣟⠄⠄⠄⠄⠘⣿⣷⣤⡀⠄⠄⠄⢈⡁⠄⠄⠄⠄⢻⣯⣿⣿⣿⣿⣿⣿⣿
			⠄⢸⣿⣿⣿⣿⠈⣼⣿⣿⣿⡄⠄⠄⠄⠄⠹⣟⠉⠁⠄⠄⢾⣿⡏⡀⠄⠄⣀⠈⡿⣿⣿⣿⣿⣿⣿⣿
			⢀⣹⣿⣿⣿⣿⣽⣿⣿⣿⣿⣿⡀⠄⠄⡀⠄⡇⠄⢀⣀⣀⣸⣯⢠⡇⠄⠄⢼⡆⢡⣿⣿⣿⣿⣿⣿⣿
			⣸⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣷⠄⠄⠨⢾⣿⣿⣿⣿⣿⣿⣿⠄⣷⡀⠄⢸⡇⢘⣻⣿⣿⣿⣿⣿⣿

		static std::vector<QHullPoint> calculateConvex(const std::vector<QHullPoint>& vertPoints, const std::string& flags = "qhull FA") {
			std::vector<QHullPoint> hull = {};

			// Setup QHULL ---
			int dimension = 2; // Set as 2D
			size_t size = vertPoints.size();

			coordT* points = reinterpret_cast<coordT*>(calloc(size * dimension, sizeof(coordT)));
			for (size_t i = 0; i < size; ++i) {
				points[2 * i] = static_cast<coordT>(vertPoints[i].x);
				points[2 * i + 1] = static_cast<coordT>(vertPoints[i].y);
			}

			// Prepare lib
			FILE* outfile = nullptr;
			FILE* errfile = stderr;

			qhT qh_qh;
			qhT* qh = &qh_qh;
			QHULL_LIB_CHECK
			qh_zero(qh, errfile);

			char* a = const_cast<char*>(flags.c_str());
			int exitcode = qh_new_qhull(qh, dimension, static_cast<int>(size), points, true, a, outfile, errfile);
			if (exitcode != 0 || qh->num_vertices == 0) {
				qh_freeqhull(qh, !qh_ALL);
				int curlong = 0, totlong = 0;
				qh_memfreeshort(qh, &curlong, &totlong);

				return hull;
			}

			vertexT* vertex = nullptr;
			FORALLvertices {
				hull.push_back(vertPoints[qh_pointid(qh, vertex->point)]);
			}

			qh_freeqhull(qh, !qh_ALL);
			int curlong = 0, totlong = 0;
			qh_memfreeshort(qh, &curlong, &totlong);

			return hull;
		}
	};
} // namespace rawrbox
  // NOLINTEND(*)
*/
