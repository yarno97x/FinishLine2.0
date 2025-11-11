#pragma once
#include <string>
#include <curl/curl.h>
#include "libxml/HTMLparser.h"
#include "libxml/xpath.h"
#include <iostream>
#include <json.hpp>
#include <vector>


// ["TrackId", "Code", "Team", "Q1", "Q2", "Q3", "Grid", "Year"]
struct Record 
{
    int trackId;
    std::string code;
    std::string team;
    double q1;
    double q2;
    double q3;
    int grid;
    int year;
};

class WebScraper 
{
    public:
        WebScraper();

        htmlDocPtr get_request(std::string url);

        std::vector<Record> get_data(htmlDocPtr doc);
};
