#include "scaler.h"

Scaler::Scaler(rapidcsv::Document &doc) : dataset(doc)
{
    features = doc.GetColumnNames();
}

void Scaler::fit_min_max(const std::vector<std::string> &columns, bool clear, bool clamp)
{
    std::map<std::string, ScalerTransform> newParameters{};
    double min;
    double max;
    double current_item;

    for (const auto& column : columns)
    {
        if (std::find(features.begin(), features.end(), column) != features.end())
        {
            min = std::numeric_limits<double>::infinity();
            max = -std::numeric_limits<double>::infinity();

            for (const auto& item : dataset.GetColumn<std::string>(std::string(column)))
            {
                // Skip imputable items
                if (item.empty() || item == "NaN") continue;

                // Check all values are numeric
                size_t pos = 0;
                try 
                {
                    current_item = std::stod(item, &pos);
                    if (pos != item.size()) throw std::invalid_argument( "Invalid input: '" + item + "' is not a valid number");

                    max = max > current_item ? max : current_item;
                    min = min < current_item ? min : current_item;
                } catch (const std::exception&) {}
            }

            if (min == std::numeric_limits<double>::infinity()) throw std::invalid_argument("No fields were found");

            fitted = true;
            newParameters[column] = ScalerTransform{
                .transformType = MINMAX,
                .parameters = { {"min", min}, {"max", max}},
                .clamp = clamp
            };
        } else 
        {
            throw std::invalid_argument("Column " + column + " not found in csv");
        }
    }

    if (clear) transforms.clear();

    updateParameters(newParameters);
}

void Scaler::fit_standardizer(const std::vector<std::string> &columns, bool clear)
{    
    std::map<std::string, ScalerTransform> newParameters{};
    double mean;
    int idx;
    double current_item;
    double variance;

    for (const auto& column : columns)
    {
        if (std::find(features.begin(), features.end(), column) != features.end())
        {
            idx = 0;
            mean = 0;

            for (const auto& item : dataset.GetColumn<std::string>(std::string(column)))
            {
                // Skip imputable items
                if (item.empty() || item == "NaN") continue;

                // Check all values are numeric
                size_t pos = 0;
                try 
                {
                    current_item = std::stod(item, &pos);
                    if (pos != item.size()) throw std::invalid_argument( "Invalid input: '" + item + "' is not a valid number");
                    idx++;
                    mean += current_item;
                } catch (const std::exception&) {}
            }

            if (idx <= 0) throw std::invalid_argument("No fields were found");

            mean /= idx;

            // Compute Stdev
            variance = 0;
            for (const auto& item : dataset.GetColumn<std::string>(std::string(column)))
            {
                // Skip imputable items
                if (item.empty() || item == "NaN") continue;

                // Check all values are numeric
                size_t pos = 0;
                try 
                {
                    current_item = std::stod(item, &pos);
                    if (pos != item.size()) throw std::invalid_argument( "Invalid input: '" + item + "' is not a valid number");
                    variance += std::pow(current_item - mean, 2);
                } catch (const std::exception&) {}
            }

            fitted = true;
            newParameters[column] = ScalerTransform{
                .transformType = STANDARDIZER,
                .parameters = { {"mean", mean}, {"std", std::sqrt(variance/idx)}},
                .clamp = false
            };
        } else 
        {
            throw std::invalid_argument("Column " + column + " not found in csv");
        }
    }

    if (clear) transforms.clear();

    updateParameters(newParameters);
}

void Scaler::updateParameters(std::map<std::string, ScalerTransform> &newParameters)
{
    for (auto it = newParameters.begin(); it != newParameters.end(); it++)
    {
        transforms[it->first] = it->second;
    }
}
