#ifndef MEDICAL_H
#define MEDICAL_H
#include <vector>
#include <QObject>
#include <QString>
#include <QThread>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QTextStream>
#include <QDataStream>
#include <QVector>
#include <QVector3D>
#include <QtOpenGL/QGLBuffer>
#include <QtOpenGL/QGLFunctions>

enum MHDElementType {
	MET_UCHAR = 0
};

struct MHDInfo {
	int num_dims = 0;
	bool isBinary = false;
	std::vector<float> spacing;
	std::vector<int> dims;
	MHDElementType type = MET_UCHAR;
	QString filename;
};

template<class T> class Slice {
public:
	Slice<T>(int xSize, int ySize) {
		mXDim = xSize;
		mYDim = ySize;
		for (int i = 0; i < xSize; i++) {
			std::vector<T> data;
			data.reserve(ySize);
			for (int j = 0; j < ySize; j++) {
				data.push_back(0);
			}
			mData.push_back(data);
		}
	}
	int getXSize() {
		return mXDim;
	}
	int getYSize() {
		return mYDim;
	}
	void setData(int x, int y, T val) {
		mData[x][y] = val;
	}
	T getData(int x, int y) {
		return mData[x][y];
	}

private:
	std::vector<std::vector<T>> mData;
	int mXDim, mYDim;
};

template<class T> class Volume {
public:
	Volume<T>(int x, int y, int z, float xSpacing, float ySpacing, float zSpacing) {
		data.reserve(z);
		for (int i = 0; i < z; i++) {
			data.push_back(Slice<T>(x, y));
		}
		_xDim = x;
		_yDim = y;
		_zDim = z;
		_xSp = xSpacing;
		_ySp = ySpacing;
		_zSp = zSpacing;
	}
	int getXDim() {
		return _xDim;
	}
	int getYDim(){
		return _yDim;
	}
	int getZDim() {
		return _zDim;
	}

	int getNumSlices() {
		return data.size();
	}

	Slice<T> getSlice(int index) {
		return data[index];
	}
	void addSlice(Slice<T> slice, int index) {
		data[index] = slice;
	}
private:
	std::vector<Slice<T>> data;
	int _xDim, _yDim, _zDim;
	float _xSp, _ySp, _zSp;
};

class UcharVolume : public Volume<uchar>{
public:
	UcharVolume(int x, int y, int z, float xSpacing, float ySpacing, float zSpacing) : 
		Volume<uchar>(x, y, z, xSpacing, ySpacing, zSpacing) {

	}
};

class MHDLoader : public QThread {
	Q_OBJECT

public:
	explicit MHDLoader(QObject* parent, const QString& filename);
	void run();

protected:	
	UcharVolume* readVolume();

signals :
	void loaded_volume(UcharVolume* vol);
	void loaded_file(QString filename);
	void error();

private:
	const QString filename;
	MHDInfo readHeaderInfo();
};

class GLPointCloud : protected QGLFunctions {
public:
	GLPointCloud(UcharVolume* vol);
	void draw(GLuint vp);

private:
	QGLBuffer vertices;
	QGLBuffer indices;
};
#endif //MEDICAL_H