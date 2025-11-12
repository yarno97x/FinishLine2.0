#include "webScraper.h"

size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    std::string* response = static_cast<std::string*>(userp);
    response->append(static_cast<char*>(contents), size * nmemb);
    return size * nmemb;
}

WebScraper::WebScraper()
{
    std::cout << "Web Scraper Activated" << std::endl;
}

htmlDocPtr WebScraper::get_request(std::string url) {
    CURL* curl = curl_easy_init();
    std::string result;

    if (!curl) {
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

    htmlDocPtr doc = htmlReadMemory(result.c_str(), result.size(), nullptr, nullptr, HTML_PARSE_NOERROR);
    return doc;
}

std::vector<Record> WebScraper::get_data(htmlDocPtr doc)
{
    xmlXPathContextPtr context = xmlXPathNewContext(doc);
    xmlXPathObjectPtr tbody = xmlXPathEvalExpression(BAD_CAST "//tbody/tr", context);

    if (!tbody)
    {
        std::cerr << "No tbody found" << std::endl; 
        xmlXPathFreeContext(context);
        std::vector<Record> s = {};
        return s;
    }

    xmlNodeSetPtr nodes = tbody->nodesetval;
    if (!nodes) {
        std::cerr << "No nodes found\n";
        xmlXPathFreeObject(tbody);
        xmlXPathFreeContext(context);
        std::vector<Record> s = {};
        return s;
    }

    std::vector<Record> records;
    int grid, year;
    std::string q1, q2, q3, team, code, temp;
    int j = 0;
    int size = (nodes->nodeNr < 0) ? 0 : nodes->nodeNr;
    for (int i = 0; i < size; ++i)
    {
        xmlNodePtr trNode = nodes->nodeTab[i];

        // Iterate over <td> children
        for (xmlNodePtr td = trNode->children; td; td = td->next) {
            if (td->type == XML_ELEMENT_NODE && xmlStrEqual(td->name, BAD_CAST "td")) {
                xmlChar* content = xmlNodeGetContent(td);
                temp = reinterpret_cast<const char*>(content);
                switch (j++)
                {
                    case 0:
                        grid = temp != "NC" ? std::stoi(temp, NULL, 10):-1;
                        break;
                    case 2:
                        temp = reinterpret_cast<const char*>(content);
                        code = temp.substr(temp.length() > 3? temp.length() - 3:0);
                        break;
                    case 3:
                        team = temp;
                        break;
                    case 4:
                        q1 = temp;
                        break;
                    case 5:
                        q2 = temp;
                        break;
                    case 6:
                        q3 = temp;
                        break;
                    default:
                        break;
                }
                xmlFree(content);
            }
        }
        j = 0;
        records.push_back(Record( {-1, code, team, q1, q2, q3, grid, -1} ));
    }
    xmlXPathFreeObject(tbody);
    xmlXPathFreeContext(context);
    return records;
}
