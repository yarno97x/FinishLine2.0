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
        void fit_constant(const std::map<std::string, std::string>& replacements, bool clear = false);
        void applyImputerTransform();
        std::map<std::string, std::string> parameters{};
        bool fitted = false;

    private:
        rapidcsv::Document& dataset;
        std::vector<std::string> features{};
        void updateParameters(std::map<std::string, std::string>& newParameters);
};
