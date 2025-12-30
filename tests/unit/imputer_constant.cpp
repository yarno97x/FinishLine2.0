// test_imputer_fit_constabt.cpp

#include "imputer.h"
#include "helpers.h"

// ---- Tests ----

TEST(ImputerFitConstant, OddCount) {
  auto doc = MakeDoc();

  doc.SetColumn<std::string>(0, {"1.0", "9.0", "3.0"}); 
  doc.SetColumnName(0, "age");

  Imputer imp(doc);
  imp.fit_constant({ {"age", "1.0"} });

  ExpectNear(std::stod(imp.parameters["age"]), 1.0);
}

TEST(ImputerFitConstant, IgnoresNaNMissingValues) {
  auto doc = MakeDoc();

  doc.SetColumn<std::string>(0, {"1.0", "NaN", "9.0", "3.0", "1.0"});
  doc.SetColumnName(0, "score");

  Imputer imp(doc);
  imp.fit_constant({ {"score", "-9.0"} });

  ExpectNear(std::stod(imp.parameters["score"]), -9.0);
}

TEST(ImputerFitConstant, IgnoresEmptyValues) {
  auto doc = MakeDoc();

  doc.SetColumn<std::string>(0, {"1.0", "", "9.0", "3.0", "9.0"});
  doc.SetColumnName(0, "score");

  Imputer imp(doc);
  imp.fit_constant({ {"score", "-9.0"} });

  ExpectNear(std::stod(imp.parameters["score"]), -9.0);
}

TEST(ImputerFitConstant, UnknownColumnThrows) {
  auto doc = MakeDoc();

  doc.SetColumn<std::string>(0, {"1.0", "2.0", "3.0"});
  doc.SetColumnName(0, "a");

  Imputer imp(doc);

  EXPECT_THROW(imp.fit_constant({ {"does_not_exist", "0"} }), std::invalid_argument);
}

TEST(ImputerFitConstant, FitsOnlySpecifiedColumns) {
  auto doc = MakeDoc();

  doc.SetColumn<std::string>(0, {"1.0", "2.0", "2.0", "1.0"});     
  doc.SetColumn<std::string>(1, {"10.0", "20.0", "30.0", "1.0"});  
  doc.SetColumnName(0, "a");
  doc.SetColumnName(1, "b");

  Imputer imp(doc);
  imp.fit_constant({ {"a", "8.0"} });

  ExpectNear(std::stod(imp.parameters["a"]), 8.0);

  EXPECT_THROW(std::stod(imp.parameters["b"]), std::invalid_argument);
}

TEST(ImputerFitConstant, FitsTwoSpecifiedColumns) {
  auto doc = MakeDoc();

  doc.SetColumn<std::string>(0, {"1.0", "2.0", "2.0", "1.0"});     
  doc.SetColumn<std::string>(1, {"10.0", "20.0", "30.0", "1.0"});  
  doc.SetColumn<std::string>(1, {"10.0", "20.0", "30.0", "2.0"}); 
  doc.SetColumnName(0, "a");
  doc.SetColumnName(1, "b");
  doc.SetColumnName(2, "o");

  std::map<std::string, int> counts;

  Imputer imp(doc);
  imp.fit_constant({ {"a", "3"}, {"b", "5"} });

  ExpectNear(std::stod(imp.parameters["a"]), 3.0);
  ExpectNear(std::stod(imp.parameters["b"]), 5.0);

  EXPECT_THROW(std::stod(imp.parameters["o"]), std::invalid_argument);
}

TEST(ImputerFitConstant, CheckNotFitted) {
  auto doc = MakeDoc();

  doc.SetColumn<std::string>(0, {"NaN", "NaN", "NaN"});
  doc.SetColumnName(0, "z");

  Imputer imp(doc);

  EXPECT_FALSE(imp.fitted);
}

TEST(ImputerFitConstant, CheckFitted) {
  auto doc = MakeDoc();

  doc.SetColumn<std::string>(0, {"1.0", "NaN", "9.0", "3.0"});
  doc.SetColumnName(0, "score");

  Imputer imp(doc);
  imp.fit_constant({{"score", "0"}});

  EXPECT_TRUE(imp.fitted);
}
