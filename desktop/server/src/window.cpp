#include "window.h"

Window::Window(COMService &comservice) : comservice(comservice)
{
  setFixedWidth(800);
  setFixedHeight(150);
  setWindowFlags(Qt::WindowStaysOnTopHint);

  Setting::Signal &signal{Setting::Signal::handle()};

  setStyleSheet("QLabel, QCheckBox {font:12pt; }");

  // First row
  grid.addWidget(&speed_label, 0, 0, 1, 1);
  speed_label.setText("Speed:");
  speed_label.setAlignment(Qt::AlignRight);
  speed_label.setFixedWidth(140);

  grid.addWidget(&speed_slider, 0, 1, 1, 1);
  speed_slider.setMinimum(signal["speed"].min);
  speed_slider.setMaximum(signal["speed"].max);
  speed_slider.setValue(speed_slider.minimum());
  speed_slider.setOrientation(Qt::Horizontal);
  connect(&speed_slider, &QSlider::valueChanged, this,
          &Window::update_speed_slider_label);

  grid.addWidget(&current_speed_label, 0, 2, 1, 1);
  current_speed_label.setNum(signal["speed"].min);
  current_speed_label.setText(current_speed_label.text() + " Kph");
  current_speed_label.setFixedWidth(75);

  // Second row
  grid.addWidget(&temp_label, 1, 0, 1, 1);
  temp_label.setText("Temperature:");
  temp_label.setAlignment(Qt::AlignRight);
  temp_label.setFixedWidth(140);

  grid.addWidget(&temp_slider, 1, 1, 1, 1);
  temp_slider.setMinimum(signal["temperature"].min);
  temp_slider.setMaximum(signal["temperature"].max);
  temp_slider.setValue(0);
  temp_slider.setOrientation(Qt::Horizontal);
  connect(&temp_slider, &QSlider::valueChanged, this,
          &Window::update_temp_slider_label);

  grid.addWidget(&current_temp_label, 1, 2, 1, 1);
  current_temp_label.setNum(0);
  current_temp_label.setText(current_temp_label.text() + " °C");
  current_temp_label.setFixedWidth(75);

  // Third row
  grid.addWidget(&battery_label, 2, 0, 1, 1);
  battery_label.setText("Battery Level:");
  battery_label.setAlignment(Qt::AlignRight);
  battery_label.setFixedWidth(140);

  grid.addWidget(&battery_slider, 2, 1, 1, 1);
  battery_slider.setMinimum(signal["battery_level"].min);
  battery_slider.setMaximum(signal["battery_level"].max);
  battery_slider.setValue(battery_slider.minimum());
  battery_slider.setOrientation(Qt::Horizontal);
  connect(&battery_slider, &QSlider::valueChanged, this,
          &Window::update_battery_slider_label);

  grid.addWidget(&current_battery_label, 2, 2, 1, 1);
  current_battery_label.setNum(signal["battery_level"].min);
  current_battery_label.setText(current_battery_label.text() + " %");
  current_battery_label.setFixedWidth(75);

  // Fourth row
  grid.addWidget(&signal_label, 3, 0, 1, 1);
  signal_label.setText("Light Signals:");
  signal_label.setAlignment(Qt::AlignRight);
  signal_label.setFixedWidth(140);

  left_signal.setText("Left");
  connect(&left_signal, &QCheckBox::clicked, this,
          &Window::disable_signal_box);
  signal_checkboxes.addWidget(&left_signal);

  right_signal.setText("Right");
  connect(&right_signal, &QCheckBox::clicked, this,
          &Window::disable_signal_box);
  signal_checkboxes.addWidget(&right_signal);

  warning_signal.setText("Warning");
  connect(&warning_signal, &QCheckBox::clicked, this,
          &Window::toggle_warning_signal);
  signal_checkboxes.addWidget(&warning_signal);

  grid.addLayout(&signal_checkboxes, 3, 1, 1, 1);

  setLayout(&grid);
  setWindowTitle("Server");
}

void Window::update_speed_slider_label(void)
{
  current_speed_label.setNum(speed_slider.value());
  current_speed_label.setText(current_speed_label.text() + " Kph");
  comservice.setSpeed(speed_slider.value());
}

void Window::update_temp_slider_label(void)
{
  current_temp_label.setNum(temp_slider.value());
  current_temp_label.setText(current_temp_label.text() + " °C");
  comservice.setTemperature(temp_slider.value());
}

void Window::update_battery_slider_label(void)
{
  current_battery_label.setNum(battery_slider.value());
  current_battery_label.setText(current_battery_label.text() + "%");
  comservice.setBatteryLevel(battery_slider.value());
}

void Window::disable_signal_box(void)
{
  // Reset left_state and right_state at the start of the function
  left_state = 0;
  right_state = 0;

  int warning_state = warning_signal.isChecked();

  if (warning_state == 1)
  {
    if (left_signal.isChecked())
    {
      left_state = 1;
      left_signal.setDisabled(0);
      right_signal.setDisabled(1);
    }
    else if (right_signal.isChecked())
    {
      right_state = 1;
      right_signal.setDisabled(0);
      left_signal.setDisabled(1);
    }
    else
    {
      right_signal.setDisabled(0);
      left_signal.setDisabled(0);
    }
  }

  else
  {
    if (left_signal.isChecked())
    {
      right_signal.setDisabled(1);
      comservice.setRightLight(0);
      left_signal.setDisabled(0);
      comservice.setLeftLight(1);
    }
    else if (right_signal.isChecked())
    {
      right_signal.setDisabled(0);
      comservice.setRightLight(1);
      left_signal.setDisabled(1);
      comservice.setLeftLight(0);
    }
    else
    {
      right_signal.setDisabled(0);
      comservice.setRightLight(0);
      left_signal.setDisabled(0);
      comservice.setLeftLight(0);
    }
  }
}

void Window::toggle_warning_signal(void)
{

  int previous_state_left = left_signal.isChecked();
  int previous_state_right = right_signal.isChecked();

  if (warning_signal.isChecked())
  {
    comservice.setLeftLight(1);
    comservice.setRightLight(1);
  }
  else
  {
    if (previous_state_left == 1)
    {
      comservice.setLeftLight(1);
      comservice.setRightLight(0);
    }

    else if (previous_state_right == 1)
    {
      comservice.setRightLight(1);
      comservice.setLeftLight(0);
    }
    else
    {
      if (left_state == 1)
      {
        comservice.setLeftLight(1);
        comservice.setRightLight(0);
        right_signal.setDisabled(1);
      }

      else if (right_state == 1)
      {
        comservice.setLeftLight(0);
        comservice.setRightLight(1);
        left_signal.setDisabled(1);
      }
      else
      {
        comservice.setLeftLight(0);
        comservice.setRightLight(0);
      }
    }
  }
}