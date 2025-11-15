#include <vector>
#include <string>
#include <rapidcsv.h>

class Imputer {
    public:
        Imputer(rapidcsv::Document doc);

        rapidcsv::Document mean(std::vector<std::string> columns);
        rapidcsv::Document median(std::vector<std::string> columns);
        rapidcsv::Document constant(std::vector<std::string> columns, std::string value = "");
        rapidcsv::Document frequency(std::vector<std::string> columns); 

    private:
        rapidcsv::Document dataset;
};
