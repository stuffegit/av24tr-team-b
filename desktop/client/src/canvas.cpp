#include "canvas.h"
#include <QPainter>
#include <QPaintEvent>
#include <QFont>
#include <QDebug>
#include <QtMath>
#include <QTimer>
#include <QTime>

Canvas::Canvas()
    : m_speed(0), m_error(true), batteryLevel(0), temperature(0), leftSignal(false), rightSignal(false)
{
    blinkTimer.setInterval(305); // Hard coded value for blink interval
    connect(&blinkTimer, &QTimer::timeout, this, [this]()
            {
        blinkOn = !blinkOn;
        update(); });
    blinkTimer.start();

    m_media.setAudioOutput(&m_audiooutput);
    m_audiooutput.setVolume(1.0);
}

void Canvas::setSpeed(int speed)
{
    if (!m_error && speed >= 0)
    {
        m_speed = qBound(0, speed, 240);
    }
    else
    {
        m_speed = 0; // If in error state, reset speed to 0.
    }
    update();
}

void Canvas::setErrorState(bool state)
{
    m_error = state;
    if (m_error)
    {
        setSpeed(0);
    }
    else
    {
        setSpeed(m_speed);
    }
}

void Canvas::drawSpeedometer(QPainter &painter)
{
    // -------------------- Stable Design Constants ------------------------
    const double baseDialRadius = 100.0; // The dial is drawn as a circle of radius 100.
    const double arcStartAngle = 200.0;  // At 0 km/h, the arc starts at 200°.
    const double arcSweepAngle = 220.0;  // For a full speed (240 km/h), the arc sweeps 220° (ends at -20°).
    const double extraArcAngle = 5.0;    // Extra angle to extend the arc beyond the dial.
    const int fullSpeed = 240;           // Maximum speed value.
    // Tick mark inner radii:
    const double largeTickInner = 80.0;   // For every 20 km/h.
    const double mediumTickInner = 85.0;  // For every 10 km/h (but not 20).
    const double smallTickInner = 90.0;   // For every 5 km/h that is not a multiple of 10.
    const double tickOuterRadius = 100.0; // Outer edge for all ticks.
    const double labelRadius = 70.0;      // Radius at which numeric labels are positioned.
    const double needleLength = 70.0;     // Shorter needle: extends 70 units from the center.
    // -----------------------------------------------------------------------

    // Set up fonts.
    QFont numFont("Helvetica", 8, QFont::Normal);        // For tick labels (and later for speed text, modified below)
    QFont iconFont("Material Icons", 40, QFont::Normal); // For the speed icon

    // --- Set up the coordinate system ---
    // Define dial region occupying the bottom-left 75% of the canvas.
    double dspWidth = 0.80 * width();   // 0.75 * 800 = 600
    double dspHeight = 0.80 * height(); // 0.75 * 600 = 450
    QRectF dialRegion(0, height() - dspHeight, dspWidth, dspHeight);

    painter.save();
    painter.setClipRect(dialRegion);
    QPointF center = dialRegion.center();
    painter.translate(center);
    int side = qMin(width(), height());
    double scaleFactor = qMin(dialRegion.width(), dialRegion.height()) / 210.0;
    painter.scale(scaleFactor, scaleFactor);

    // --- Draw the dial background ---
    QRectF dialRect(-baseDialRadius, -baseDialRadius, baseDialRadius * 2, baseDialRadius * 2);
    painter.setPen(Qt::NoPen);
    painter.setBrush(Qt::NoBrush);
    painter.drawEllipse(dialRect);

    // --- Draw the extended arc ---
    painter.setPen(QPen(Qt::white, 2));
    // Draw an arc starting at arcStartAngle and sweeping -arcSweepAngle (from 205° to -25°).
    painter.drawArc(dialRect, (arcStartAngle + extraArcAngle) * 16, -(arcSweepAngle + (extraArcAngle * 2)) * 16);

    // --- Draw tick marks (every 5 km/h) ---
    // Loop from 0 to 240 in steps of 5.
    for (int v = 0; v <= fullSpeed; v += 5)
    {
        // Map speed to an angle. (0 km/h -> 200°, 240 km/h -> 200 - 220 = -20°)
        double tickAngle = arcStartAngle - ((static_cast<double>(v) / fullSpeed) * arcSweepAngle);
        painter.save();
        painter.rotate(-tickAngle);

        double innerRadius;
        if (v % 20 == 0)
        {
            innerRadius = largeTickInner; // large tick every 20 km/h
        }
        else if (v % 10 == 0)
        {
            innerRadius = mediumTickInner; // medium tick every 10 km/h (except multiples of 20)
        }
        else
        {
            innerRadius = smallTickInner; // small tick every 5 km/h
        }
        painter.drawLine(QPointF(innerRadius, 0), QPointF(tickOuterRadius, 0));
        painter.restore();
    }

    // --- Draw tick labels (numbers) every 20 km/h ---
    painter.setFont(numFont);
    for (int v = 0; v <= fullSpeed; v += 20)
    {
        double tickAngle = arcStartAngle - ((static_cast<double>(v) / fullSpeed) * arcSweepAngle);
        double angleRad = qDegreesToRadians(-tickAngle); // Use standard polar conversion.
        double x = labelRadius * cos(angleRad);
        double y = labelRadius * sin(angleRad);
        QString label = QString::number(v);
        painter.drawText(QRectF(x - 15, y - 10, 30, 20), Qt::AlignCenter, label);
    }

    // --- Draw the center pivot ---
    // Draw a white-filled circle with a white outline, red center
    painter.setBrush(Qt::red);
    painter.setPen(QPen(Qt::white, 2));
    painter.drawEllipse(QPointF(0, 0), 5, 5);

    // --- Draw the needle as a line ---
    painter.save();
    // Map m_speed (0–240) to an angle along the arc.
    double needleAngle = arcStartAngle - ((static_cast<double>(m_speed) / fullSpeed) * arcSweepAngle);
    painter.rotate(-needleAngle);
    painter.setPen(QPen(Qt::red, 3));
    painter.drawLine(QPointF(0, 0), QPointF(needleLength, 0));
    painter.restore();

    painter.restore(); // End of dial-region drawing.

    // --- Draw the speed icon and numeric value ---
    // Reset transform for screen (window) coordinate drawing.
    painter.resetTransform();
    // Compute a dynamic text rectangle based on the widget size.
    double rectWidth = side * 0.25;
    double rectHeight = side * 0.25;
    QRectF textRect((dspWidth - rectWidth) / 2,
                    (dspHeight + rectHeight) / 1.75,
                    rectWidth,
                    rectHeight);

    if (m_error)
    {
        numFont.setPointSize(10); // Larger font for error message
        painter.setPen(Qt::red);  // Red color for error text
    }
    else
    {
        numFont.setPointSize(24); // Normal speed display size
        painter.setPen(Qt::white);
    }
    painter.setFont(iconFont);
    QString speedIcon = m_error ? QChar(0xE628) : QChar(0xE9E4); // Use Material Icons for speed icon (error or normal).
    painter.drawText(textRect.adjusted(0, -rectHeight * 0.3, 0, 0), Qt::AlignCenter, speedIcon);

    painter.setFont(numFont);
    QString speedText = m_error ? QString("Connection Error") : (QString::number(m_speed) + " kph");
    painter.drawText(textRect.adjusted(0, rectHeight * 0.3, 0, 0), Qt::AlignCenter, speedText);
}

void Canvas::setBatteryLevel(int level)
{
    if (!m_error)
    {
        batteryLevel = level;
    }
    else
    {
        batteryLevel = 0;
    }
}

void Canvas::setTemperature(int temp)
{
    if (!m_error)
    {
        temperature = temp;
    }
    else
    {
        temperature = 0;
    }
}

void Canvas::setLeftSignal(bool state)
{
    if (!m_error)
    {
        leftSignal = state;
    }
    else
    {
        leftSignal = false;
    }
    update();
}

void Canvas::setRightSignal(bool state)
{
    if (!m_error)
    {
        rightSignal = state;
    }
    else
    {
        rightSignal = false;
    }
    update();
}

void Canvas::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Clear the canvas with a black background
    painter.fillRect(rect(), Qt::black);

    // Draw the energy icon
    drawEnergyIcon(painter, batteryLevel);

    // Draw the temperature icon
    drawTemperatureIcon(painter, temperature);
    // Draw the turn signals
    drawTurnSignal(painter, leftSignal, rightSignal);
    // Draw the speedometer
    drawSpeedometer(painter);
}

void Canvas::drawEnergyIcon(QPainter &painter, int batteryLevel)
{
    // Setup font and painter
    materialIconsFont.setPixelSize(145);
    painter.setFont(materialIconsFont);
    painter.setRenderHint(QPainter::Antialiasing);

    // Battery color based on level
    QColor color;
    if (batteryLevel < 25)
    {
        color = Qt::red;
    }
    else if (batteryLevel <= 49)
    {
        color = Qt::yellow;
    }
    else
    {
        color = Qt::green;
    }

    // Set the pen color to match the battery level
    painter.setPen(color);

    // Hardcoded rectangle position and size
    // Compute the battery bar so its bottom stays at y = 349.
    int batteryHeight = batteryLevel; // Battery level as pixel height.
    QRect rect(680, 345 - batteryHeight, 40, batteryHeight);
    painter.fillRect(rect, color);
    painter.drawRect(rect);

    // Draw the battery icon at the specified position
    QRect textRect(675, 230, 50, 145);
    painter.drawText(textRect, Qt::AlignCenter, QChar(0xebdc)); // Center the icon

    // Draw the battery level text below the icon
    QFont percentFont = painter.font();
    percentFont.setPixelSize(20);
    painter.setFont(percentFont);
    painter.setPen(Qt::white);

    // Calculate a rectangle just below the icon for the percentage text, moved up by 10 pixels
    QRect percentRect(textRect.left(), textRect.bottom() - 20, textRect.width(), 30);
    painter.drawText(percentRect, Qt::AlignCenter, QString::number(batteryLevel) + "%");
}

void Canvas::drawTemperatureIcon(QPainter &painter, int temperature)
{
    // Set the color based on temperature
    QColor color;
    if (temperature < 5)
    {
        color = Qt::white;
    }
    else if (temperature >= 5 && temperature <= 39)
    {
        color = Qt::blue;
    }
    else
    {
        color = Qt::red;
    }

    // Set the pen color
    painter.setPen(color);

    // Adjust font size for larger icon
    QFont font = painter.font();
    font.setPixelSize(100); // Increase font size significantly
    painter.setFont(font);

    // Hardcoded rectangle position and size for the temperature icon
    QRect textRect(600, 400, 200, 100);
    painter.drawText(textRect, Qt::AlignCenter, QChar(0xe1ff)); // Center the icon

    // Adjust font size for the temperature text
    QFont degreeFont = painter.font();
    degreeFont.setPixelSize(20); // Set font size for temperature text
    painter.setFont(degreeFont);
    painter.setPen(Qt::white);

    // Place the temperature text clearly below the temperature icon
    QRect degreesRect(textRect.left(), textRect.bottom() - 16, textRect.width(), 30);
    painter.drawText(degreesRect, Qt::AlignCenter, QString::number(temperature) + "°C");
}

void Canvas::drawTurnSignal(QPainter &painter, bool leftSignal, bool rightSignal)
{
    QColor signalColor = QColor(255, 165, 0); // Orange color for turn signals
    painter.setBrush(signalColor);

    QFont signalFont = materialIconsFont;
    signalFont.setPixelSize(125);
    painter.setFont(signalFont);

    if (leftSignal || rightSignal)
    {
        if (m_media.playbackState() != QMediaPlayer::PlayingState || m_media.mediaStatus() == QMediaPlayer::MediaStatus::EndOfMedia)
        {
            m_media.setSource(QUrl());
            m_media.setSource(QUrl::fromLocalFile("sound.wav"));
            m_media.play();
        }
    }
    else
    {
        if (m_media.playbackState() == QMediaPlayer::PlayingState)
        {
            m_media.pause();
        }
    }

    // Draw left turn signal in top-left corner
    if (leftSignal)
    {
        QColor color = blinkOn ? signalColor : Qt::transparent;
        painter.setPen(color);
        QRect textRect(20, 20, 150, 150);
        painter.drawText(textRect, Qt::AlignCenter, QChar(0xe5c4));
    }

    // Draw right turn signal in top-right corner
    if (rightSignal)
    {
        QColor color = blinkOn ? signalColor : Qt::transparent;
        painter.setPen(color);
        QRect textRect(width() - 170, 20, 150, 150);
        painter.drawText(textRect, Qt::AlignCenter, QChar(0xe5c8));
    }
}
