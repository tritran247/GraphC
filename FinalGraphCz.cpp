#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <chrono>
#include <iomanip>
#include "subdirectory/matplotlibcpp.h"

namespace plt = matplotlibcpp;

// Structure for log entry
struct LogEntry {
    std::string date;
    std::string start;
    std::string end;
    std::string person;
    double time_spent;
};

// Function to parse time and return duration in minutes
double parse_time_diff(const std::string &start, const std::string &end) {
    std::tm tm_start = {}, tm_end = {};
    std::istringstream ss_start(start);
    std::istringstream ss_end(end);
    ss_start >> std::get_time(&tm_start, "%H:%M");
    ss_end >> std::get_time(&tm_end, "%H:%M");

    auto start_time = std::chrono::system_clock::from_time_t(std::mktime(&tm_start));
    auto end_time = std::chrono::system_clock::from_time_t(std::mktime(&tm_end));

    return std::chrono::duration<double, std::ratio<60>>(end_time - start_time).count();
}

// Function to read log files and return a vector of LogEntry
std::vector<LogEntry> read_logs(const std::vector<std::string> &log_files) {
    std::vector<LogEntry> entries;

    for (const auto &file : log_files) {
        std::ifstream infile(file);
        if (!infile.is_open()) {
            std::cerr << "Error opening file: " << file << std::endl;
            continue;
        }
        
        std::string line;
        std::getline(infile, line);
        std::getline(infile, line); 
        std::string person_name = file.substr(file.find_last_of("/") + 1, file.find_last_of(".") - file.find_last_of("/") - 1);

        while (std::getline(infile, line)) {
            std::istringstream ss(line);
            std::string date, start, end;

            std::getline(ss, date, '\t');
            std::getline(ss, start, '\t');
            std::getline(ss, end, '\t');

            // Calculate time spent
            double time_spent = parse_time_diff(start, end);
            if (time_spent >= 0) {
                entries.push_back({date, start, end, person_name, time_spent});
            }
        }

        infile.close();
    }

    return entries;
}

// Main function
int main() {
    std::vector<std::string> log_files = {
        "path/to/your/DennisGavinLog.csv",
        "path/to/your/DoeJaneLog.csv",
        "path/to/your/SmithJohnLog.csv"
    };

    auto all_data = read_logs(log_files);

    if (all_data.empty()) {
        std::cout << "No data found in the log files." << std::endl;
        return 0;
    }

    // Group by date and person, finding the highest time spent
    std::map<std::string, std::map<std::string, double>> daily_max_per_person;

    for (const auto &entry : all_data) {
        daily_max_per_person[entry.date][entry.person] = std::max(daily_max_per_person[entry.date][entry.person], entry.time_spent);
    }

    // Prepare data for plotting
    std::vector<std::string> dates;
    std::vector<std::string> persons;
    std::vector<double> max_times;

    for (const auto &date_entry : daily_max_per_person) {
        for (const auto &person_entry : date_entry.second) {
            dates.push_back(date_entry.first);
            persons.push_back(person_entry.first);
            max_times.push_back(person_entry.second);
        }
    }

    // Plotting
    plt::figure_size(800, 600);
    std::map<std::string, std::string> person_colors;

    // Create color map
    for (size_t i = 0; i < persons.size(); ++i) {
        if (person_colors.find(persons[i]) == person_colors.end()) {
            person_colors[persons[i]] = plt::get_color(i);
        }
        plt::bar(dates, max_times, "color", person_colors[persons[i]]);
    }

    plt::title("Highest Daily Minute Count", "fontsize", 16, "fontweight", "bold");
    plt::xlabel("Date", "fontsize", 14);
    plt::ylabel("Highest Daily Minute Count (minutes)", "fontsize", 14);
    
    plt::legend(person_colors);
    plt::show();

    return 0;
}