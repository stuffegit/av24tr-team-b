#include "window.h"
#include "setting.h"
#include <QVBoxLayout>
#include <iostream>

Window::Window(COMService &comservice)
    : comservice(comservice), m_timer(this) // Construct m_timer with this as parent
{
    setFixedSize(800, 600);
    setWindowFlags(Qt::WindowStaysOnTopHint);
    // Even though we aren't using setParent explicitly for m_canvas,
    // its lifetime is tied to Window because it's declared as a member.
    m_canvas.setGeometry(0, 0, 800, 600);

    // Set initial values
    m_canvas.setErrorState(false);

    // Optionally, using a layout if you wish to manage positioning:
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(&m_canvas);
    setLayout(layout);

    // Connect the timer's timeout signal to the updateCanvas slot.
    connect(&m_timer, &QTimer::timeout, this, &Window::updateCanvas);
    m_timer.start(Setting::INTERVAL); // For example, 40ms (or whatever interval you need)
}

void Window::updateCanvas()
{
    m_canvas.setErrorState(!comservice.getStatus()); // Comservice shows true if connected, false if not. Error state is set to true if not connected.
    m_canvas.setSpeed(comservice.getSpeed());
    m_canvas.setBatteryLevel(comservice.getBatteryLevel());
    m_canvas.setTemperature(comservice.getTemperature());
    m_canvas.setLeftSignal(comservice.getLeftLight());
    m_canvas.setRightSignal(comservice.getRightLight());
    m_canvas.update();
}
