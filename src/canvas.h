#ifndef CANVAS_H
#define CANVAS_H

#include <QtOpenGL/QGLShaderProgram>
#include <QMatrix4x4>

#include "medical.h"
#include "mesh_util.h"

class GLMesh;
class Mesh;
class Backdrop;

class Canvas : public QGLWidget, protected QGLFunctions
{
    Q_OBJECT

public:
	explicit Canvas(const QGLFormat& format, QWidget* parent=nullptr);
	~Canvas();

public slots:
    void set_status(const QString& s);
    void clear_status();
    void load_mesh(Mesh* m);
	void load_volume(UcharVolume *vol);
	void load_mesh_file(MeshUtil::Mesh *m);

protected:
	void initializeGL() override;
	void resizeGL(int width, int height) override;
	void mousePressEvent(QMouseEvent* event) override;
	void mouseReleaseEvent(QMouseEvent* event) override;
	void mouseMoveEvent(QMouseEvent* event) override;
	void wheelEvent(QWheelEvent* event) override;
	void paintEvent(QPaintEvent* event) override;


private:
    void draw_mesh();
	void draw_cloud();
	void draw_mesh_file();
    QMatrix4x4 transform_matrix() const;
    QMatrix4x4 view_matrix() const;

    QGLShaderProgram mesh_shader;
    QGLShaderProgram quad_shader;

    GLMesh* mesh;
	GLPointCloud* cloud;
	MeshUtil::GLMesh *meshFromFile;
    Backdrop* backdrop;

    QVector3D center;
    float scale;
    float zoom;
    float tilt;
    float yaw;

    QPoint mouse_pos;
    QString status;
};

#endif // CANVAS_H
