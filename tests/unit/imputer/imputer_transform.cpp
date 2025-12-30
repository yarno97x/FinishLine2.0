// test_imputer_transform.cpp

#include "imputer.h"
#include "helpers.h"

// ---- Tests ----

TEST(ImputerTransformTest, CheckAllImputed) {
  auto doc = MakeDoc();

  doc.SetColumn<std::string>(0, {"1.0", "2.0", "2.0", ""});     
  doc.SetColumn<std::string>(1, {"10.0", "", "30.0", "1.0"});  
  doc.SetColumn<std::string>(2, {"10.0", "20.0", "", "2.0"}); 
  doc.SetColumn<std::string>(3, {"10.0", "20.0", "", "2.0"}); 
  doc.SetColumnName(0, "a");
  doc.SetColumnName(1, "b");
  doc.SetColumnName(2, "o");
  doc.SetColumnName(3, "c");

  std::map<std::string, int> counts;

  Imputer imp(doc);
  imp.fit_mean( {"a"} );
  imp.fit_median( {"b"} );
  imp.fit_constant( {{"o", "3.67"}} );
  imp.fit_frequency( {"c"} );
  imp.applyImputerTransform();

  ExpectNear(doc.GetCell<double>("a", 3), 5.0/3);
  ExpectNear(doc.GetCell<double>("b", 1), 10.0);
  ExpectNear(doc.GetCell<double>("o", 2), 3.67);
  ExpectNear(doc.GetCell<double>("c", 2), 10.0);
}
