#pragma once
#include <algorithm>

template <typename T> class PIDControl {
public:
  PIDControl(double P_gain, double I_gain, double D_gain, double max_integral) {
    this->P_gain = P_gain;
    this->I_gain = I_gain;
    this->D_gain = D_gain;
    this->max_integral = max_integral;
  };

  void update(T measured, unsigned int delta) {
    double d_delta = static_cast<double>(delta);
    double error = this->target - static_cast<double>(measured);
    double p = (error) * this->P_gain;

    this->integral += (error * d_delta);
    this->integral = std::clamp(this->integral, -max_integral, max_integral);
    double i = this->integral * this->I_gain;

    double d = ((error - this->prev_error) / d_delta) * this->D_gain;
    this->prev_error = error;

    this->current = p + i + d;
  };

  T get() { return static_cast<T>(this->current); }

  void set_target(T new_target) {
    this->target = static_cast<double>(new_target);
  }

  void set_gain(double P_gain, double I_gain, double D_gain) {
    this->P_gain = P_gain;
    this->I_gain = I_gain;
    this->D_gain = D_gain;
  }

private:
  double current = 0;
  double target = 0;

  double prev_error = 0;
  double integral = 0;

  double max_integral;
  double P_gain;
  double I_gain;
  double D_gain;
  unsigned int interval;
};
