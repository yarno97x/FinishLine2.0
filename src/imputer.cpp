#include "imputer.h"

rapidcsv::Document Imputer::mean(std::vector<std::string> columns);

rapidcsv::Document Imputer::median(std::vector<std::string> columns);

rapidcsv::Document Imputer::constant(std::vector<std::string> columns);

rapidcsv::Document Imputer::frequency(std::vector<std::string> columns);

Imputer::Imputer(rapidcsv::Document doc) : dataset(doc) {}
