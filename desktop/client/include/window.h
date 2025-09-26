#ifndef WINDOW_H
#define WINDOW_H

#include <QDialog>
#include "canvas.h"
#include "comservice.h"
#include <QTimer>

class Window : public QDialog
{
public:
    explicit Window(COMService &comservice);
    void updateCanvas();

private:
    Canvas m_canvas;
    COMService &comservice;
    QTimer m_timer;
};

#endif