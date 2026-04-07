#pragma once

#include "PIDControll.hpp"
class PositionTargetPID {
public:
  PositionTargetPID() : x_pid(0, 0, 0, 1000), y_pid(0, 0, 0, 1000) {};

private:
  PIDControl<double> x_pid;
  PIDControl<double> y_pid;
};
