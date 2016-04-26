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

namespace MeshUtil {
	static std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems);
	static std::vector<std::string> split(const std::string &s, char delim);

	struct Point {
		Point() {
			x = 0.0;
			y = 0.0;
			z = 0.0;
		}
		Point(int x1, int y1, int z1) {
			x = x1;
			y = y1;
			z = z1;
		}
		float x;
		float y;
		float z;
		float distanceTo(Point p) {
			float xDif = p.x - x;
			float yDif = p.y - y;
			float zDif = p.z - z;
			return sqrtf(pow(xDif, 2.0) + pow(yDif, 2.0) + pow(zDif, 2.0));
		}
	};
	struct Triangle {
		Triangle() {
			x = 0;
			y = 0;
			z = 0;
		}
		Triangle(int x1, int y1, int z1) {
			x = x1;
			y = y1;
			z = z1;
		}
		int x;
		int y;
		int z;
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
	private:
		std::vector<Point> verts;
		std::vector<Triangle> tris;
		std::vector<Tetrahedron> tetras;
	};

	
	class MeshLoader : public QThread{
		Q_OBJECT

	public: 
		explicit MeshLoader(QObject *parent, QString filename);
		void run();

	signals:
		void meshLoaded(Mesh *mesh);
		void meshLoadFailed();
	protected:
		Mesh* readMeshFromFile();
	private:
		QString filename;
	};
}
#endif