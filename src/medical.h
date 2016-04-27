#ifndef MEDICAL_H
#define MEDICAL_H
#include <vector>
#include <iostream>
#include <fstream>
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
	std::vector<float> offset;
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

/**
* Template class representing a .mhd and .raw volume. 
*/
template<class T> class Volume {
public:
	/**
	* Class that encapsulates a volume. 
	* @param x the x dimension size.
	* @param y the y dimension size.
	* @param z the z dimension size.
	* @param xSpacing the voxel size in x
	* @param ySpacing the voxel size in y
	* @param zSpacing the voxel size in z
	*/
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
		_xOff = 0.0;
		_yOff = 0.0;
		_zOff = 0.0;
	}

	/**
	* Set offsets in x y and z.
	* @param xOff the x offset. 
	* @param yOff the y offset.
	* @param zOff the z offset. 
	*/
	void setOffset(float xOff, float yOff, float zOff) {
		_xOff = xOff;
		_yOff = yOff;
		_zOff = zOff;
	}

	/**
	* Returns the x offset.
	* @return x offset.
	*/
	float getXOffset() {
		return _xOff;
	}

	/**
	* Return the y offset.
	* @return y offset. 
	*/
	float getYOffset() {
		return _yOff;
	}

	/**
	* Returns the z offset.
	* @return z offset. 
	*/
	float getZOffset() {
		return _zOff;
	}

	/**
	* Returns the x dimension.
	* @return the x dimension.
	*/
	int getXDim() {
		return _xDim;
	}

	/**
	* Returns the y dimension.
	* @return the y dimension.
	*/
	int getYDim(){
		return _yDim;
	}

	/**
	* Returns the z dimension.
	* @return the z dimension.
	*/
	int getZDim() {
		return _zDim;
	}

	/**
	* Returns the x voxel size.
	* @return the x voxel size.
	*/
	float getXVoxel() {
		return _xSp;
	}

	/**
	* Returns the y voxel size.
	* @return the y voxel size.
	*/
	float getYVoxel() {
		return _ySp;
	}

	/**
	* Returns the z voxel size.
	* @return the z voxel size. 
	*/
	float getZVoxel() {
		return _zSp;
	}

	/**
	* Returns the total number of slices in the volume. 
	* @return int the total slices. 
	*/
	int getNumSlices() {
		return data.size();
	}

	/**
	* Returns a Slice<T> at a given index. 
	* @param index the index to look for a slice.
	* @return Slice<T> the slice at the given index. 
	*/
	Slice<T> getSlice(int index) {
		return data[index];
	}
	
	/**
	* Adds a given slice to at the given index to the volume. 
	* @param slice the slice to add. 
	* @param index where to add the slice. 
	*/
	void addSlice(Slice<T> slice, int index) {
		data[index] = slice;
	}
private:
	std::vector<Slice<T>> data;
	int _xDim, _yDim, _zDim;
	float _xSp, _ySp, _zSp, _xOff, _yOff, _zOff;
};

/**
* Volume for unsigned char type. 
*/
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

class INRSaver : public QThread {
	Q_OBJECT

public:
	explicit INRSaver(QObject *parent, UcharVolume *vol);
	void run();

protected:
	bool saveVolume();

signals:
	void volumeSavedSuccessfully();
	void volumeSaveFailed();

private:
	UcharVolume* mVol;
};

class GLPointCloud : protected QGLFunctions {
public:
	GLPointCloud(UcharVolume* vol);
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
	QGLBuffer vertices;
	QGLBuffer indices;

	std::vector<GLfloat> mVerts;
	std::vector<GLuint> mInd;
};
#endif //MEDICAL_H