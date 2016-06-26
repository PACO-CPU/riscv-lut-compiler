/** \file deviations.h
  * \brief Defines metrics for computing errors in approximation.
  */

#ifndef RISCV_LUT_COMPULER_DEVIATION_H
#define RISCV_LUT_COMPULER_DEVIATION_H

#include "error.h"
#include "segment.h"
#include "target-types.h"
#include "util.h"
#include "bounds.h"
#include "weights.h"

#include <alpha/alpha.h>

/** Structure representing the mean deviation in a set of samples.
  *
  * Stores both the (weighted) mean and the accumulated weight / count of
  * data points so that multiple instances can be combined arithmetically.
  */
struct deviation_t {
  double mean;
  double weight;

  deviation_t() : mean(0), weight(0) { 

  }
  deviation_t(double mean, double weight) : mean(mean), weight(weight) { 

  }

  deviation_t operator+(const deviation_t &e) const {
    deviation_t r;
    r.weight=weight+e.weight;
    r.mean=((mean*weight)+(e.mean*e.weight))/r.weight;
    return r;
  }

  bool operator<(const deviation_t &e) const { return mean<e.mean; }
  bool operator>(const deviation_t &e) const { return mean>e.mean; }
  bool operator==(const deviation_t &e) const { return mean==e.mean; }
  bool operator!=(const deviation_t &e) const { return mean!=e.mean; }
  bool operator<=(const deviation_t &e) const { return mean<=e.mean; }
  bool operator>=(const deviation_t &e) const { return mean>=e.mean; }
};

/** Principal error metric, like absolute or square deviation
  */
typedef double (*error_metric_t)(
  uint32_t x, const seg_data_t &y_target, double y_approx);

/** Absolute deviation used for error metrics.
  */
double error_absolute(
  uint32_t x, const seg_data_t &y_target, double y_approx);

/** Square deviation used for error metrics.
  */
double error_square(
  uint32_t x, const seg_data_t &y_target, double y_approx);

#endif
