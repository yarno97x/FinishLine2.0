// test_imputer_fit_frequency.cpp

#include "imputer.h"
#include "helpers.h"

// ---- Tests ----

TEST(ImputerFitFrequency, OddCount) {
  auto doc = MakeDoc();

  doc.SetColumn<std::string>(0, {"1.0", "9.0", "3.0"}); 
  doc.SetColumnName(0, "age");

  Imputer imp(doc);
  imp.fit_frequency({"age"});

  ExpectNear(std::stod(imp.parameters["age"]), 1.0);
}

TEST(ImputerFitFrequency, EvenCountAveragesMiddleTwo) {
  auto doc = MakeDoc();

  doc.SetColumn<std::string>(0, {"10.0", "2.0", "8.0", "2.0"}); 
  doc.SetColumnName(0, "x");

  Imputer imp(doc);
  imp.fit_frequency({"x"});

  ExpectNear(std::stod(imp.parameters["x"]), 2.0);
}

TEST(ImputerFitFrequency, IgnoresNaNMissingValues) {
  auto doc = MakeDoc();

  doc.SetColumn<std::string>(0, {"1.0", "NaN", "9.0", "3.0", "1.0"});
  doc.SetColumnName(0, "score");

  Imputer imp(doc);
  imp.fit_frequency({"score"});

  ExpectNear(std::stod(imp.parameters["score"]), 1.0);
}

TEST(ImputerFitFrequency, IgnoresEmptyValues) {
  auto doc = MakeDoc();

  doc.SetColumn<std::string>(0, {"1.0", "", "9.0", "3.0", "9.0"});
  doc.SetColumnName(0, "score");

  Imputer imp(doc);
  imp.fit_frequency({"score"});

  ExpectNear(std::stod(imp.parameters["score"]), 9.0);
}

TEST(ImputerFitFrequency, UnknownColumnThrows) {
  auto doc = MakeDoc();

  doc.SetColumn<std::string>(0, {"1.0", "2.0", "3.0"});
  doc.SetColumnName(0, "a");

  Imputer imp(doc);

  EXPECT_THROW(imp.fit_frequency({"does_not_exist"}), std::invalid_argument);
}

TEST(ImputerFitFrequency, AllMissingColumnThrowsOrNaN) {
  auto doc = MakeDoc();

  doc.SetColumn<std::string>(0, {"NaN", "", "NaN"});
  doc.SetColumnName(0, "z");

  Imputer imp(doc);

  EXPECT_THROW(imp.fit_frequency({"z"}), std::invalid_argument);
}

TEST(ImputerFitFrequency, FitsOnlySpecifiedColumns) {
  auto doc = MakeDoc();

  doc.SetColumn<std::string>(0, {"1.0", "2.0", "2.0", "1.0"});     
  doc.SetColumn<std::string>(1, {"10.0", "20.0", "30.0", "1.0"});  
  doc.SetColumnName(0, "a");
  doc.SetColumnName(1, "b");

  Imputer imp(doc);
  imp.fit_frequency({"a"});

  ExpectNear(std::stod(imp.parameters["a"]), 2.0);

  EXPECT_THROW(std::stod(imp.parameters["b"]), std::invalid_argument);
}

TEST(ImputerFitFrequency, CheckNotFitted) {
  auto doc = MakeDoc();

  doc.SetColumn<std::string>(0, {"NaN", "NaN", "NaN"});
  doc.SetColumnName(0, "z");

  Imputer imp(doc);

  EXPECT_FALSE(imp.fitted);
}

TEST(ImputerFitFrequency, CheckFitted) {
  auto doc = MakeDoc();

  doc.SetColumn<std::string>(0, {"1.0", "NaN", "9.0", "3.0"});
  doc.SetColumnName(0, "score");

  Imputer imp(doc);
  imp.fit_frequency({"score"});

  EXPECT_TRUE(imp.fitted);
}
