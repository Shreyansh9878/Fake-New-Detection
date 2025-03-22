#include <bits/stdc++.h>
#include "DataFrame.h"
#include "tokenizer.h"
#include "web_search.h"

using namespace std;

// Function to extract domain from URL
string get_site(const string& url) {
    string domain;

    // Remove protocol
    size_t pos = url.find("://");
    if (pos != string::npos) {
        domain = url.substr(pos + 3);
    } else {
        domain = url;
    }

    // Remove path
    pos = domain.find("/");
    if (pos != string::npos) {
        domain = domain.substr(0, pos);
    }

    // Remove "www."
    if (domain.substr(0, 4) == "www.") {
        domain = domain.substr(4);
    }

    // Extract base domain
    stringstream ss(domain);
    string part;
    vector<string> domainParts;

    while (getline(ss, part, '.')) {
        domainParts.push_back(part);
    }

    domain = domainParts[0];
    return domain;
}

// Node structure
struct Node {
    string site_name;
    float score;

    Node(string name, float score) : site_name(name), score(score) {};
};

// Graph to track source traversal
class Graph {
    map<Node*, vector<Node*>> adj_list;

public:
    void append(string site_name, Node* prev_node, DataFrame& df) {
        float score = df.getScore(site_name);
        Node* newNode = new Node(site_name, score);

        adj_list[newNode] = vector<Node*>();

        if (prev_node != nullptr) {
            adj_list[prev_node].push_back(newNode);
        }
    }

    void clearGraph() {
        for (auto& pair : adj_list) {
            delete pair.first;
        }
        adj_list.clear();
    }

    ~Graph() {
        clearGraph();
    }
};

int main() {
    string url;
    cout << "Enter URL to news: ";
    getline(cin, url);

    // Extract site from URL
    string site_name = get_site(url);

    // Create and load dataframe
    DataFrame df;
    df.loadCSV("data.csv");

    // Create graph and add the site
    Graph graph;
    graph.append(site_name, nullptr, df);

    // Display result
    float score = df.getScore(site_name);
    if (score != -1.0f) {
        cout << "Site: " << site_name << " | Score: " << score << endl;
        if (df.isFakeNews(site_name)) {
            cout << "⚠️  This site is likely spreading fake news." << endl;
        } else {
            cout << "✅  This site appears legitimate." << endl;
        }
    }

    return 0;
}
