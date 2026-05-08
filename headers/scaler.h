#include <string>
#include <vector>
#include <map>
#include <math.h> 
#include "rapidcsv.h"
#include <algorithm>
#include <optional>

enum ScalerType {
    STANDARD,
    MINMAX,
    ROBUST
};

class Scaler {
    public:
        Scaler(rapidcsv::Document& doc);

        void fit_standard(const std::vector<std::string>& columns, bool clear = false);
        void fit_minmax(const std::vector<std::string>& columns, bool clear = false);
        void fit_robust(const std::vector<std::string>& columns, bool clear = false);

        void apply();

        std::map<std::string, std::map<std::string, double>> parameters{};
        bool fitted = false;

    private:
        rapidcsv::Document& dataset;
        std::vector<std::string> features{};

        void operate(const std::string& column);
        bool getMinMax(const std::string& column, std::optional<double>& min, std::optional<double>& max);
        void updateParameters(std::map<std::string, std::map<std::string, double>>& newParameters);
};