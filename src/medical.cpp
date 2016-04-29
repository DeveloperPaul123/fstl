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
* Reads a .mhd header and the corresponding .raw file as a volume. 
* @return UcharVolume a volume with type uchar. 
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
					int position = x + (y*xDim) + (z*(yDim*xDim));
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
			else if (dataItem == "offset") {
				QString spac = items.at(1);
				QStringList l = spac.split(" ");
				std::vector<float> f;
				for (int i = 0; i < l.length(); i++) {
					f.push_back(l.at(i).trimmed().toFloat());
				}
				info.offset = f;
			}
		}
	}
	return info;
}

/**
* Render class for a uchar volume in OpenGL. 
* @param vol the volume to render. 
*/
GLPointCloud::GLPointCloud(UcharVolume* vol) :
vertices(QGLBuffer::VertexBuffer), indices(QGLBuffer::IndexBuffer) {
	
	initializeGLFunctions();
	vertices.create();
	indices.create();

	vertices.setUsagePattern(QGLBuffer::StaticDraw);
	indices.setUsagePattern(QGLBuffer::StaticDraw);

	int pointCount = 0;
	int size = vol->getNumSlices();
	for (int i = 0; i < size; i++) {
		Slice<uchar> slice = vol->getSlice(i);
		for (int y = 0; y < slice.getYSize(); y++) {
			for (int x = 0; x < slice.getXSize(); x++) {
				uchar data = slice.getData(x, y);
				if (data != 0) {
					float xVal = (float)x;
					float yVal = (float)y;
					float zVal = (float)i;
					if (vol->getXOffset() != 0.0) {
						xVal += vol->getXOffset();
					}
					if (vol->getYOffset() != 0.0) {
						yVal += vol->getYOffset();
					}
					if (vol->getZOffset() != 0.0) {
						zVal += vol->getZOffset();
					}
					mVerts.push_back(xVal);
					mVerts.push_back(yVal);
					mVerts.push_back(zVal);
					mInd.push_back(pointCount);
					pointCount++;
				}
			}
		}
	}

	//allocate the verticies
	vertices.bind();
	vertices.allocate(mVerts.data(),
		mVerts.size() * sizeof(float));
	vertices.release();

	//allocate the indices
	indices.bind();
	indices.allocate(mInd.data(),
		mInd.size() * sizeof(uint32_t));
	indices.release();
}

/**
* Draw the point cloud in the opengl scene. 
* @param vp the GLuint to draw around. 
*/
void GLPointCloud::draw(GLuint vp) {
	vertices.bind();
	indices.bind();

	glVertexAttribPointer(vp, 3, GL_FLOAT, false, 3 * sizeof(float), NULL);
	glDrawElements(GL_POINTS, indices.size() / sizeof(uint32_t),
		GL_UNSIGNED_INT, NULL);

	vertices.release();
	indices.release();
}

/**
* Gets the minimum of the mesh.
* @param start the axis to search on. 0 = x, 1 = y, 2 = z.
*/
float GLPointCloud::min(size_t start) const
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
float GLPointCloud::max(size_t start) const
{
	float v = mVerts[start];
	for (size_t i = start; i < mVerts.size(); i += 3)
	{
		v = fmax(v, mVerts[i]);
	}
	return v;
}

/**
* Class for saving a volume as an inr image stack. 
* @param parent QObject parent
* @param UcharVolume the unsigned char volume to save. 
*/
INRSaver::INRSaver(QObject * parent, UcharVolume *vol) :
QThread(parent){
	//copy volume. 
	mVol = new UcharVolume(*vol);
}

/**
* Run the inr saver. 
*/
void INRSaver::run() {
	bool saved = saveVolume();
	if (saved) {
		emit(volumeSavedSuccessfully());
	}
	else {
		emit(volumeSaveFailed());
	}
}

/**
* Saves the actual volume, see here for info: 
* http://serdis.dis.ulpgc.es/~krissian/InrView1/IOformat.html
* @return true if saved successfully, false otherwise. 
*/
bool INRSaver::saveVolume() {
	if (mVol) {
		std::ofstream output;
		//open as binary. 
		output.open("C:\\Users\\Paul T\\Desktop\\test.inr", std::ios_base::binary | std::ios_base::out);
		int start = output.tellp();
		output << '#' << 'I' << 'N' << 'R' << 'I' << 'M' << 'A' << 'G' << 'E' << '-' << '4' << '#' << '{' << '\n';
		int xDim = mVol->getXDim();
		int yDim = mVol->getYDim();
		int zDim = mVol->getZDim();

		output << 'X' <<'D' <<'I' << 'M' << '=' << xDim << '\n';
		output << 'Y' << 'D' << 'I' << 'M' << '=' << yDim << '\n';
		output << 'Z' << 'D' << 'I' << 'M' << '=' << zDim << '\n';
		output << 'V' << 'D' << 'I' << 'M' << '=' << 1 << '\n';
		output << 'V'<<'X'<<'=' << mVol->getXVoxel() << '\n';
		output << 'V' << 'Y' << '=' << mVol->getYVoxel() << '\n';
		output << 'V' << 'Z' << '=' << mVol->getZVoxel() << '\n';
		output << 'T' << 'Y' << 'P' << 'E' << '=' << 'u' << 'n' << 's' << 'i' << 'g' << 'n' << 'e' << 'd' << ' ' << 'f' << 'i' << 'x' << 'e' << 'd' << '\n';
		output << 'P' << 'I' << 'X' << 'S' << 'I' << 'Z' << 'E' << '=' << 8 << ' ' << 'b' << 'i' << 't' << 's' << '\n';
		output << 'S' << 'C' << 'A' << 'L' << 'E' << '=' << '2' << '*' << '*' << '0' << '\n';
		output << 'C' << 'P' << 'U' << '=' << 'd' << 'e' << 'c' << 'm' << '\n';
		int soFar = output.tellp();
		int dif = soFar - start;
		int bSize = (dif * sizeof(char));
		int remainder = 256 - bSize - 5;
		for (int i = 0; i < remainder; i++) {
			output << '\n';
		}
		output << '#' <<'#'<<'}' << '\n';
		
		//now write the pixel data, row by row, column by column, slice by slice. 
		int slices = mVol->getNumSlices();
		for (int z = 0; z < slices; z++) {
			//get the slice. 
			Slice<uchar> slice = mVol->getSlice(z);
			int xDim = slice.getXSize();
			int yDim = slice.getYSize();
			for (int y = 0; y < yDim; y++) {
				for (int x = 0; x < xDim; x++) {
					//go through one row at a time and write each column. 
					uchar val = slice.getData(x, y);
					output << val;
				}
			}
		}
		//close the file. 
		output.close();
		return true;
	}
	return false;
}