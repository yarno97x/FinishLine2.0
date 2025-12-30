// test_scaler_fit_standardizer.cpp
#include <gtest/gtest.h>
#include <cmath>

#include "scaler.h"
#include "helpers.h"

// ---- Standardizer (population mean / population std) Tests ----

TEST(ScalerFitStandardizer, FitStandardizer_PopulationMeanStd) {
  auto doc = MakeDoc();

  // values: 1, 9, 3
  doc.SetColumn<std::string>(0, {"1.0", "9.0", "3.0"});
  doc.SetColumnName(0, "age");

  Scaler scaler(doc);
  scaler.fit_standardizer({"age"});

  // population mean
  const double mean = (1.0 + 9.0 + 3.0) / 3.0;

  // population variance = ((1-μ)^2 + (9-μ)^2 + (3-μ)^2) / 3
  const double variance =
      (std::pow(1.0 - mean, 2) +
       std::pow(9.0 - mean, 2) +
       std::pow(3.0 - mean, 2)) / 3.0;

  const double stddev = std::sqrt(variance);

  ExpectNear(scaler.transforms.at("age").parameters.at("mean"), mean);
  ExpectNear(scaler.transforms.at("age").parameters.at("std"), stddev);

  EXPECT_TRUE(scaler.fitted);
  EXPECT_EQ(scaler.transforms.at("age").transformType, STANDARDIZER);
  EXPECT_FALSE(scaler.transforms.at("age").clamp);
}

TEST(ScalerFitStandardizer, FitStandardizer_TwoColumns_PopulationStd) {
  auto doc = MakeDoc();

  doc.SetColumn<std::string>(0, {"1.0", "9.0", "3.0"});
  doc.SetColumnName(0, "age");

  doc.SetColumn<std::string>(1, {"10.0", "20.0", "15.0"});
  doc.SetColumnName(1, "height");

  Scaler scaler(doc);
  scaler.fit_standardizer({"age", "height"});

  // age
  const double age_mean = (1.0 + 9.0 + 3.0) / 3.0;
  const double age_var =
      (std::pow(1.0 - age_mean, 2) +
       std::pow(9.0 - age_mean, 2) +
       std::pow(3.0 - age_mean, 2)) / 3.0;

  ExpectNear(scaler.transforms.at("age").parameters.at("mean"), age_mean);
  ExpectNear(scaler.transforms.at("age").parameters.at("std"), std::sqrt(age_var));
  EXPECT_EQ(scaler.transforms.at("age").transformType, STANDARDIZER);

  // height
  const double h_mean = (10.0 + 20.0 + 15.0) / 3.0;
  const double h_var =
      (std::pow(10.0 - h_mean, 2) +
       std::pow(20.0 - h_mean, 2) +
       std::pow(15.0 - h_mean, 2)) / 3.0;

  ExpectNear(scaler.transforms.at("height").parameters.at("mean"), h_mean);
  ExpectNear(scaler.transforms.at("height").parameters.at("std"), std::sqrt(h_var));
  EXPECT_EQ(scaler.transforms.at("height").transformType, STANDARDIZER);

  EXPECT_TRUE(scaler.fitted);
}

TEST(ScalerFitStandardizer, FitStandardizer_ClearTrue_ReplacesTransforms) {
  auto doc = MakeDoc();

  doc.SetColumn<std::string>(0, {"1.0", "9.0", "3.0"});
  doc.SetColumnName(0, "age");

  doc.SetColumn<std::string>(1, {"10.0", "20.0", "15.0"});
  doc.SetColumnName(1, "height");

  Scaler scaler(doc);

  scaler.fit_standardizer({"age", "height"});
  ASSERT_EQ(scaler.transforms.size(), 2u);

  scaler.fit_standardizer({"age"}, /*clear=*/true);

  EXPECT_EQ(scaler.transforms.size(), 1u);
  EXPECT_TRUE(scaler.transforms.count("age"));
  EXPECT_FALSE(scaler.transforms.count("height"));
}

TEST(ScalerFitStandardizer, FitStandardizer_AllEqual_StdZero) {
  auto doc = MakeDoc();

  doc.SetColumn<std::string>(0, {"7.0", "7.0", "7.0"});
  doc.SetColumnName(0, "const_col");

  Scaler scaler(doc);
  scaler.fit_standardizer({"const_col"});

  ExpectNear(scaler.transforms.at("const_col").parameters.at("mean"), 7.0);
  ExpectNear(scaler.transforms.at("const_col").parameters.at("std"), 0.0);

  EXPECT_TRUE(scaler.fitted);
  EXPECT_EQ(scaler.transforms.at("const_col").transformType, STANDARDIZER);
}

TEST(ScalerFitStandardizer, FitStandardizer_ThrowsOnMissingColumn) {
  auto doc = MakeDoc();

  doc.SetColumn<std::string>(0, {"1.0", "2.0", "3.0"});
  doc.SetColumnName(0, "present");

  Scaler scaler(doc);

  EXPECT_THROW(
      scaler.fit_standardizer({"missing"}),
      std::exception
  );
}
