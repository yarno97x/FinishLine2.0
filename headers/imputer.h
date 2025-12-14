#include <vector>
#include <string>
#include <rapidcsv.h>
#include <algorithm>
#include <stdexcept>

enum class TRANSFORM 
{
    MEAN,
    MEDIAN,
    FREQUENCY,
    CONSTANT,
    NOT_FITTED
};

struct ColumnChange 
{
    std::string columnName;
    std::string value;
};

class Imputer {
    public:
        Imputer(rapidcsv::Document& doc);

        void fit_mean(const std::vector<std::string>& columns);
        void fit_median(const std::vector<std::string>& columns);
        void fit_frequency(const std::vector<std::string>& columns); 
        void constant(const std::vector<std::string>& columns, const std::string value);
        void applyTransform(const rapidcsv::Document& doc);
        std::vector<ColumnChange> parameters{};

    private:
        rapidcsv::Document& dataset;
        TRANSFORM mode = TRANSFORM::NOT_FITTED;
        std::vector<std::string> features{};

};
