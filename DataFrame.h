#ifndef DATAFRAME_H
#define DATAFRAME_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <unordered_map>

class DataFrame {
private:
    std::unordered_map<std::string, float> siteScores;

public:
    // Load the CSV file and fill the map
    void loadCSV(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Error: Could not open file " << filename << std::endl;
            return;
        }

        std::string line, site;
        float score;
        bool firstLine = true;

        while (std::getline(file, line)) {
            if (firstLine) {
                // Skip header
                firstLine = false;
                continue;
            }

            std::stringstream ss(line);
            std::getline(ss, site, ',');
            ss >> score;

            // Store in map
            siteScores[site] = score;
        }

        file.close();
    }

    // Get score of a specific site
    float getScore(const std::string& site) {
        if (siteScores.find(site) != siteScores.end()) {
            return siteScores[site];
        } else {
            std::cerr << "Site not found: " << site << std::endl;
            return -1.0f; // Indicate not found
        }
    }

    // Display all site-score pairs
    void displayAll() {
        for (const auto& pair : siteScores) {
            std::cout << pair.first << " : " << pair.second << std::endl;
        }
    }
};

#endif // DATAFRAME_H
