#include <bits/stdc++.h>
#include "DataFrame.h"
#include "tokenizer.h"
#include "web_search.h"
using namespace std;

const int depth_treshold = 5;
const float influence = 0.75;

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

    //remove www.
    if (domain.substr(0, 4) == "www.") {
        domain = domain.substr(4);
    }

    // Split domain into parts
    stringstream ss(domain);
    string part;
    vector<string> domainParts;

    while (getline(ss, part, '.')) {
        domainParts.push_back(part);
    }

    domain = domainParts[0];
    transform(domain.begin(), domain.end(), domain.begin(), ::tolower);
    return domain;
}

struct GraphNode{
    string site_name;
    float score;
    float T_node;
    int depth;

    GraphNode(string name, float score): site_name(name), score(score), depth(0), T_node(-1) {};
};

struct Site{
    float trust_score;
    string url;
    GraphNode* node;
    float weight;

    Site(float score, string url, GraphNode* node, int w): trust_score(score), url(url), node(node), weight(w) {};
};

class Graph{
    map<GraphNode*, vector<pair<GraphNode*, float>>> adj_list;
    unordered_map<string, Site*> list_of_sites;
    GraphNode* start_node;
    Trie original_content;

private:
    GraphNode* append(string site_name, string url, GraphNode* prev_node, float edge_weight){
        DataFrame data;
        data.loadCSV("data.csv");
        float score = data.getScore(site_name);
        GraphNode* newNode = new GraphNode(site_name, round(score*100)/100);

        adj_list[newNode] = vector<pair<GraphNode*, float>> {};
        list_of_sites[site_name] = new Site(score, url, newNode, edge_weight);

        if (prev_node == NULL){
            newNode->depth = 0;
            start_node = newNode;
            return;
        }
        
        newNode->depth = prev_node->depth+1;
        adj_list[prev_node].push_back({newNode, edge_weight});

        return newNode;
    }

    void create_Graph(string url, GraphNode* prev_Node){
        if (url.empty()) return;

        string site_name = get_site(url);

        //web_search
        vector<string> links;
        string news_content_original;

        //tokenizer
        vector<string> content;
        
        float weight = 0;//compare content with Trie original_content
        
        if (list_of_sites.find(site_name) != list_of_sites.end()){
            adj_list[prev_Node].push_back({list_of_sites[site_name]->node, list_of_sites[site_name]->weight});
        }

        //Append Initial Node to graph
        GraphNode* newNode = append(site_name, url, prev_Node, weight);

        if (newNode->depth < depth_treshold){
            for (string link: links){
                create_Graph(link, newNode);
            }
        }

        return;
    }

    float dfs_visit(GraphNode* node){
        if (node->T_node != -1) return node->T_node;
        float score = influence*node->score;
        float propagated_score = 0;
        float total_weights=0;

        for (auto it: adj_list[node]){
            total_weights += it.second;
            propagated_score += dfs_visit(it.first)*it.second;
        }

        score += (1-influence)*(propagated_score/total_weights);
        node->T_node = score;

        return score;
    }


public:
    void create(string url){
        if (url.empty()) return;

        //web_search
        vector<string> links;
        string news_content_original;

        //tokenizer
        // original_content;
        
        //Append Initial Node to graph
        string site_name = get_site(url);
        GraphNode* newNode = append(site_name, url, NULL, 0);

        for (string link: links){
            create_Graph(link, newNode);
        }
    }

    float get_score(){
        return dfs_visit(start_node);
    }

    vector<pair<string, float>> get_top_sites(){
        vector<pair<string, float>> result;
        priority_queue<pair<float, string>> pq;

        for (auto it: list_of_sites){
            pq.push({it.second->node->T_node, it.second->url});            
        }

        for (int i=0; i<5; i++){
            if (pq.empty()) break;

            pair<float, string> curr = pq.top();
            result.push_back({curr.second, curr.first});            
        }

        return result;
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

    Graph obj;
    obj.create(url);
    float final_score = obj.get_score();
    vector<pair<string, float>> top_sites = obj.get_top_sites();

    cout << "Trust_Score:" << final_score;

    cout << "You can get most trusted information of this news from: " << endl;
    for (auto it: top_sites){
        cout << it.first << " " << fixed << setprecision(2) << it.second << endl;
    }
    return 0;
}
