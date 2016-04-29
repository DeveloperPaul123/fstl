#ifndef MESH_UTIL_CORE
#define MESH_UTIL_CORE

#include <vector>
#include <algorithm>

namespace MeshUtil {
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

		Point operator-(const Point &other) {
			return Point(x - other.x, y - other.y, z - other.z);
		}

		float distanceTo(Point p) {
			float xDif = p.x - x;
			float yDif = p.y - y;
			float zDif = p.z - z;
			return sqrtf(pow(xDif, 2.0) + pow(yDif, 2.0) + pow(zDif, 2.0));
		}
	};

	struct Vector3F {
		Point st;
		Point to;
		float x, y, z;
		Vector3F(Point start, Point to) {
			this->st = start;
			this->to = to;
			Point p = this->to - this->st;
			x = p.x;
			y = p.y;
			z = p.z;
		}

		/**
		* Calculates the cross product between this vector and the given vector. 
		* @param other the other vector to use. 
		*/
		Point crossProduct(const Vector3F &other) {
			float nx = y * other.z - z * other.y;
			float ny = z * other.x - x *other.z;
			float nz = x *other.y - y * other.x;
			return Point(nx, ny, nz);
		}
		
	};

	struct TriangleF {
		Point p1, p2, p3;
		TriangleF(Point one, Point two, Point three) {
			p1 = one;
			p2 = two;
			p3 = three;
		}
		Point normal() {
			Vector3F first(p1, p2);
			Vector3F second(p1, p3);
			return first.crossProduct(second);
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
		* Get sorts the x y z indices of the triangle.
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

		/**
		* Unsorts the triangle back to its original order. 
		*/
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
}
#endif