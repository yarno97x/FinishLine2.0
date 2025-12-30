#include "imputer.h"

Imputer::Imputer(rapidcsv::Document& doc) : dataset(doc) 
{
    features = doc.GetColumnNames();
}

void Imputer::updateParameters(std::map<std::string, std::string>& newParameters)
{
    for (auto it = newParameters.begin(); it != newParameters.end(); it++)
    {
        parameters[it->first] = it->second;
    }
}

void Imputer::fit_mean(const std::vector<std::string>& columns, bool clear)
{
    std::map<std::string, std::string> newParameters{};
    double total;
    int idx;

    for (const auto& column : columns)
    {
        if (std::find(features.begin(), features.end(), column) != features.end())
        {
            total = 0;
            idx = 0;

            for (const auto& item : dataset.GetColumn<std::string>(std::string(column)))
            {
                // Skip imputable items
                if (item.empty() || item == "NaN") continue;

                // Check all values are numeric
                size_t pos = 0;
                try 
                {
                    total += std::stod(item, &pos);
                    if (pos != item.size()) throw std::invalid_argument( "Invalid input: '" + item + "' is not a valid number");
                    idx++;
                } catch (const std::exception&) {}
            }

            if (idx <= 0) throw std::invalid_argument("No values were properly formatted in column " + std::string(column));

            fitted = true;
            newParameters[column] = std::to_string(total / idx);
        } else 
        {
            throw std::invalid_argument("Column " + column + " not found in csv");
        }
    }

    if (clear) parameters.clear();

    updateParameters(newParameters);
}

void Imputer::fit_median(const std::vector<std::string> &columns, bool clear)
{
    std::map<std::string, std::string> newParameters{};
    int middle;
    double current;
    int median;

    for (const auto& column : columns)
    {
        if (std::find(features.begin(), features.end(), column) != features.end())
        {
            std::priority_queue<double> heap;

            for (const auto& item : dataset.GetColumn<std::string>(std::string(column)))
            {
                // Skip imputable items
                if (item.empty() || item == "NaN") continue;

                // Check all values are numeric
                size_t pos = 0;
                try 
                {
                    current = std::stod(item, &pos);
                    if (pos != item.size()) throw std::invalid_argument( "Invalid input: '" + item + "' is not a valid number");
                    heap.push(current);
                } catch (const std::exception&) {}
            }

            if (heap.size() <= 0) throw std::invalid_argument("No values were properly formatted in column " + std::string(column));

            fitted = true;
            middle = static_cast<int>((heap.size() - 1)/2);
            if (heap.size() % 2 == 0) {
                // Get average of middle two values
                for (int i = 0; i < middle; i++) heap.pop();
                median = heap.top();
                heap.pop();
                median = (median + heap.top())/2;
            } else 
            {
                for (int i = 0; i < middle; i++) heap.pop();
                median = heap.top();
            }

            newParameters[column] = std::to_string(median);
        }
        else 
        {
            throw std::invalid_argument("Column " + column + " not found in csv");
        }
    }
    
    if (clear) parameters.clear();

    updateParameters(newParameters);
}

void Imputer::fit_frequency(const std::vector<std::string> &columns, bool clear)
{
    std::map<std::string, std::string> newParameters{};
    int middle;
    double current;
    int median;

    for (const auto& column : columns)
    {
        if (std::find(features.begin(), features.end(), column) != features.end())
        {
            std::map<std::string, int> counts;
            std::pair<std::string, int> maxCount("", -1);

            for (const auto& item : dataset.GetColumn<std::string>(std::string(column)))
            {
                // Skip imputable items
                if (item.empty() || item == "NaN") continue;

                counts[item]++;

                if (counts[item] > maxCount.second)
                {
                    maxCount.first = item;
                    maxCount.second = counts[item];
                }
            }

            if (maxCount.first == "") throw std::invalid_argument("No values were properly formatted in column " + std::string(column));

            fitted = true;
            newParameters[column] = maxCount.first;
        }
        else 
        {
            throw std::invalid_argument("Column " + column + " not found in csv");
        }
    }
    
    if (clear) parameters.clear();

    updateParameters(newParameters);
}

void Imputer::fit_constant(const std::map<std::string, std::string>& replacements, bool clear)
{
    std::map<std::string, std::string> newParameters{};

    for (const auto& [key, value] : replacements)
    {
        if (std::find(features.begin(), features.end(), key) == features.end())
            throw std::invalid_argument("Replacement for unknown column: " + key);

        newParameters[key] = value;
    }

    if (clear) parameters.clear();

    fitted |= !replacements.empty();
    updateParameters(newParameters);
}
