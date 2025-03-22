#include <bits/stdc++.h>
#include "DataFrame.h"
#include "tokenizer.h"
#include "web_search.h"
using namespace std;

string get_site(const string& url) {
    string domain;

    // Remove protocol (https:// or http://)
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

    // Split domain into parts
    stringstream ss(domain);

    //remove www.
    if (domain.substr(0, 4) == "www.") {
        domain = domain.substr(4);
    }

    string part;
    vector<string> domainParts;

    while (getline(ss, part, '.')) {
        domainParts.push_back(part);
    }

    domain = domainParts[0];
    return domain;
}

struct Node{
    string site_name;
    float score;

    Node(string name, float score): site_name(name), score(score) {};
};

class Graph{
    map<Node*, vector<Node*>> adj_list;
private:
public:
    void append(string site_name, Node* prev_node){
        DataFrame data;
        data.loadCSV("data.csv");
        float score = data.getScore(site_name);
        Node* newNode = new Node(site_name, score);

        adj_list[newNode] = vector<Node*> {};

        if (prev_node == NULL) return;
        adj_list[prev_node].push_back(newNode);

        return;
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


int main(){
    string url;
    cout << "Enter URL to news: ";
    getline(cin, url);

    string site_name = get_site(url);

    Graph graph;
    graph.append(site_name, NULL);

    return 0;
}
