#pragma once

#include <vector>
#include <string_view>
#include <unordered_set>
#include <iostream>
#include <fstream> 
#include <thread>
#include <mutex>

class Crawler {
    public :
        static std::vector<std::string> load_urls();
        static std::unordered_set<std::string> load_urls_already_scraped();
        void save_scraped_urls();
        void scrape_urls();
        void worker_thread(int start_index);

    private :
        std::vector<Record> new_records;
        std::mutex record_mutex;
        std::mutex print_mutex;
        std::mutex visited_mutex;
        std::vector<std::string> all_urls;
        std::unordered_set<std::string> urls_already_scraped;
        int size;
        int worker_count;
};
