#include "webScraper.h"

int main() 
{
    std::cout << "Hello, World!" << std::endl;
    WebScraper ws;
    htmlDocPtr html = ws.get_request("https://www.formula1.com/en/results/2025/races/1254/australia/qualifying");
    std::vector<Record> s = ws.get_data(html);
    return 0;
}
