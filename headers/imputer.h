#include <vector>
#include <string>
#include <rapidcsv.h>
#include <algorithm>
#include <stdexcept>
#include <queue>

class Imputer {
    public:
        Imputer(rapidcsv::Document& doc);

        void fit_mean(const std::vector<std::string>& columns, bool clear = false);
        void fit_median(const std::vector<std::string>& columns, bool clear = false);
        void fit_frequency(const std::vector<std::string>& columns, bool clear = false); 
        void constant(const std::vector<std::string>& columns, const std::string value, bool clear = false);
        void applyTransform();
        std::map<std::string, std::string> parameters{};

    private:
        rapidcsv::Document& dataset;
        bool fitted = false;
        std::vector<std::string> features{};
};
