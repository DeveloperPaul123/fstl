#include "medical.h"
/**
* Loads an mhd and .raw file. 
* @param parent the qobject parent. 
* @param filename full path to the .mhd file. 
*/
MHDLoader::MHDLoader(QObject* parent, const QString& filename) :
	QThread(parent), filename(filename) {

}

/**
* Run the loader. 
*/
void MHDLoader::run() {
	UcharVolume *vol = readVolume();
	if (vol) {
		emit loaded_file(filename);
		emit loaded_volume(vol);
	}
}

/**
* Reads a volume and header. 
*/
UcharVolume* MHDLoader::readVolume() {
	//first read the header file.
	MHDInfo info = readHeaderInfo();
	
	//get parent path of mhd header. 
	QFileInfo fileInfo(filename);
	QDir d = fileInfo.dir();
	QString dirPath = d.absolutePath();
	dirPath.append("\\");
	QString rawFile = dirPath + info.filename;
	
	//open the data file.
	QFile raw(rawFile);
	raw.open(QIODevice::ReadOnly);

	//open as byte stream. 
	QDataStream stream(&raw);
	stream.setByteOrder(QDataStream::LittleEndian);
	stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
	
	long result = 1;
	std::vector<int> dims = info.dims;
	for (int i = 0; i < info.num_dims; i++) {
		result *= dims[i];
	}
	if (result != raw.size()) {
		emit error();
		return NULL;
	}

	//read the raw data. 
	QVector<uchar> rawData(result);

	QByteArray arr = raw.readAll();
	for (int i = 0; i < arr.length(); i++) {
		rawData[i] = arr.at(i);
	}
	int xDim = info.dims[0];
	int yDim = info.dims[1];
	int zDim = info.dims[2];
	float xS = info.spacing[0];
	float yS = info.spacing[1];
	float zS = info.spacing[2];
	UcharVolume *vol = new UcharVolume(xDim, yDim, zDim, xS, yS, zS);
	if (rawData.size() > 0) {
		for (int z = 0; z < zDim; z++) {
			Slice<uchar> curSlice(xDim, yDim);
			for (int y = 0; y < yDim; y++) {
				for (int x = 0; x < xDim; x++) {
					int position = x + (y*yDim) + (z*(yDim*xDim));
					curSlice.setData(x, y, rawData.at(position));
				}
			}
			vol->addSlice(curSlice, z);
		}
	}

	return vol;
}

/**
* Reads an mhd header and gets the pertinant info. 
* @param MHDInfo info structure on the raw file. 
*/
MHDInfo MHDLoader::readHeaderInfo() {
	MHDInfo info;
	QFile file(filename);
	file.open(QIODevice::ReadOnly);
	QString line;
	QTextStream in(&file);
	while (!in.atEnd()){
		line = in.readLine();
		QStringList items = line.split(" = ", QString::SkipEmptyParts);
		if (items.length() > 0) {
			QString dataItem = items.at(0).toLower();
			if (dataItem == "ndims") {
				int dims = items.at(1).toInt();
				info.num_dims = dims;
			}
			else if (dataItem == "dimsize") {
				QString dims = items.at(1);
				QStringList l = dims.split(" ");
				std::vector<int> d;
				for (int i = 0; i < l.length(); i++) {
					d.push_back(l.at(i).trimmed().toInt());
				}
				info.dims = d;
			}
			else if (dataItem == "elementType") {
				QString type = items.at(1);
				if (type == "MET_UCHAR") {
					info.type = MET_UCHAR;
				}
			}
			else if (dataItem == "elementdatafile") {
				info.filename = items.at(1).trimmed();
			}
			else if (dataItem == "elementspacing") {
				QString spac = items.at(1);
				QStringList l = spac.split(" ");
				std::vector<float> f;
				for (int i = 0; i < l.length(); i++) {
					f.push_back(l.at(i).trimmed().toFloat());
				}
				info.spacing = f;
			}
		}
	}
	return info;
}

GLPointCloud::GLPointCloud(UcharVolume* vol) :
vertices(QGLBuffer::VertexBuffer), indices(QGLBuffer::IndexBuffer) {
	
	initializeGLFunctions();
	vertices.create();
	indices.create();

	vertices.setUsagePattern(QGLBuffer::StaticDraw);
	indices.setUsagePattern(QGLBuffer::StaticDraw);

	std::vector<GLfloat> mVerts;
	std::vector<GLuint> mInd;
	int pointCount = 0;
	int size = vol->getNumSlices();
	for (int i = 0; i < size; i++) {
		Slice<uchar> slice = vol->getSlice(i);
		for (int y = 0; y < slice.getYSize(); y++) {
			for (int x = 0; x < slice.getXSize(); x++) {
				uchar data = slice.getData(x, y);
				if (data != 0) {
					mVerts.push_back(GLfloat(x));
					mVerts.push_back(GLfloat(y));
					mVerts.push_back(GLfloat(i));
					mInd.push_back(pointCount);
					pointCount++;
				}
			}
		}
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

void GLPointCloud::draw(GLuint vp) {
	vertices.bind();
	indices.bind();

	glVertexAttribPointer(vp, 3, GL_FLOAT, false, 3 * sizeof(float), NULL);
	glDrawElements(GL_POINTS, indices.size() / sizeof(uint32_t),
		GL_UNSIGNED_INT, NULL);

	vertices.release();
	indices.release();
}