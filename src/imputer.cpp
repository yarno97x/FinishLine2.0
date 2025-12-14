#include "imputer.h"

Imputer::Imputer(rapidcsv::Document& doc) (doc) 
{
    if (doc == nullptr)
    {
        return;
    }
    features = doc.GetColumnNames();
}

void Imputer::fit_mean(const std::vector<std::string_view>& columns)
{
    std::vector<ColumnChange> newParameters{};
    double total;
    int idx;
    for (const auto& column : columns)
    {
        if (std::find(features.begin(), features.end(), column) != features.end())
        {
            total = 0;
            idx = 0;

            for (const auto& item : dataset.GetColumn(column))
            {
                // Skip imputable items
                if (item.empty()) continue;

                // Check all values are numeric
                size_t pos = 0;
                try 
                {
                    total += std::stod(item, &pos);
                    if (pos != item.size()) throw std::invalid_argument( "Invalid input: '" + item + "' is not a valid number");
                    idx++;
                } catch (const std::exception&) 
                {
                    throw std::invalid_argument( "Invalid input: '" + item + "' is not a valid number");
                }
            }

            if (idx <= 0) throw std::invalid_argument("No values were properly formatted in column " + std::string(column));

            newParameters.push_back(ColumnChange{
                std::string(column),
                std::to_string(total / idx)
            });
        }
    }
    
    parameters.clear();
    parameters = std::move(newParameters);
    mode = TRANSFORM::MEAN;
}
