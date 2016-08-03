#ifndef MESH_UTIL_H
#define MESH_UTIL_H

#include <vector>
#include <fstream>

#include <QObject>
#include <QString>
#include <QFile>
#include <QThread>
#include <QTextStream>

#include <QtOpenGL/QGLFunctions>

#include "mesh_util_core.h"

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
		void run() override;

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