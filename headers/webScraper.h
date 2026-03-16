#pragma once
#include <string>
#include <curl/curl.h>
#include "libxml/HTMLparser.h"
#include "libxml/xpath.h"
#include <iostream>
#include <rapidcsv.h>
#include <vector>
#include <stdlib.h>


// ["TrackId", "Code", "Team", "Q1", "Q2", "Q3", "Grid", "Year"]
struct Record 
{
    int trackId;
    std::string code;
    std::string team;
    std::string q1;
    std::string q2;
    std::string q3;
    int grid;
    int year;
};

class WebScraper 
{
    public:
        WebScraper();

        static htmlDocPtr get_request(const std::string &url);
        static std::vector<Record> get_data(htmlDocPtr doc);
};
