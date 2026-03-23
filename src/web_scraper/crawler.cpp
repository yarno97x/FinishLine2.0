#include <webScraper.h>
#include <fstream>
#include "crawler.h"

std::vector<std::string> Crawler::load_urls()
{
    std::ifstream url_file("/home/yarno97x/Desktop/FinishLine2.0/data/race_links.txt");
    std::string temp;
    std::vector<std::string> result{};

    while (getline(url_file, temp)) {
        result.emplace_back(temp + "/qualifying");
    }
    return result;
}

std::unordered_set<std::string> Crawler::load_urls_already_scraped()
{
    std::ifstream url_file("/home/yarno97x/Desktop/FinishLine2.0/data/scraped_urls.txt");
    std::string temp;
    std::unordered_set<std::string> result{};

    while (getline(url_file, temp)) {
        result.insert(temp);
    }
    return result;
}

void Crawler::save_scraped_urls()
{
    std::ofstream url_file("/home/yarno97x/Desktop/FinishLine2.0/data/scraped_urls.txt");

    for (const std::string &url : urls_already_scraped) {
        url_file << url << "\n";
    }
}

void Crawler::worker_thread(int start_index) 
{
    std::string current_url{};
    std::vector<Record> parsed_list;
    int size = static_cast<int>(all_urls.size());

    // Privatized list of new records
    std::vector<std::vector<Record>> _new_records;
    std::vector<std::string> _urls_scraped;

    for (start_index; start_index < size; start_index += worker_count) 
    {
        current_url = all_urls.at(start_index);
        _urls_scraped.emplace_back(current_url);

        // Scrape the URL 
        parsed_list = WebScraper::scrapeURL(current_url);

        if (parsed_list.empty()) {
            _urls_scraped.pop_back();
            continue;
        } 

        {   // Save the record
            std::lock_guard<std::mutex> print_lock(print_mutex);
            std::cout << current_url << "\n";
            for (const Record& record : parsed_list) {
                std::cout << "\t" << record.code << " " << record.grid << "\n";
            }
        }
        _new_records.emplace_back(parsed_list);
    }

    int url_index = 0;
    for (auto& record_list : _new_records) {
        {   // Check the url isn't already in the vector
            std::lock_guard<std::mutex> existence_lock(visited_mutex);
            if (urls_already_scraped.find(_urls_scraped.at(url_index++)) != urls_already_scraped.end()) {
                continue;
            }
            urls_already_scraped.insert(current_url);
        }
        {   // Insert the record list inside the shared vector
            std::lock_guard<std::mutex> insertion_lock(write_mutex);
            new_records.insert(new_records.end(), record_list.begin(), record_list.end());
        }
    }
}

void Crawler::scrape_urls()
{
    worker_count = 6;
    all_urls = load_urls(); 
    urls_already_scraped = load_urls_already_scraped();
    std::cout << "Size already scraped is " << urls_already_scraped.size() << "\n";

    std::vector<std::thread> threads; 
    threads.reserve((std::size_t)worker_count);

    for (int thread_index = 0; thread_index < worker_count; thread_index++) {
        threads.emplace_back(&Crawler::worker_thread, this, thread_index);
    }

    for (auto& thread : threads) {
        thread.join();
    }
    save_scraped_urls();
}
