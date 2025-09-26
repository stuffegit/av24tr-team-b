#ifndef WINDOW_H
#define WINDOW_H

#include "comservice.h"
#include <QDialog>
#include <QLabel>
#include <QSlider>
#include <QCheckBox>
#include <QGridLayout>
#include <QPushButton>

class Window : public QDialog
{
  COMService &comservice;
  QGridLayout grid;
  QHBoxLayout signal_checkboxes; // Contains all signal checkboxes

  QCheckBox left_signal;    // Left turning signal
  QCheckBox right_signal;   // Right turning signal
  QCheckBox warning_signal; // Warning signal

  // Info text boxes
  QLabel speed_label;
  QLabel temp_label;
  QLabel battery_label;
  QLabel signal_label;

  // Sliders which will communicate with client
  QSlider speed_slider;
  QSlider temp_slider;
  QSlider battery_slider;

  // Dynamic text boxes that reflect slider value
  QLabel current_speed_label;
  QLabel current_temp_label;
  QLabel current_battery_label;

public:
  Window(COMService &comservice);

private:
  int left_state = 0;
  int right_state = 0;
  // Add doxygen commenting
  void update_speed_slider_label(void);

  void update_temp_slider_label(void);

  void update_battery_slider_label(void);

  void disable_signal_box(void);

  void toggle_warning_signal(void);
};

#endif