#include <gtest/gtest.h>
#include <rapidcsv.h>

#include <vector>
#include <string>
#include <limits>
#include <cmath>
#include <stdexcept>

static inline void ExpectNear(double value1, double value2, double eps = 1e-2) {
  ASSERT_TRUE(std::fabs(value1 - value2) <= eps) << "Expected " << value1 << " ~= " << value2;
}

static inline bool IsNaN(double value) {
  return std::isnan(value);
}

static rapidcsv::Document MakeDoc() {
  return rapidcsv::Document(
      "",                           
      rapidcsv::LabelParams(0, -1)  
  );
}
