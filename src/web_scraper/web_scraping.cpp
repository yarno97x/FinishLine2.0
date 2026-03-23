#include "webScraper.h"

#include <curl/curl.h>
#include <libxml/HTMLparser.h>
#include <libxml/xpath.h>

#include <cctype>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

namespace
{
size_t write_callback(void* contents, size_t size, size_t nmemb, void* user_data)
{
    auto* response = static_cast<std::string*>(user_data);
    response->append(static_cast<char*>(contents), size * nmemb);
    return size * nmemb;
}

std::string normalize_spaces(const std::string& input)
{
    std::string output;
    output.reserve(input.size());

    bool previous_was_space = false;
    for (unsigned char character : input) {
        if (std::isspace(character) != 0) {
            if (!previous_was_space) {
                output.push_back(' ');
                previous_was_space = true;
            }
        } else {
            output.push_back(static_cast<char>(character));
            previous_was_space = false;
        }
    }

    if (!output.empty() && output.front() == ' ') {
        output.erase(output.begin());
    }
    if (!output.empty() && output.back() == ' ') {
        output.pop_back();
    }

    return output;
}

int to_int_or_default(const std::string& text, int fallback_value = -1)
{
    try {
        return std::stoi(text);
    } catch (...) {
        return fallback_value;
    }
}

std::string node_content_to_string(xmlNodePtr node_ptr)
{
    xmlChar* content_ptr = xmlNodeGetContent(node_ptr);
    std::string text = (content_ptr != nullptr)
        ? reinterpret_cast<const char*>(content_ptr)
        : "";

    if (content_ptr != nullptr) {
        xmlFree(content_ptr);
    }

    return normalize_spaces(text);
}

bool is_element_named(xmlNodePtr node_ptr, const char* expected_name)
{
    return node_ptr != nullptr &&
           node_ptr->type == XML_ELEMENT_NODE &&
           xmlStrEqual(node_ptr->name, BAD_CAST expected_name) != 0;
}

std::unordered_map<std::string, int> parse_header_indices(xmlNodePtr thead_ptr)
{
    std::unordered_map<std::string, int> column_indices;

    for (xmlNodePtr tr_ptr = thead_ptr->children; tr_ptr != nullptr; tr_ptr = tr_ptr->next) {
        if (!is_element_named(tr_ptr, "tr")) {
            continue;
        }

        int column_index = 0;
        for (xmlNodePtr th_ptr = tr_ptr->children; th_ptr != nullptr; th_ptr = th_ptr->next) {
            if (!is_element_named(th_ptr, "th")) {
                continue;
            }

            const std::string header_text = node_content_to_string(th_ptr);
            column_indices[header_text] = column_index;
            ++column_index;
        }

        break;
    }

    return column_indices;
}

std::vector<std::string> parse_row_cells(xmlNodePtr tr_ptr)
{
    std::vector<std::string> cells;

    for (xmlNodePtr td_ptr = tr_ptr->children; td_ptr != nullptr; td_ptr = td_ptr->next) {
        if (!is_element_named(td_ptr, "td")) {
            continue;
        }

        cells.emplace_back(node_content_to_string(td_ptr));
    }

    return cells;
}

std::string get_cell_value(
    const std::vector<std::string>& cells,
    const std::unordered_map<std::string, int>& column_indices,
    const std::string& header_name)
{
    const auto column_iterator = column_indices.find(header_name);
    if (column_iterator == column_indices.end()) {
        return "";
    }

    const int column_index = column_iterator->second;
    if (column_index < 0 || column_index >= static_cast<int>(cells.size())) {
        return "";
    }

    return cells[static_cast<std::size_t>(column_index)];
}

xmlNodePtr find_child_node(xmlNodePtr parent_ptr, const char* child_name)
{
    if (parent_ptr == nullptr) {
        return nullptr;
    }

    for (xmlNodePtr child_ptr = parent_ptr->children; child_ptr != nullptr; child_ptr = child_ptr->next) {
        if (is_element_named(child_ptr, child_name)) {
            return child_ptr;
        }
    }

    return nullptr;
}
} // namespace

WebScraper::WebScraper()
{
    std::cout << "Web Scraper Activated\n";
}

std::vector<Record> WebScraper::scrapeURL(const std::string& url)
{
    htmlDocPtr document_ptr = get_request(url);
    if (document_ptr == nullptr) {
        return {};
    }

    std::vector<Record> records = get_data(document_ptr);
    xmlFreeDoc(document_ptr);
    return records;
}

htmlDocPtr WebScraper::get_request(const std::string& url)
{
    CURL* curl_handle = curl_easy_init();
    std::string response_body;

    if (curl_handle == nullptr) {
        std::cerr << "Failed to init curl\n";
        return nullptr;
    }

    curl_easy_setopt(curl_handle, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, &response_body);
    curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYHOST, 0L);

    const CURLcode result_code = curl_easy_perform(curl_handle);
    if (result_code != CURLE_OK) {
        std::cerr << "curl_easy_perform() failed: "
                  << curl_easy_strerror(result_code) << "\n";
        curl_easy_cleanup(curl_handle);
        return nullptr;
    }

    curl_easy_cleanup(curl_handle);

    if (response_body.empty()) {
        std::cerr << "Empty response\n";
        return nullptr;
    }

    return htmlReadMemory(
        response_body.c_str(),
        static_cast<int>(response_body.size()),
        nullptr,
        nullptr,
        HTML_PARSE_NOERROR | HTML_PARSE_NOWARNING);
}

std::vector<Record> WebScraper::get_data(htmlDocPtr doc)
{
    if (doc == nullptr) {
        return {};
    }

    xmlXPathContextPtr xpath_context_ptr = xmlXPathNewContext(doc);
    if (xpath_context_ptr == nullptr) {
        return {};
    }

    xmlXPathObjectPtr table_object_ptr =
        xmlXPathEvalExpression(BAD_CAST "//table", xpath_context_ptr);

    if (table_object_ptr == nullptr ||
        table_object_ptr->nodesetval == nullptr ||
        table_object_ptr->nodesetval->nodeNr == 0) {
        if (table_object_ptr != nullptr) {
            xmlXPathFreeObject(table_object_ptr);
        }
        xmlXPathFreeContext(xpath_context_ptr);
        return {};
    }

    xmlNodePtr table_node_ptr = table_object_ptr->nodesetval->nodeTab[0];
    xmlNodePtr thead_node_ptr = find_child_node(table_node_ptr, "thead");
    xmlNodePtr tbody_node_ptr = find_child_node(table_node_ptr, "tbody");

    if (thead_node_ptr == nullptr || tbody_node_ptr == nullptr) {
        xmlXPathFreeObject(table_object_ptr);
        xmlXPathFreeContext(xpath_context_ptr);
        return {};
    }

    const std::unordered_map<std::string, int> column_indices =
        parse_header_indices(thead_node_ptr);

    if (column_indices.find("Pos.") == column_indices.end() ||
        column_indices.find("No.") == column_indices.end() ||
        column_indices.find("Driver") == column_indices.end() ||
        column_indices.find("Team") == column_indices.end()) {
        xmlXPathFreeObject(table_object_ptr);
        xmlXPathFreeContext(xpath_context_ptr);
        return {};
    }

    std::vector<Record> records;

    for (xmlNodePtr tr_ptr = tbody_node_ptr->children; tr_ptr != nullptr; tr_ptr = tr_ptr->next) {
        if (!is_element_named(tr_ptr, "tr")) {
            continue;
        }

        const std::vector<std::string> cells = parse_row_cells(tr_ptr);
        if (cells.empty()) {
            continue;
        }

        const int position = to_int_or_default(get_cell_value(cells, column_indices, "Pos."), -1);
        const int number = to_int_or_default(get_cell_value(cells, column_indices, "No."), -1);
        const std::string driver = get_cell_value(cells, column_indices, "Driver");
        const std::string team = get_cell_value(cells, column_indices, "Team");

        const std::string qualifying_one = get_cell_value(cells, column_indices, "Q1");
        const std::string qualifying_two = get_cell_value(cells, column_indices, "Q2");
        const std::string qualifying_three = get_cell_value(cells, column_indices, "Q3");

        const int laps = to_int_or_default(get_cell_value(cells, column_indices, "Laps"), -1);

        const std::string time_or_retired =
            get_cell_value(cells, column_indices, "Time / Retired");
        [[maybe_unused]] const int points =
            to_int_or_default(get_cell_value(cells, column_indices, "Pts."), 0);

        const std::string first_session =
            qualifying_one.empty() ? time_or_retired : qualifying_one;

        if (position <= 0 || number <= 0 || driver.empty() || team.empty()) {
            continue;
        }

        records.emplace_back(Record{
            number,
            driver,
            team,
            first_session,
            qualifying_two,
            qualifying_three,
            position,
            laps
        });
    }

    xmlXPathFreeObject(table_object_ptr);
    xmlXPathFreeContext(xpath_context_ptr);
    return records;
}