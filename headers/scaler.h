#include <vector>
#include <string>
#include <rapidcsv.h>
#include <algorithm>
#include <stdexcept>
#include <queue>
#include <limits>
#include <cmath>

enum ScalerTransformType
{
    MINMAX,
    STANDARDIZER,
    ROBUST,
    L1_NORM,
    L2_NORM
};

struct ScalerTransform
{
    ScalerTransformType transformType;
    std::map<std::string, double> parameters;
    bool clamp;
};

class Scaler
{
    public:
        Scaler(rapidcsv::Document& doc);

        void fit_min_max(const std::vector<std::string>& columns, bool clear = false, bool clamp = false);
        void fit_standardizer(const std::vector<std::string>& columns, bool clear = false);
        void fit_robust(const std::vector<std::string>& columns, bool clear = false); 
        void fit_l2_norm(const std::vector<std::string>& columns, bool clear = false);
        void applyScalerTransform();
        std::map<std::string, ScalerTransform> transforms{};
        bool fitted = false;

    private:
        rapidcsv::Document& dataset;
        std::vector<std::string> features{};
        void updateParameters(std::map<std::string, ScalerTransform>& newParameters);
};
