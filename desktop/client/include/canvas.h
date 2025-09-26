#ifndef CANVAS_H
#define CANVAS_H

#include <QWidget>
#include <stdbool.h>
#include <QPainter>
#include <QFont>
#include <QFontDatabase>
#include <QTimer>
#include <QMediaPlayer>
#include <QAudioOutput>

class Canvas : public QWidget
{

private:
    int m_speed;
    bool m_error;
    int batteryLevel;
    int temperature;
    bool leftSignal;
    bool rightSignal;
    void drawSpeedometer(QPainter &painter);
    QFont materialIconsFont;
    void drawEnergyIcon(QPainter &painter, int batteryLevel);
    void drawTemperatureIcon(QPainter &painter, int temperature);
    void drawTurnSignal(QPainter &painter, bool leftSignal, bool rightSignal);
    QTimer blinkTimer;
    QMediaPlayer m_media;
    QAudioOutput m_audiooutput;
    bool blinkOn = true;

public:
    explicit Canvas();

    void setSpeed(int speed);
    void setErrorState(bool state);
    void setBatteryLevel(int level);
    void setTemperature(int temp);
    void setLeftSignal(bool state);
    void setRightSignal(bool state);

protected:
    void paintEvent(QPaintEvent *event) override;
};

#endif
