#include "webScraper.h"

size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    auto* response = static_cast<std::string*>(userp);
    response->append(static_cast<char*>(contents), size * nmemb);
    return size * nmemb;
}

WebScraper::WebScraper()
{
    std::cout << "Web Scraper Activated" << "\n";
}

htmlDocPtr WebScraper::get_request(const std::string &url) {
    CURL* curl = curl_easy_init();
    std::string result;

    if (curl == nullptr) {
        std::cerr << "Failed to init curl\n";
        return nullptr;
    }

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &result);
    // curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L); // testing only
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        std::cerr << "curl_easy_perform() failed: "
                  << curl_easy_strerror(res) << "\n";
        curl_easy_cleanup(curl);
        return nullptr;
    }

    curl_easy_cleanup(curl);

    if (result.empty()) {
        std::cerr << "Empty response!\n";
        return nullptr;
    }

    htmlDocPtr doc = htmlReadMemory(result.c_str(), (int) result.size(), nullptr, nullptr, HTML_PARSE_NOERROR);
    return doc;
}

std::vector<Record> WebScraper::get_data(htmlDocPtr doc)
{
    xmlXPathContextPtr context = xmlXPathNewContext(doc);
    xmlXPathObjectPtr tbody = xmlXPathEvalExpression(BAD_CAST "//tbody/tr", context);

    if (tbody == nullptr)
    {
        std::cerr << "No tbody found" << "\n"; 
        xmlXPathFreeContext(context);
        std::vector<Record> emptySetOfRecords = {};
        return emptySetOfRecords;
    }

    xmlNodeSetPtr nodes = tbody->nodesetval;
    if (nodes == nullptr) {
        std::cerr << "No nodes found\n";
        xmlXPathFreeObject(tbody);
        xmlXPathFreeContext(context);
        std::vector<Record> emptySetOfRecords = {};
        return emptySetOfRecords;
    }

    std::vector<Record> records;
    int grid;
    int year;
    std::string quali1;
    std::string quali2;
    std::string quali3;
    std::string team;
    std::string code;
    std::string temp;
    int childIndex = 0;
    int size = (nodes->nodeNr < 0) ? 0 : nodes->nodeNr;
    for (int i = 0; i < size; ++i)
    {
        xmlNodePtr trNode = nodes->nodeTab[i];

        // Iterate over <td> children
        for (xmlNodePtr td = trNode->children; td != nullptr; td = td->next) {
            if (td->type == XML_ELEMENT_NODE && xmlStrEqual(td->name, BAD_CAST "td") == 1) {
                xmlChar* content = xmlNodeGetContent(td);
                temp = reinterpret_cast<const char*>(content);
                switch (childIndex++)
                {
                    case 0:
                        grid = temp != "NC" ? std::stoi(temp, nullptr, 10):-1;
                        break;
                    case 2:
                        temp = reinterpret_cast<const char*>(content);
                        code = temp.substr(temp.length() > 3? temp.length() - 3:0);
                        break;
                    case 3:
                        team = temp;
                        break;
                    case 4:
                        quali1 = temp;
                        break;
                    case 5:
                        quali2 = temp;
                        break;
                    case 6:
                        quali3 = temp;
                        break;
                    default:
                        break;
                }
                xmlFree(content);
            }
        }
        childIndex = 0;
        records.push_back(Record( {-1, code, team, quali1, quali2, quali3, grid, -1} ));
    }
    xmlXPathFreeObject(tbody);
    xmlXPathFreeContext(context);
    return records;
}
