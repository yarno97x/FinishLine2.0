#include "scaler.h"
#include <optional>

Scaler::Scaler(rapidcsv::Document &doc) : dataset(doc)
{
    features = doc.GetColumnNames();
}

void Scaler::fit_standard(const std::vector<std::string> &columns, bool clear)
{
    // std = \sqrt(E(x^2) - E(X))
    double expectedValueX = 0;
    double expectedValueX2 = 0;

    // Loop over values
    std::map<std::string, std::map<std::string, double>> newParameters{};
    int idx;

    for (const auto& column : columns)
    {
        if (std::find(features.begin(), features.end(), column) != features.end())
        {
            idx = 0;

            for (const auto& item : dataset.GetColumn<std::string>(std::string(column)))
            {
                // Skip imputable items
                if (item.empty() || item == "NaN") { continue; }

                // Check all values are numeric
                size_t pos = 0;
                try 
                {
                    double value = std::stod(item, &pos);
                    if (pos != item.size()) {
                        throw std::invalid_argument( "Invalid input: '" + item + "' is not a valid number");
                    }
                    expectedValueX += value;
                    expectedValueX2 += pow(value, 2);
                    idx++;
                } catch (const std::exception&) {
                    std::cout << "Exception caught\n";
                }
            }

            if (idx <= 0) { 
                throw std::invalid_argument("No values were properly formatted in column " + std::string(column));
            }
            
            fitted = true;
            // Compute mean
            double mean = expectedValueX / idx;
            // Compute std
            double std = sqrt(expectedValueX2/ idx - pow(mean, 2));
            newParameters[column]["mean"] = mean;
            newParameters[column]["std"] = std;
            newParameters[column]["type"] = STANDARD;
            // std::cout << "Mean -> " << newParameters[column]["mean"] << "\n";
            // std::cout << "Std -> " << newParameters[column]["std"] << "\n";
        } else 
        {
            throw std::invalid_argument("Column " + column + " not found in csv");
        }
    }

    if (clear) { parameters.clear(); }

    updateParameters(newParameters);
}

void Scaler::fit_minmax(const std::vector<std::string> &columns, bool clear)
{
    std::optional<double> min;
    std::optional<double> max;

    // Loop over values
    std::map<std::string, std::map<std::string, double>> newParameters{};
    int idx;

    for (const auto& column : columns)
    {
        if (std::find(features.begin(), features.end(), column) != features.end())
        {
            idx = 0;

            for (const auto& item : dataset.GetColumn<std::string>(std::string(column)))
            {
                // Skip imputable items
                if (item.empty() || item == "NaN") { continue; }

                // Check all values are numeric
                size_t pos = 0;
                try 
                {
                    double value = std::stod(item, &pos);

                    if (pos != item.size()) {
                        throw std::invalid_argument( "Invalid input: '" + item + "' is not a valid number");
                    }

                    if (!min.has_value() || min > value) {
                        min = value;
                    }

                    if (!max.has_value() || max < value) {
                        max = value;
                    }
                    idx++;
                } catch (const std::exception&) {
                    std::cout << "Exception caught\n";
                }
            }

            if (idx <= 0) { 
                throw std::invalid_argument("No values were properly formatted in column " + std::string(column));
            }
            
            if (min.has_value() && max.has_value() && min < max) {
                newParameters[column]["min"] = min.value();
                newParameters[column]["max"] = max.value();
                newParameters[column]["type"] = MINMAX;
                // std::cout << "Min -> " << newParameters[column]["min"] << "\n";
                // std::cout << "Max -> " << newParameters[column]["max"] << "\n";
                fitted = true;
            }
        } else 
        {
            throw std::invalid_argument("Column " + column + " not found in csv");
        }
    }
    if (clear) { parameters.clear(); }

    updateParameters(newParameters);
}

static double QuantileLinear(const std::vector<double>& values, const double probability)
{
    if (values.size() == 1U)
    {
        return values[0];
    }

    const auto valueCountMinusOne = static_cast<double>(values.size() - 1U);
    const auto position = probability * valueCountMinusOne;

    const auto lowerIndex = static_cast<std::size_t>(std::floor(position));
    const auto upperIndex = static_cast<std::size_t>(std::ceil(position));
    const auto interpolationWeight = position - static_cast<double>(lowerIndex);

    return values[lowerIndex] * (1.0 - interpolationWeight) +
           values[upperIndex] * interpolationWeight;
}

void Scaler::fit_robust(const std::vector<std::string> &columns, bool clear)
{
    // Loop over values
    std::map<std::string, std::map<std::string, double>> newParameters{};

    for (const auto& column : columns)
    {
        if (std::find(features.begin(), features.end(), column) != features.end())
        {
            std::vector<std::string> col = dataset.GetColumn<std::string>(column);
            std::vector<double> filtered;

            for (const auto& str_value : col) {
                if (!str_value.empty() && str_value != "NaN") {

                    size_t pos = 0;
                    double value;
                    try 
                    {
                        value = std::stod(str_value, &pos);
                        if (pos != str_value.size()) {
                            throw std::invalid_argument( "Invalid input: '" + str_value + "' is not a valid number");
                        }
                        filtered.emplace_back(value);
                    } catch (const std::exception&) {
                        std::cout << "Exception caught\n";
                    }
                }
            }

            std::sort(filtered.begin(), filtered.end());

            newParameters[column]["q1"] = QuantileLinear(filtered, 0.25);
            newParameters[column]["median"] = QuantileLinear(filtered, 0.50);
            newParameters[column]["q3"] = QuantileLinear(filtered, 0.75);
            newParameters[column]["type"] = ROBUST;

            fitted = true;
        } else 
        {
            throw std::invalid_argument("Column " + column + " not found in csv");
        }
    }

    if (clear) { parameters.clear(); }

    updateParameters(newParameters);
}

void Scaler::operate(const std::string& column) {

    for (int i = 0; i < dataset.GetRowCount(); i++)
    {
        auto item = dataset.GetCell<std::string>(column, i);
        double value;
        if (item.empty() || item == "NaN") { continue; }

        // Check all values are numeric
        size_t pos = 0;
        try 
        {
            value = std::stod(item, &pos);
            if (pos != item.size()) {
                throw std::invalid_argument( "Invalid input: '" + item + "' is not a valid number");
            }
        } catch (const std::exception&) {
            std::cout << "Exception caught\n";
        }

        double newValue;
        if (parameters[column]["type"] == STANDARD) {
            newValue = (value - parameters[column]["mean"]) / parameters[column]["std"];
        } else if (parameters[column]["type"] == MINMAX) {
            newValue = (value - parameters[column]["min"]) / (parameters[column]["max"] - parameters[column]["min"]);
        } else {
            newValue = (value - parameters[column]["median"]) / (parameters[column]["q3"] - parameters[column]["q1"]);
        }

        dataset.SetCell(column, i, newValue);
    }   
}

void Scaler::apply()
{     
    if (!fitted) { return; }

    for (auto &parameter : parameters)
    {
        if (std::find(features.begin(), features.end(), parameter.first) == features.end())
        {
            throw std::invalid_argument("Did not find " + parameter.first);
        }

        operate(parameter.first);
    }
}


void Scaler::updateParameters(std::map<std::string, std::map<std::string, double>>& newParameters)
{
    for (auto &parameter : newParameters)
    {
        parameters[parameter.first] = parameter.second;
    }
}


