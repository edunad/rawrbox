
#pragma once
#include <rawrbox/math/vector2.h>
#include <fmt/printf.h>

extern "C"
{
	#include <libqhull_r/libqhull_r.h>
}

#include <string>

namespace rawrBox {
	class QHull {
		public:
		/*
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
		*/
		static std::vector<rawrBox::Vector2> calculateConvex(const std::vector<rawrBox::Vector2>& vertPoints, const std::string& flags = "qhull FA") {
			std::vector<rawrBox::Vector2> hull = {};

			// Setup QHULL ---
			int dimension = 2; // Set as 2D
			size_t size = vertPoints.size();

			coordT* points = reinterpret_cast<coordT*> (calloc (size * dimension, sizeof(coordT)));
			for (size_t i = 0; i < size; ++i) {
				points[2 * i] = static_cast<coordT>(vertPoints[i].x);
				points[2 * i + 1] = static_cast<coordT>(vertPoints[i].y);
			}

			// Prepare lib
			FILE *outfile = nullptr;
			FILE *errfile = stderr;

			qhT qh_qh;
			qhT* qh = &qh_qh;
			QHULL_LIB_CHECK
			qh_zero(qh, errfile);

			int exitcode = qh_new_qhull(qh, dimension, static_cast<int>(size), points, true, flags.c_str(), outfile, errfile);
			if (exitcode != 0 || qh->num_vertices == 0) {
				fmt::print("[QHull] Unable to compute a convex hull for the given points\n");

				qh_freeqhull (qh, !qh_ALL);
				int curlong, totlong;
				qh_memfreeshort (qh, &curlong, &totlong);

				return hull;
			}

			vertexT * vertex;
			FORALLvertices
			{
				hull.push_back(vertPoints[qh_pointid(qh, vertex->point)]);
			}

			qh_freeqhull (qh, !qh_ALL);
			int curlong, totlong;
			qh_memfreeshort (qh, &curlong, &totlong);

			return hull;
		}
	}
}
