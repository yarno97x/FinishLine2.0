// test_imputer_fit_median.cpp

#include "imputer.h"
#include "helpers.h"

// ---- Tests ----

TEST(ImputerFitMedian, OddCount) {
  auto doc = MakeDoc();
  doc.SetColumn<std::string>(0, {"1.0", "9.0", "3.0"}); 
  doc.SetColumnName(0, "age");

  Imputer imp(doc);
  imp.fit_median({"age"});

  ExpectNear(std::stod(imp.parameters["age"]), 3.0);
}

TEST(ImputerFitMedian, EvenCountAveragesMiddleTwo) {
  auto doc = MakeDoc();
  doc.SetColumn<std::string>(0, {"10.0", "2.0", "8.0", "4.0"}); 
  doc.SetColumnName(0, "x");

  Imputer imp(doc);
  imp.fit_median({"x"});

  ExpectNear(std::stod(imp.parameters["x"]), 6.0);
}

TEST(ImputerFitMedian, IgnoresNaNMissingValues) {
  auto doc = MakeDoc();

  doc.SetColumn<std::string>(0, {"1.0", "NaN", "9.0", "3.0"});
  doc.SetColumnName(0, "score");

  Imputer imp(doc);
  imp.fit_median({"score"});

  ExpectNear(std::stod(imp.parameters["score"]), 3.0);
}

TEST(ImputerFitMedian, IgnoresEmptyValues) {
  auto doc = MakeDoc();

  doc.SetColumn<std::string>(0, {"1.0", "", "9.0", "3.0"});
  doc.SetColumnName(0, "score");

  Imputer imp(doc);
  imp.fit_median({"score"});

  ExpectNear(std::stod(imp.parameters["score"]), 3.0);
}

TEST(ImputerFitMedian, UnknownColumnThrows) {
  auto doc = MakeDoc();

  doc.SetColumn<std::string>(0, {"1.0", "2.0", "3.0"});
  doc.SetColumnName(0, "a");

  Imputer imp(doc);

  EXPECT_THROW(imp.fit_median({"does_not_exist"}), std::invalid_argument);
}

TEST(ImputerFitMedian, AllMissingColumnThrowsOrNaN) {
  auto doc = MakeDoc();

  doc.SetColumn<std::string>(0, {"", "NaN", "NaN"});
  doc.SetColumnName(0, "z");

  Imputer imp(doc);

  EXPECT_THROW(imp.fit_median({"z"}), std::invalid_argument);
}

TEST(ImputerFitMedian, FitsOnlySpecifiedColumns) {
  auto doc = MakeDoc();
  doc.SetColumn<std::string>(0, {"1.0", "2.0", "3.0"});     // mean 2
  doc.SetColumn<std::string>(1, {"10.0", "20.0", "30.0"});  // mean 20
  doc.SetColumnName(0, "a");
  doc.SetColumnName(1, "b");

  Imputer imp(doc);
  imp.fit_median({"a"});

  ExpectNear(std::stod(imp.parameters["a"]), 2.0);

  EXPECT_THROW(std::stod(imp.parameters["b"]), std::invalid_argument);
}

TEST(ImputerFitMedian, CheckNotFitted) {
  auto doc = MakeDoc();

  doc.SetColumn<std::string>(0, {"NaN", "NaN", "NaN"});
  doc.SetColumnName(0, "z");

  Imputer imp(doc);

  EXPECT_FALSE(imp.fitted);
}

TEST(ImputerFitMedian, CheckFitted) {
  auto doc = MakeDoc();

  doc.SetColumn<std::string>(0, {"1.0", "NaN", "9.0", "3.0"});
  doc.SetColumnName(0, "score");

  Imputer imp(doc);
  imp.fit_median({"score"});

  EXPECT_TRUE(imp.fitted);
}
