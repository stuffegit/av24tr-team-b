#include <QApplication>
#include "window.h"
#ifdef UARTCOM
#include "uartservice.h"
#else
#include "tcpservice.h"
#endif

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

#ifdef UARTCOM
    UARTService service;
#else
    TCPService service;
#endif

    Window window(service);
    window.setWindowTitle("Client");
    window.show();

    return app.exec();
}
