#include <vector>
#include <string_view>
#include <rapidcsv.h>
#include <algorithm>
#include <stdexcept>

enum TRANSFORM 
{
    MEAN,
    MEDIAN,
    FREQUENCY,
    CONSTANT,
    NOT_FITTED
}

struct ColumnChange 
{
    std::string columnName;
    std::string value;
}

class Imputer {
    public:
        Imputer(rapidcsv::Document& doc);

        void fit_mean(const std::vector<std::string_view>& columns);
        void fit_median(const std::vector<std::string_view>& columns);
        void fit_frequency(const std::vector<std::string_view>& columns); 
        void constant(const std::vector<std::string_view>& columns, const std::string_view value);
        void applyTransform(const rapidcsv::Document& doc);

    private:
        rapidcsv::Document& dataset;
        TRANSFORM mode = TRANSFORM::NOT_FITTED;
        std::vector<std::string_view> features{};
        std::vector<std::string> parameters{};

};
