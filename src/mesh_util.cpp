#include "mesh_util.h"


namespace MeshUtil {

	/**
	* Mesh class that encapsulates a MEDIT style .mesh file. 
	* @param vert the verticies
	* @param ts the triangles
	* @param tts tetrahedra.
	*/
	Mesh::Mesh(std::vector<Point> verts, std::vector<Triangle> ts, std::vector<Tetrahedron> tts) {
		this->verts.reserve(verts.size());
		this->verts = verts;

		this->tris.reserve(ts.size());
		this->tris = ts;

		this->tetras.reserve(tts.size());
		this->tetras = tts;
	}

	/**
	* Mesh class that encapsulates a MEDIT style .mesh file.
	* @param pointSize the number of points in the mesh.
	* @param triangleSize the number of triangles in the mesh. 
	* @param tetraSize the number of tetrahedra in the mesh. 
	*/
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

	/**
	* Get a single point from the mesh. 
	* @param index the index to get the point from. 
	*/
	Point Mesh::getPoint(int index) {
		return verts[index];
	}

	/**
	* Set a point in the mesh at a given index. 
	* @param newP the new point.
	* @param index the index to insert the new point at. 
	*/
	void Mesh::setPoint(Point newP, int index) {
		verts[index] = newP;
	}
	/**
	* Get a triangle at a given index. 
	* @param index the triangle index. 
	* @return Triangle
	*/
	Triangle Mesh::getTriangle(int index) {
		return tris[index];
	}

	/**
	* Sets a triangle at a given index. 
	* @param newT the new Triangle.
	* @param index the index to insert the triangle at. 
	*/
	void Mesh::setTriangle(Triangle newT, int index) {
		tris[index] = newT;
	}

	/**
	* Get a tetrahedron at a given index. 
	* @param the index to read at. 
	* @return Tetrahedron the read tetrahedron. 
	*/
	Tetrahedron Mesh::getTetrahedron(int index) {
		return tetras[index];
	}

	/*
	* Set a tetrahedron at a given index. 
	* @param tts the new tetrahedron to insert. 
	* @param index the index to insert the tetrahedron at. 
	*/
	void Mesh::setTetrahedron(Tetrahedron tts, int index) {
		tetras[index] = tts;
	}

	/**
	* Returns the unique faces of the mesh. These should be the surface faces (i.e. triangles).
	* @return std::vector<Triangle> a vector of triangle faces. 
	*/
	std::vector<Triangle> Mesh::getSurfaceFaces() {
		//first need to get all the faces for all the tetraheda using these coordinates
		//(x y z), (x y h), (x z h), (y z h)
		std::vector<Triangle> faces;
		if (tetras.size() > 0) {
			for (int i = 0; i < tetras.size(); i++) {
				Tetrahedron t = tetras[i];
				Triangle f1(t.x, t.y, t.z);
				Triangle f2(t.x, t.y, t.h);
				Triangle f3(t.x, t.z, t.h);
				Triangle f4(t.y, t.z, t.h);
				//sort all the triangles (used to eliminate duplicates) 
				f1.sort(); f2.sort(); f3.sort(); f4.sort();
				//add all the faces. 
				faces.push_back(f1); faces.push_back(f2); faces.push_back(f3); faces.push_back(f4);
			}
	
			//sort all the faces. 
			std::sort(faces.begin(), faces.end());
			//create frequency map (histogram) to get faces that only occur once. 
			std::map<Triangle, int>freq_map;
			for (auto const & x : faces) {
				++freq_map[x];
			}
			//get the unique faces. 
			std::vector<Triangle> uFaces;
			for (auto &p : freq_map) {
				//only 1 so it's unique. 
				if (p.second == 1) {
					uFaces.push_back(p.first);
				}
			}
			//return the unique faces. 
			return uFaces;
		}
		return faces;
	}

	/**
	* Class that loads a mesh from a .mesh file. 
	* @param parent the QObject parent
	* @param filename the full file path. 
	* @param params parameters for the mesh, specifying offsets for x y and z. 
	*/
	MeshLoader::MeshLoader(QObject* parent, QString filename, MeshParams params) :
		QThread(parent), filename(filename) {
		//set the params. 
		this->params = params;
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
		//point, triangle and tetrahedron containers. 
		std::vector<Point> verts;
		std::vector<Triangle> tris;
		std::vector<Tetrahedron> tets;

		//open the file. 
		QFile input(filename);
		input.open(QIODevice::ReadOnly);
		//input stream
		QTextStream ts(&input);
		int numVerts, numTriangles, numTetrahedron;
		int state = -1; // state for what we're currently reading. 
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
						//apply the offset as we're reading the values. 
						if (params.xOffset != 0.0 || params.yOffset != 0.0 || params.zOffset != 0.0) {
							//apply the offset. 
							x+=params.xOffset;
							y+=params.yOffset;
							z+=params.zOffset;
						}
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
		if (tets.size() > 0) {
			//sort and remove duplicate tetrahedrons. 
			std::sort(tets.begin(), tets.end());
			tets.erase(std::unique(tets.begin(), tets.end()), tets.end());	
		}
		//return the new mesh. 
		Mesh *mesh = new Mesh(verts, tris, tets);
		return mesh;
	}

	/**
	* Rendering class for a MEDIT .mesh file. 
	* @param mesh the MEDIT mesh to render. 
	*/
	GLMesh::GLMesh(Mesh* mesh)	{
		initializeGLFunctions();

		//need to get the verticies and indices of the mesh.
		int pointCount = 0;
		//get the surface faces. 
		std::vector<Triangle> faces = mesh->getSurfaceFaces();
		//set our verticies based on the faces. 
		for (int i = 0; i < faces.size(); i++) {
			Triangle t = faces[i];
			Point first = mesh->getPoint(t.x-1);
			Point second = mesh->getPoint(t.y-1);
			Point third = mesh->getPoint(t.z-1);
			mVerts.push_back(first.x);
			mVerts.push_back(first.y);
			mVerts.push_back(first.z);
			pointCount++;
			mInd.push_back(pointCount);
			mVerts.push_back(second.x);
			mVerts.push_back(second.y);
			mVerts.push_back(second.z);
			pointCount++;
			mInd.push_back(pointCount);
			mVerts.push_back(third.x);
			mVerts.push_back(third.y);
			mVerts.push_back(third.z);
			pointCount++;
			mInd.push_back(pointCount);
		}
	}

	/**
	* Bind the mesh and draw it into the opengl scene. 
	* @param vp the point to draw around.
	*/
	void GLMesh::draw(GLuint vp)
	{
		glBegin(GL_TRIANGLES);
		for (int i = 0; i < mVerts.size(); i+=3) {
			glVertex3f(mVerts[i], mVerts[i + 1], mVerts[i + 2]);
		}
		glEnd();
	}

	/**
	* Gets the minimum of the mesh.
	* @param start the axis to search on. 0 = x, 1 = y, 2 = z.
	*/
	float GLMesh::min(size_t start) const
	{
		float v = mVerts[start];
		for (size_t i = start; i < mVerts.size(); i += 3)
		{
			v = fmin(v, mVerts[i]);
		}
		return v;
	}

	/**
	* Gets the max of the mesh.
	* @param start the axis to search on. 0 = x, 1 = y, 2 = z.
	*/
	float GLMesh::max(size_t start) const
	{
		float v = mVerts[start];
		for (size_t i = start; i < mVerts.size(); i += 3)
		{
			v = fmax(v, mVerts[i]);
		}
		return v;
	}
}
