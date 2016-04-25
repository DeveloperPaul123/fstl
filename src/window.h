#ifndef WINDOW_H
#define WINDOW_H

#include <QMainWindow>
#include "medical.h"

class Canvas;

class Window : public QMainWindow
{
    Q_OBJECT
public:
    explicit Window(QWidget* parent=0);
    bool load_stl(const QString& filename);
	bool load_mhd(const QString& filename);

protected:
    void dragEnterEvent(QDragEnterEvent* event);
    void dropEvent(QDropEvent* event);

public slots:
    void on_open();
    void on_about();
    void on_bad_stl();
	void on_open_mhd();

    void enable_open();
    void disable_open();

private:
    QAction* const open_action;
    QAction* const about_action;
    QAction* const quit_action;
	QAction* const open_mhd_action;

    Canvas* canvas;
};

#endif // WINDOW_H
