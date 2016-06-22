#include "deviation.h"
#include <math.h>

double error_absolute(
  uint32_t x, const seg_data_t &y_target, double y_approx) {
  double e=fabs(((double)y_target)-y_approx);
  return e;
}

double error_square(
  uint32_t x, const seg_data_t &y_target, double y_approx) {
  double e=((double)y_target)-y_approx;
  return e*e;
}

