#include <webScraper.h>
#include "crawler.h"
#include <fstream>

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
    {
        std::lock_guard<std::mutex> lock(print_mutex);
        std::cout << "Thread " << start_index + 1 << " started\n";
    }
    std::string current_url{};
    std::vector<Record> new_record;
    int size = static_cast<int>(all_urls.size());

    for (start_index; start_index < size; start_index += worker_count) 
    {
        current_url = all_urls.at(start_index);
        {
            std::lock_guard<std::mutex> lock3(visited_mutex);
            if (urls_already_scraped.find(current_url) != urls_already_scraped.end()) {
                continue;
            }
            urls_already_scraped.insert(current_url);
        }

        // Scrape url 
        new_record = WebScraper::scrapeURL(current_url);

        if (new_record.empty()) {
            {
                std::lock_guard<std::mutex> lock(visited_mutex);
                urls_already_scraped.erase(current_url);
            }
            continue;
        } 
        std::lock_guard<std::mutex> lock3(visited_mutex);
        urls_already_scraped.insert(current_url);

        // Save the record
        {
            std::lock_guard<std::mutex> lock(print_mutex);
            for (const Record& record : new_record) {
                std::cout << record.code << " " << record.grid << "\n";
            }
        }

        {
            std::lock_guard<std::mutex> lock(record_mutex);
            new_records.insert(new_records.end(), new_record.begin(), new_record.end());
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
