#include <gtest/gtest.h>
#include <rapidcsv.h>

#include <vector>
#include <string>
#include <limits>
#include <cmath>
#include <stdexcept>

static inline void ExpectNear(double a, double b, double eps = 1e-2) {
  ASSERT_TRUE(std::fabs(a - b) <= eps) << "Expected " << a << " ~= " << b;
}

static inline bool IsNaN(double x) {
  return std::isnan(x);
}

static rapidcsv::Document MakeDoc() {
  return rapidcsv::Document(
      "",                           
      rapidcsv::LabelParams(0, -1)  
  );
}
