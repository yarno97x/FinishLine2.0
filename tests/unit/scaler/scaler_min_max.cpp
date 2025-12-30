// test_scaler_fit_minmax.cpp
#include <gtest/gtest.h>

#include "scaler.h"
#include "helpers.h"

// ---- MinMax Tests ----

TEST(ScalerFitMinMax, FitMinMax1_9_DefaultClampFalse) {
  auto doc = MakeDoc();

  doc.SetColumn<std::string>(0, {"1.0", "9.0", "3.0"});
  doc.SetColumnName(0, "age");

  Scaler scaler(doc);
  scaler.fit_min_max({"age"});

  ExpectNear(scaler.transforms.at("age").parameters.at("min"), 1.0);
  ExpectNear(scaler.transforms.at("age").parameters.at("max"), 9.0);
  EXPECT_TRUE(scaler.fitted);
  EXPECT_EQ(scaler.transforms.at("age").transformType, MINMAX);
  EXPECT_FALSE(scaler.transforms.at("age").clamp);
}

TEST(ScalerFitMinMax, FitMinMax_ClampTrue_WhenPassed) {
  auto doc = MakeDoc();

  doc.SetColumn<std::string>(0, {"-5.0", "0.0", "10.0"});
  doc.SetColumnName(0, "x");

  Scaler scaler(doc);
  scaler.fit_min_max({"x"}, /*clear=*/false, /*clamp=*/true);

  ExpectNear(scaler.transforms.at("x").parameters.at("min"), -5.0);
  ExpectNear(scaler.transforms.at("x").parameters.at("max"), 10.0);
  EXPECT_TRUE(scaler.fitted);
  EXPECT_EQ(scaler.transforms.at("x").transformType, MINMAX);
  EXPECT_TRUE(scaler.transforms.at("x").clamp);
}

TEST(ScalerFitMinMax, FitMinMax_AllEqual_MinEqualsMax) {
  auto doc = MakeDoc();

  doc.SetColumn<std::string>(0, {"7.0", "7.0", "7.0"});
  doc.SetColumnName(0, "const_col");

  Scaler scaler(doc);
  scaler.fit_min_max({"const_col"});

  ExpectNear(scaler.transforms.at("const_col").parameters.at("min"), 7.0);
  ExpectNear(scaler.transforms.at("const_col").parameters.at("max"), 7.0);
  EXPECT_TRUE(scaler.fitted);
  EXPECT_EQ(scaler.transforms.at("const_col").transformType, MINMAX);
}

TEST(ScalerFitMinMax, FitMinMax_TwoColumns) {
  auto doc = MakeDoc();

  doc.SetColumn<std::string>(0, {"1.0", "9.0", "3.0"});
  doc.SetColumnName(0, "age");

  doc.SetColumn<std::string>(1, {"10.0", "20.0", "15.0"});
  doc.SetColumnName(1, "height");

  Scaler scaler(doc);
  scaler.fit_min_max({"age", "height"});

  // age
  ExpectNear(scaler.transforms.at("age").parameters.at("min"), 1.0);
  ExpectNear(scaler.transforms.at("age").parameters.at("max"), 9.0);
  EXPECT_EQ(scaler.transforms.at("age").transformType, MINMAX);

  // height
  ExpectNear(scaler.transforms.at("height").parameters.at("min"), 10.0);
  ExpectNear(scaler.transforms.at("height").parameters.at("max"), 20.0);
  EXPECT_EQ(scaler.transforms.at("height").transformType, MINMAX);

  EXPECT_TRUE(scaler.fitted);
}

TEST(ScalerFitMinMax, FitMinMax_ClearTrue_ReplacesExistingTransforms) {
  auto doc = MakeDoc();

  doc.SetColumn<std::string>(0, {"1.0", "9.0", "3.0"});
  doc.SetColumnName(0, "age");

  doc.SetColumn<std::string>(1, {"10.0", "20.0", "15.0"});
  doc.SetColumnName(1, "height");

  Scaler scaler(doc);

  // First fit both columns
  scaler.fit_min_max({"age", "height"});
  ASSERT_EQ(scaler.transforms.size(), 2u);

  // Now clear and fit only "age"
  scaler.fit_min_max({"age"}, /*clear=*/true, /*clamp=*/false);

  EXPECT_EQ(scaler.transforms.size(), 1u);
  EXPECT_TRUE(scaler.transforms.find("age") != scaler.transforms.end());
  EXPECT_TRUE(scaler.transforms.find("height") == scaler.transforms.end());

  ExpectNear(scaler.transforms.at("age").parameters.at("min"), 1.0);
  ExpectNear(scaler.transforms.at("age").parameters.at("max"), 9.0);
  EXPECT_TRUE(scaler.fitted);
}

TEST(ScalerFitMinMax, FitMinMax_DoesNotClear_WhenClearFalse) {
  auto doc = MakeDoc();

  doc.SetColumn<std::string>(0, {"1.0", "9.0", "3.0"});
  doc.SetColumnName(0, "age");

  doc.SetColumn<std::string>(1, {"10.0", "20.0", "15.0"});
  doc.SetColumnName(1, "height");

  Scaler scaler(doc);

  scaler.fit_min_max({"age"});
  ASSERT_EQ(scaler.transforms.size(), 1u);

  scaler.fit_min_max({"height"}, /*clear=*/false);
  EXPECT_EQ(scaler.transforms.size(), 2u);

  ExpectNear(scaler.transforms.at("age").parameters.at("min"), 1.0);
  ExpectNear(scaler.transforms.at("age").parameters.at("max"), 9.0);

  ExpectNear(scaler.transforms.at("height").parameters.at("min"), 10.0);
  ExpectNear(scaler.transforms.at("height").parameters.at("max"), 20.0);

  EXPECT_TRUE(scaler.fitted);
}

TEST(ScalerFitMinMax, FitMinMax_ThrowsOnMissingColumn) {
  auto doc = MakeDoc();

  doc.SetColumn<std::string>(0, {"1.0", "2.0", "3.0"});
  doc.SetColumnName(0, "present");

  Scaler scaler(doc);

  EXPECT_THROW(
      scaler.fit_min_max({"missing_column_name"}),
      std::exception
  );
}

