#include "mesh_util.h"

namespace MeshUtil {
	

	Mesh::Mesh(std::vector<Point> verts, std::vector<Triangle> ts, std::vector<Tetrahedron> tts) {
		this->verts.reserve(verts.size());
		this->verts = verts;

		this->tris.reserve(ts.size());
		this->tris = ts;

		this->tetras.reserve(tts.size());
		this->tetras = tts;
	}

	Mesh::Mesh(int pointSize, int triangleSize, int tetraSize) {
		this->verts.reserve(pointSize);
		this->tris.reserve(triangleSize);
		this->tetras.reserve(tetraSize);
		for (int i = 0; i < pointSize; i++) {
			Point p;
			verts.push_back(p);
		}
		for (int j = 0; j < triangleSize; j++) {
			Triangle t;
			tris.push_back(t);
		}
		for (int q = 0; q < tetraSize; q++) {
			Tetrahedron tr;
			tetras.push_back(tr);
		}
	}

	Point Mesh::getPoint(int index) {
		return verts[index];
	}

	void Mesh::setPoint(Point newP, int index) {
		verts[index] = newP;
	}

	Triangle Mesh::getTriangle(int index) {
		return tris[index];
	}

	void Mesh::setTriangle(Triangle newT, int index) {
		tris[index] = newT;
	}

	Tetrahedron Mesh::getTetrahedron(int index) {
		return tetras[index];
	}

	void Mesh::setTetrahedron(Tetrahedron tts, int index) {
		tetras[index] = tts;
	}

	/**
	* Class that loads a mesh from a .mesh file. 
	* @param parent the QObject parent
	* @param filename the full file path. 
	*/
	MeshLoader::MeshLoader(QObject* parent, QString filename) :
		QThread(parent), filename(filename) {
		//nothing to do here. 
	}

	/**
	* Runs the mesh loader. 
	*/
	void MeshLoader::run() {
		Mesh* m = readMeshFromFile();
		if (m) {
			emit(meshLoaded(m));
		}
		else {
			emit(meshLoadFailed());
		}
	}

	/**
	* Performs the work of reading a mesh from a file. 
	* @return Mesh* the mesh read from the file. 
	*/
	Mesh* MeshLoader::readMeshFromFile() {
		std::vector<Point> verts;
		std::vector<Triangle> tris;
		std::vector<Tetrahedron> tets;
		QFile input(filename);
		input.open(QIODevice::ReadOnly);
		QTextStream ts(&input);
		int numVerts, numTriangles, numTetrahedron;
		int state = -1;
		QString line;
		while (!ts.atEnd()) {
			line = ts.readLine();
			if (line.contains("Vertices")) {
				state = 0;
				line = ts.readLine();
				numVerts = line.toInt();
			}
			else if (line.contains("Triangles")) {
				state = 1;
				line = ts.readLine();
				numTriangles = line.toInt();
			}
			else if (line.contains("Tetrahedra")) {
				state = 2;
				line = ts.readLine();
				numTetrahedron = line.toInt();
			}
			else {
				switch (state) {
				case -1: //nothing
					break;
				case 0:  {
					//verticies
					QStringList values = line.trimmed().split(" ");
					if (values.size() >= 3) {
						float x = values[0].toFloat();
						float y = values[1].toFloat();
						float z = values[2].toFloat();
						Point p(x, y, z);
						verts.push_back(p);
					}
				}
					break;
				case 1: {
					//triangles
					QStringList tVals = line.trimmed().split(" ");
					if (tVals.size() >= 3) {
						int x = tVals[0].toInt();
						int y = tVals[1].toInt();
						int z = tVals[2].toInt();
						Triangle t(x, y, z);
						tris.push_back(t);
					}
				}
					break;
				case 2: {
					//tetrahedra
					QStringList tetVals = line.trimmed().split(" ");
					if (tetVals.size() >= 4) {
						int x = tetVals[0].toInt();
						int y = tetVals[1].toInt();
						int z = tetVals[2].toInt();
						int h = tetVals[3].toInt();
						Tetrahedron t(x, y, z, h);
						tets.push_back(t);
					}
				}
					break;
				}
			}
		}
		//clean up the verticies.
		std::sort(verts.begin(), verts.end());
		verts.erase(std::unique(verts.begin(), verts.end()), verts.end());
		Mesh *mesh = new Mesh(verts, tris, tets);
		return mesh;
	}
		
	std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems) {
		std::stringstream ss(s);
		std::string item;
		while (std::getline(ss, item, delim)) {
			elems.push_back(item);
		}
		return elems;
	}

	std::vector<std::string> split(const std::string &s, char delim) {
		std::vector<std::string> elems;
		split(s, delim, elems);
		return elems;
	}
}
