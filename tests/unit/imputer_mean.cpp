// test_imputer_fit_mean.cpp

#include "imputer.h"
#include "helpers.h"

// ---- Tests ----

TEST(ImputerFitMean, OddCount) {
  auto doc = MakeDoc();

  doc.SetColumn<std::string>(0, {"1.0", "9.0", "3.0"}); 
  doc.SetColumnName(0, "age");

  Imputer imp(doc);
  imp.fit_mean({"age"});

  ExpectNear(std::stod(imp.parameters["age"]), 13.0/3);
}

TEST(ImputerFitMean, EvenCountAveragesMiddleTwo) {
  auto doc = MakeDoc();

  doc.SetColumn<std::string>(0, {"10.0", "2.0", "8.0", "4.0"}); 
  doc.SetColumnName(0, "x");

  Imputer imp(doc);
  imp.fit_mean({"x"});

  ExpectNear(std::stod(imp.parameters["x"]), 6.0);
}

TEST(ImputerFitMean, IgnoresNaNMissingValues) {
  auto doc = MakeDoc();

  doc.SetColumn<std::string>(0, {"1.0", "NaN", "9.0", "3.0"});
  doc.SetColumnName(0, "score");

  Imputer imp(doc);
  imp.fit_mean({"score"});

  ExpectNear(std::stod(imp.parameters["score"]), 13.0/3);
}

TEST(ImputerFitMean, IgnoresEmptyValues) {
  auto doc = MakeDoc();

  doc.SetColumn<std::string>(0, {"1.0", "", "9.0", "3.0"});
  doc.SetColumnName(0, "score");

  Imputer imp(doc);
  imp.fit_mean({"score"});

  ExpectNear(std::stod(imp.parameters["score"]), 13.0/3);
}

TEST(ImputerFitMean, UnknownColumnThrows) {
  auto doc = MakeDoc();

  doc.SetColumn<std::string>(0, {"1.0", "2.0", "3.0"});
  doc.SetColumnName(0, "a");

  Imputer imp(doc);

  EXPECT_THROW(imp.fit_mean({"does_not_exist"}), std::invalid_argument);
}

TEST(ImputerFitMean, AllMissingColumnThrowsOrNaN) {
  auto doc = MakeDoc();
  
  doc.SetColumn<std::string>(0, {"NaN", "NaN", "NaN"});
  doc.SetColumnName(0, "z");

  Imputer imp(doc);

  EXPECT_THROW(imp.fit_mean({"z"}), std::invalid_argument);
}

TEST(ImputerFitMean, FitsOnlySpecifiedColumns) {
  auto doc = MakeDoc();

  doc.SetColumn<std::string>(0, {"1.0", "2.0", "3.0"});     
  doc.SetColumn<std::string>(1, {"10.0", "20.0", "30.0"});  
  doc.SetColumnName(0, "a");
  doc.SetColumnName(1, "b");

  Imputer imp(doc);
  imp.fit_mean({"a"});

  ExpectNear(std::stod(imp.parameters["a"]), 2.0);

  EXPECT_THROW(std::stod(imp.parameters["b"]), std::invalid_argument);
}

TEST(ImputerFitMean, CheckNotFitted) {
  auto doc = MakeDoc();

  doc.SetColumn<std::string>(0, {"NaN", "NaN", "NaN"});
  doc.SetColumnName(0, "z");

  Imputer imp(doc);

  EXPECT_FALSE(imp.fitted);
}

TEST(ImputerFitMean, CheckFitted) {
  auto doc = MakeDoc();

  doc.SetColumn<std::string>(0, {"1.0", "NaN", "9.0", "3.0"});
  doc.SetColumnName(0, "score");

  Imputer imp(doc);
  imp.fit_mean({"score"});

  EXPECT_TRUE(imp.fitted);
}
