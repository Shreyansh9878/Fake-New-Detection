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

    domain = domainParts[0]; //return only main domain
    transform(domain.begin(), domain.end(), domain.begin(), ::tolower); //lower for searching from data
    return domain;
}

struct GraphNode{
    string site_name;
    float score; //score saved in data
    float T_node; //updated score after propagation
    int depth; //starting node (user inputed) is 0

    GraphNode(string name, float score): site_name(name), score(score), depth(0), T_node(-1) {};
};

struct Site{
    float trust_score; //original score from data
    string url; //stores url encountered of a site to return the best 5 sites
    GraphNode* node; //stores node in the graph to avoid duplicate notes
    float weight; //weight of the site in the wrt starting node

    Site(float score, string url, GraphNode* node, int w): trust_score(score), url(url), node(node), weight(w) {};
};

class Graph{
    map<GraphNode*, vector<pair<GraphNode*, float>>> adj_list; //directed connections between 2 GraphNodes
    unordered_map<string, Site*> list_of_sites; //Stores name of all the sites with all the related data to it (Site structure)
    GraphNode* start_node; //user inputed node
    Trie original_content; //content of user inputed site

private:
    GraphNode* append(string site_name, string url, GraphNode* prev_node, float edge_weight){
        // get score from data.csv
        DataFrame data;
        data.loadCSV("data.csv");
        float score = data.getScore(site_name);

        //create a node, make a connection list from site and update the list of sites
        GraphNode* newNode = new GraphNode(site_name, round(score*100)/100);
        adj_list[newNode] = vector<pair<GraphNode*, float>> {};
        list_of_sites[site_name] = new Site(score, url, newNode, edge_weight);

        //if starting node then depth is 0 and start_node is updated
        if (prev_node == NULL){
            newNode->depth = 0;
            start_node = newNode;
            return newNode;
        }
        
        //update depth
        newNode->depth = prev_node->depth+1;

        adj_list[prev_node].push_back({newNode, edge_weight}); //make a connection from site that listed it

        return newNode;
    }

    void create_Graph(string url, GraphNode* prev_Node){ //recursively finds new nodes from present sites
        if (url.empty()) return;

        string site_name = get_site(url);

        // if the site if already a node then just create the connection instead of creating a new node
        if (list_of_sites.find(site_name) != list_of_sites.end()){
            adj_list[prev_Node].push_back({list_of_sites[site_name]->node, list_of_sites[site_name]->weight});
            return;
        }

        //web_search
        vector<string> links;
        string news_content_original;

        //tokenize
        vector<string> content;
        
        //create a weight based on content matching
        float weight = 0;//compare content with Trie original_content
        
        //Append Node to graph
        GraphNode* newNode = append(site_name, url, prev_Node, weight);

        //if depth of this node reaches the threshold then dont make further connections
        if (newNode->depth < depth_treshold){
            for (string link: links){
                create_Graph(link, newNode);
            }
        }

        return;
    }

    float dfs_visit(GraphNode* node, unordered_map<GraphNode*, int>& ancestor){
        if (node->T_node != -1) return node->T_node; //if this node is already visited it will have its T_node != -1. So we can use this value instead of visiting all its children again

        ancestor[node] = 1;

        float score = influence * node->score;
        float propagated_score = 0;
        float total_weights = 0;

        for (auto it: adj_list[node]){
            if (ancestor[it.first]==1) {//if a node points to one of its ancestral node then it should just update the score of the ancestral node instead of propagating the score
                total_weights += it.second;
                propagated_score += it.first->score;
            }
            else{
                total_weights += it.second;
                propagated_score += dfs_visit(it.first, ancestor)*it.second;
            }
        }

        score += (1-influence)*(propagated_score/total_weights);
        node->T_node = score; //update T_node

        ancestor[node] = 0; //making ancestor as 0 because it is not ancestor for next node that comes

        return score;
    }


public:
    void create(string url){
        if (url.empty()) return;

        //web_search
        vector<string> links;
        string news_content_original;

        //tokenizer and create a Trie
        // original_content;
        
        //Append Initial Node to graph
        string site_name = get_site(url);
        GraphNode* newNode = append(site_name, url, NULL, 1);

        //automatically create nodes from mentioned sites
        for (string link: links){
            create_Graph(link, newNode);
        }
    }

    float get_score(){
        unordered_map<GraphNode*, int> ancestor; //to avoid unneccessary looping
        return dfs_visit(start_node, ancestor);
    }

    vector<pair<string, float>> get_top_sites(){
        vector<pair<string, float>> result;
        priority_queue<pair<float, string>> pq; //max heap to get top sites with top scores

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
        // Free all GraphNode objects
        for (auto& pair : adj_list) {
            delete pair.first; // Delete the GraphNode itself
        }
        adj_list.clear(); // Clear adjacency list
    
        // Free all Site objects
        for (auto& pair : list_of_sites) {
            delete pair.second; // Delete the Site object
        }
        list_of_sites.clear(); // Clear site map
    
        // Reset the start node
        start_node = nullptr;
    
        // Clear the trie
        // original_content.clear();
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
