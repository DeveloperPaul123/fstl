#ifndef MESH_UTIL_H
#define MESH_UTIL_H

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>

#include <QObject>
#include <QString>
#include <QFile>
#include <QThread>
#include <QTextStream>
#include <QDataStream>

#include <QtOpenGL/QGLBuffer>
#include <QtOpenGL/QGLFunctions>

namespace MeshUtil {
	template <class T>
	void reorder(std::vector<T> &v, std::vector<size_t> const &order)  {
		for (int s = 1, d; s < order.size(); ++s) {
			for (d = order[s]; d < s; d = order[d]);
			if (d == s) while (d = order[d], d != s) std::swap(v[s], v[d]);
		}

	}
	struct MeshParams {
		float xOffset;
		float yOffset;
		float zOffset;
		MeshParams() {
			xOffset = 0.0;
			yOffset = 0.0;
			zOffset = 0.0;
		}
	};
	struct Point {

		float x;
		float y;
		float z;
		
		Point() {
			x = 0.0;
			y = 0.0;
			z = 0.0;
		}

		Point(float x1, float y1, float z1) {
			x = x1;
			y = y1;
			z = z1;
		}
		
		float distanceTo(Point p) {
			float xDif = p.x - x;
			float yDif = p.y - y;
			float zDif = p.z - z;
			return sqrtf(pow(xDif, 2.0) + pow(yDif, 2.0) + pow(zDif, 2.0));
		}
	};

	struct Triangle {
		int x;
		int y;
		int z;
		int _oldX, _oldY, _oldZ;

		Triangle() {
			x = 0;
			y = 0;
			z = 0;
			_oldX = 0;
			_oldY = 0;
			_oldZ = 0;
		}
		Triangle(int x1, int y1, int z1) {
			x = x1;
			y = y1;
			z = z1;
		}

		/**
		* Get a sorted copy of the current triangle. 
		*/
		void sort() {
			_oldX = x; _oldY = y; _oldZ = z;
			std::vector<int> vals;
			vals.push_back(x);
			vals.push_back(y);
			vals.push_back(z);
			std::sort(vals.begin(), vals.end());
			x = vals[0];
			y = vals[1];
			z = vals[2];
		}

		void unsort() {
			x = _oldX; y = _oldY; z = _oldZ;
		}

		/**
		* Comparison operator for sorting. Sort by x, then y, then z
		* @param other other Triangle to compare to.
		* @return true if this Triangle is less than the other.
		*/
		bool operator<(const Triangle &other) const {
			if (x == other.x) {
				if (y != other.y) {
					return y < other.y;
				}
				else {
					if (z != other.z) {
						return z < other.z;
					}
					else {
						return false;
					}
				}
			}
			else {
				return x < other.x;
			}
		}

		/**
		* Comparison operator for unique algorithm. Compares across x, y, z
		* @param other the Triangle to compare against.
		* @return true if all values are equal, false otherwise.
		*/
		bool operator==(const Triangle &other) const {
			return x == other.x && y == other.y && z == other.z;
		}
	};

	struct Tetrahedron {
		Tetrahedron() {
			x = 0; 
			y = 0;
			z = 0;
			h = 0;
		}
		Tetrahedron(int x1, int y1, int z1, int h1) {
			x = x1;
			y = y1;
			z = z1;
			h = h1;
		}
		int x;
		int y;
		int z;
		int h;

		/**
		* Comparison operator for sorting. Sort by x, then y, then z then h.
		* @param other other tetrahedron to compare to. 
		* @return true if this tetrahedron is less than the other. 
		*/
		bool operator<(const Tetrahedron &other) const {
			if (x == other.x) {
				if (y != other.y) {
					return y < other.y;
				}
				else {
					if (z != other.z) {
						return z < other.z;
					}
					else {
						if (h != other.h) {
							return h < other.h;
						}
						else {
							return false;
						}
					}
				}
			}
			else {
				return x < other.x;
			}
		}

		/**
		* Comparison operator for unique algorithm. Compares across x, y, z and h.
		* @param other the Tetrahedron to compare against.
		* @return true if all values are equal, false otherwise. 
		*/
		bool operator==(const Tetrahedron &other) const {
			return x == other.x && y == other.y && z == other.z && h == other.h;
		}
	};

	class Mesh {
	public:
		Mesh(std::vector<Point> verts, std::vector<Triangle> ts, std::vector<Tetrahedron> tts);
		Mesh(int pointSize, int triangleSize, int tetraSize);
		Point getPoint(int index);
		void setPoint(Point newP, int index);
		Triangle getTriangle(int index);
		void setTriangle(Triangle newT, int index);
		Tetrahedron getTetrahedron(int index);
		void setTetrahedron(Tetrahedron tetra, int index);
		std::vector<Triangle> getSurfaceFaces();

	private:
		std::vector<Point> verts;
		std::vector<Triangle> tris;
		std::vector<Tetrahedron> tetras;
	};

	class GLMesh : protected QGLFunctions
	{
	public:
		GLMesh(Mesh* mesh);
		void draw(GLuint vp);
		float min(size_t start) const;
		float max(size_t start) const;

		float xmin() const { return min(0); }
		float ymin() const { return min(1); }
		float zmin() const { return min(2); }
		float xmax() const { return max(0); }
		float ymax() const { return max(1); }
		float zmax() const { return max(2); }
	private:
		std::vector<GLfloat> mVerts;
		std::vector<GLuint> mInd;
	};

	class MeshLoader : public QThread{
		Q_OBJECT

	public: 
		explicit MeshLoader(QObject *parent, QString filename, MeshParams params);
		void run();

	signals:
		void meshLoaded(Mesh *mesh);
		void meshLoadFailed();
	protected:
		Mesh* readMeshFromFile();
	private:
		QString filename;
		MeshParams params;
	};
}
#endif