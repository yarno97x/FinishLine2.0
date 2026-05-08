#include "scaler.h"
#include "helpers.h"

#include <sstream>

TEST(ScalerFitStandard, StoresCenterAndScale)
{
  auto doc = MakeDoc();
  doc.SetColumn<std::string>(0, {"1.0", "2.0", "3.0", "4.0"});
  doc.SetColumnName(0, "x");

  Scaler scaler(doc);
  scaler.fit_standard({"x"});

  ASSERT_TRUE(scaler.fitted);
  ASSERT_TRUE(scaler.parameters.count("x") > 0);

  const auto mean = scaler.parameters["x"]["mean"];
  const auto std = scaler.parameters["x"]["std"];
  ExpectNear(mean, 2.5);
  ExpectNear(std, std::sqrt(1.25));
}

TEST(ScalerFitMinMax, ScalesRangeToZeroOne)
{
  auto doc = MakeDoc();
  doc.SetColumn<std::string>(0, {"10.0", "20.0", "30.0", "40.0"});
  doc.SetColumnName(0, "x");

  Scaler scaler(doc);
  scaler.fit_minmax({"x"});
  scaler.apply();

  const auto min = scaler.parameters["x"]["min"];
  const auto max = scaler.parameters["x"]["max"];
  ExpectNear(min, 10.0);
  ExpectNear(max, 40.0);

  ExpectNear(doc.GetCell<double>("x", 0), 0.0);
  ExpectNear(doc.GetCell<double>("x", 1), 1.0 / 3.0);
  ExpectNear(doc.GetCell<double>("x", 2), 2.0 / 3.0);
  ExpectNear(doc.GetCell<double>("x", 3), 1.0);
}

TEST(ScalerFitRobust, UsesMedianAndIQR)
{
  auto doc = MakeDoc();
  doc.SetColumn<std::string>(0, {"1.0", "2.0", "3.0", "100.0"});
  doc.SetColumnName(0, "x");

  Scaler scaler(doc);
  scaler.fit_robust({"x"});
  scaler.apply();

  const auto quartile1 = scaler.parameters["x"]["q1"];
  const auto median = scaler.parameters["x"]["median"];
  const auto quartile3 = scaler.parameters["x"]["q3"];
  ExpectNear(median, 2.5);
  ExpectNear(quartile1, 1.75);
  ExpectNear(quartile3, 27.25);

  // Median = 2.5, Q1 = 1.75, Q3 = 27.25, IQR = 25.5
  ExpectNear(doc.GetCell<double>("x", 0), (1.0 - 2.5) / 25.5, 1e-2);
  ExpectNear(doc.GetCell<double>("x", 1), (2.0 - 2.5) / 25.5, 1e-2);
  ExpectNear(doc.GetCell<double>("x", 2), (3.0 - 2.5) / 25.5, 1e-2);
  ExpectNear(doc.GetCell<double>("x", 3), (100.0 - 2.5) / 25.5, 1e-2);
}

TEST(ScalerTransform, LeavesMissingValuesUnchanged)
{
  auto doc = MakeDoc();
  doc.SetColumn<std::string>(0, {"1.0", "", "NaN", "4.0"});
  doc.SetColumnName(0, "x");

  Scaler scaler(doc);
  scaler.fit_minmax({"x"});
  scaler.apply();

  ExpectNear(doc.GetCell<double>("x", 0), 0.0);
  EXPECT_EQ(doc.GetCell<std::string>("x", 1), "");
  EXPECT_EQ(doc.GetCell<std::string>("x", 2), "NaN");
  ExpectNear(doc.GetCell<double>("x", 3), 1.0);
}

TEST(ScalerFitStandard, UnknownColumnThrows)
{
  auto doc = MakeDoc();
  doc.SetColumn<std::string>(0, {"1.0", "2.0"});
  doc.SetColumnName(0, "x");

  Scaler scaler(doc);
  EXPECT_THROW(scaler.fit_standard({"does_not_exist"}), std::invalid_argument);
}

TEST(ScalerFitMinMax, UnknownColumnThrows)
{
  auto doc = MakeDoc();
  doc.SetColumn<std::string>(0, {"1.0", "2.0"});
  doc.SetColumnName(0, "x");

  Scaler scaler(doc);
  EXPECT_THROW(scaler.fit_minmax({"does_not_exist"}), std::invalid_argument);
}

TEST(ScalerFitRobust, UnknownColumnThrows)
{
  auto doc = MakeDoc();
  doc.SetColumn<std::string>(0, {"1.0", "2.0"});
  doc.SetColumnName(0, "x");

  Scaler scaler(doc);
  EXPECT_THROW(scaler.fit_robust({"does_not_exist"}), std::invalid_argument);
}

TEST(ScalerFitStandard, AllMissingValuesThrow)
{
  auto doc = MakeDoc();
  doc.SetColumn<std::string>(0, {"", "NaN", ""});
  doc.SetColumnName(0, "x");

  Scaler scaler(doc);
  EXPECT_THROW(scaler.fit_standard({"x"}), std::invalid_argument);
}

TEST(ScalerFitMinMax, AllMissingValuesThrow)
{
  auto doc = MakeDoc();
  doc.SetColumn<std::string>(0, {"", "NaN", ""});
  doc.SetColumnName(0, "x");

  Scaler scaler(doc);
  EXPECT_THROW(scaler.fit_minmax({"x"}), std::invalid_argument);
}

TEST(ScalerFitStandard, FitsOnlySpecifiedColumns)
{
  auto doc = MakeDoc();
  doc.SetColumn<std::string>(0, {"1.0", "2.0", "3.0"});
  doc.SetColumn<std::string>(1, {"10.0", "20.0", "30.0"});
  doc.SetColumnName(0, "a");
  doc.SetColumnName(1, "b");

  Scaler scaler(doc);
  scaler.fit_standard({"a"});

  ASSERT_TRUE(scaler.parameters.count("a") > 0);
  ASSERT_EQ(scaler.parameters.count("b"), 0U);
}

TEST(ScalerFitMinMax, FitsTwoSpecifiedColumns)
{
  auto doc = MakeDoc();
  doc.SetColumn<std::string>(0, {"1.0", "2.0", "3.0"});
  doc.SetColumn<std::string>(1, {"10.0", "20.0", "30.0"});
  doc.SetColumnName(0, "a");
  doc.SetColumnName(1, "b");

  Scaler scaler(doc);
  scaler.fit_minmax({"a", "b"});

  ASSERT_TRUE(scaler.parameters.count("a") > 0);
  ASSERT_TRUE(scaler.parameters.count("b") > 0);
}

TEST(ScalerFitStandard, ClearReplacesOldParameters)
{
  auto doc = MakeDoc();
  doc.SetColumn<std::string>(0, {"1.0", "2.0", "3.0"});
  doc.SetColumn<std::string>(1, {"10.0", "20.0", "30.0"});
  doc.SetColumnName(0, "a");
  doc.SetColumnName(1, "b");

  Scaler scaler(doc);
  scaler.fit_standard({"a", "b"});
  ASSERT_EQ(scaler.parameters.size(), 2U);

  scaler.fit_standard({"a"}, true);
  ASSERT_EQ(scaler.parameters.size(), 1U);
  ASSERT_TRUE(scaler.parameters.count("a") > 0);
  ASSERT_EQ(scaler.parameters.count("b"), 0U);
}

TEST(ScalerFitMinMax, CheckFittedFlag)
{
  auto doc = MakeDoc();
  doc.SetColumn<std::string>(0, {"2.0", "4.0"});
  doc.SetColumnName(0, "x");

  Scaler scaler(doc);
  EXPECT_FALSE(scaler.fitted);

  scaler.fit_minmax({"x"});
  EXPECT_TRUE(scaler.fitted);
}
