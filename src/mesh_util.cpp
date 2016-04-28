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
				//sort each individual triangle. 
				f1.sort();
				f2.sort();
				f3.sort();
				f4.sort();
				//check for duplicates as we go to save time later. 
				std::vector<Triangle> temp;
				temp.push_back(f1);
				temp.push_back(f2);
				temp.push_back(f3);
				temp.push_back(f4);
				std::sort(temp.begin(), temp.end());
				std::vector<Triangle>::iterator mIt;
				mIt = std::adjacent_find(temp.begin(), temp.end());
				while (mIt != temp.end()) {
					temp.erase(std::remove(temp.begin(), temp.end(), *mIt), temp.end());
					mIt = std::adjacent_find(temp.begin(), temp.end());
				}
			
				if (temp.size() > 0) {
					for (int j = 0; j < temp.size(); j++) {
						faces.push_back(temp[j]);
					}
				}
			}
	
			//sort all the faces. 
			std::sort(faces.begin(), faces.end());
			//create frequency map to get faces that only occur once. 
			std::map<Triangle, int>freq_map;
			for (auto const & x : faces) {
				++freq_map[x];
			}
			//get the unique faces. 
			std::vector<Triangle> uFaces;
			for (auto const & p : freq_map) {
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
	*/
	MeshLoader::MeshLoader(QObject* parent, QString filename, MeshParams params) :
		QThread(parent), filename(filename) {
		
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
		//check for offset. 
		if (verts.size() > 0 &&
			(params.xOffset != 0.0 || params.yOffset != 0.0 || params.zOffset != 0.0)) {
			//adjust the points for offset. 
			for (int i = 0; i < verts.size(); i++) {
				Point p = verts[i];
				float x = p.x;
				float y = p.y;
				float z = p.z;
				p.x = x + params.xOffset;
				p.y = y + params.yOffset;
				p.z = z + params.zOffset;
				verts[i] = p;
			}
		}
		if (tets.size() > 0) {
			//sort and remove duplicate tetrahedrons. 
			std::sort(tets.begin(), tets.end());
			tets.erase(std::unique(tets.begin(), tets.end()), tets.end());
			//now need to get unique x positions
			//create a column vector. 
			std::vector<size_t> columnVec;
			for (int i = 0; i < tets.size(); i++) {
				Tetrahedron t = tets[i];
				columnVec.push_back(t.x);
				columnVec.push_back(t.y);
				columnVec.push_back(t.z);
				columnVec.push_back(t.h);
			}
			std::sort(columnVec.begin(), columnVec.end());
			columnVec.erase(std::unique(columnVec.begin(), columnVec.end()), columnVec.end());
		}

		//clean up the verticies.
		Mesh *mesh = new Mesh(verts, tris, tets);
		return mesh;
	}

	GLMesh::GLMesh(Mesh* mesh)
		: vertices(QGLBuffer::VertexBuffer), indices(QGLBuffer::IndexBuffer)
	{
		initializeGLFunctions();

		vertices.create();
		indices.create();

		vertices.setUsagePattern(QGLBuffer::StaticDraw);
		indices.setUsagePattern(QGLBuffer::StaticDraw);
		
		//need to get the verticies and indices of the mesh.
		int pointCount = 0;
		std::vector<Triangle> faces = mesh->getSurfaceFaces();
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
			mVerts.push_back(second.x-1);
			mVerts.push_back(second.y-1);
			mVerts.push_back(second.z-1);
			pointCount++;
			mInd.push_back(pointCount);
			mVerts.push_back(third.x-1);
			mVerts.push_back(third.y-1);
			mVerts.push_back(third.z-1);
			pointCount++;
			mInd.push_back(pointCount);
		}

		vertices.bind();
		vertices.allocate(mVerts.data(),
			mVerts.size() * sizeof(float));
		vertices.release();

		indices.bind();
		indices.allocate(mInd.data(),
			mInd.size() * sizeof(uint32_t));
		indices.release();
	}

	void GLMesh::draw(GLuint vp)
	{
		/*vertices.bind();
		indices.bind();

		glVertexAttribPointer(vp, 3, GL_FLOAT, false, 3 * sizeof(float), NULL);
		glDrawElements(GL_TRIANGLES, indices.size() / sizeof(uint32_t),
			GL_UNSIGNED_INT, NULL);

		vertices.release();
		indices.release();*/

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

	std::vector<Tetrahedron> isMember(std::vector<Tetrahedron> tets, std::vector<int> nodes) {
		std::vector<Tetrahedron> output;
		//todo...
		return output;
	}
}
