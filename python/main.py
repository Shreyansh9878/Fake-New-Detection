import numpy as np
import pandas as pd
import heapq
import re
from collections import defaultdict
from urllib.parse import urlparse

import Tokenizer_Trie as tk_t
import web_scrapper as ws

DEPTH_THRESHOLD = 5
INFLUENCE = 0.75

def get_site(url):
    parsed_url = urlparse(url)
    domain = parsed_url.netloc
    if domain.startswith("www."):
        domain = domain[4:]
    return domain.split('.')[0].lower()

# Class for Graph Nodes
class GraphNode:
    def __init__(self, name, score):
        self.site_name = name
        self.score = round(score, 2)
        self.T_node = -1
        self.depth = 0

# Class for Site data
class Site:
    def __init__(self, score, url, node, weight):
        self.trust_score = score
        self.url = url
        self.node = node
        self.weight = weight

# Class for Graph
class Graph:
    def __init__(self):
        self.adj_list = defaultdict(list)
        self.list_of_sites = {}
        self.start_node = None
        self.trie_obj = None
        
    def load_csv(self, path):
        data = pd.read_csv(path)
        columns = data.columns.tolist()
        data = data.set_index(columns[0])
        return data
    
    def get_score_from_data(self, site_name):
        data = self.load_csv("data.csv")
        return data[site_name.lower()]
    
    def append(self, site_name, url, prev_node=None, edge_weight=0):
        # get score from data.csv
        score = self.get_score_from_data(site_name)

        # create a node, make a connection list from site and update the list of sites
        new_node = GraphNode(site_name, round(score,2))
        self.adj_list[new_node] = []
        self.list_of_sites[site_name] = Site(score, url, new_node, edge_weight)
        
        if prev_node is None:
            new_node.depth = 0
            self.start_node = new_node
        else:
            new_node.depth = prev_node.depth + 1
            self.adj_list[prev_node].append((new_node, edge_weight))
        
        return new_node
    
    def create_graph(self, url, prev_node=None):
        if not url:
            return
        
        else:
            site_name = get_site(url)
            
            if site_name in self.list_of_sites:
                self.adj_list[prev_node].append((self.list_of_sites[site_name].node, self.list_of_sites[site_name].weight))
                return
            
            else: 
                links, content = ws.get_data(url)
                weight = tk_t.compare_data(self.trie_obj, content)
                
                new_node = self.append(site_name, url, prev_node, weight)
                
                if new_node.depth < DEPTH_THRESHOLD:
                    for link in links:
                        self.create_graph(link, new_node)
        
    def dfs_visit(self, node, ancestor):
        if node.T_node != -1:
            return node.T_node
        
        ancestor[node] = True
        score = INFLUENCE * node.score
        propagated_score = 0
        total_weights = 0
        
        for neighbor, weight in self.adj_list[node]:
            if neighbor in ancestor:
                total_weights += weight
                propagated_score += neighbor.score
            else:
                total_weights += weight
                propagated_score += self.dfs_visit(neighbor, ancestor) * weight
        
        score += (1 - INFLUENCE) * (propagated_score / total_weights)
        
        node.T_node = score
        ancestor[node] = False
        return score
    
    def create(self, url):
        if not url:
            return
        
        links, original_news = ws.get_data(url)
        self.trie_obj = tk_t.extract_keywords(original_news, True)

        site_name = get_site(url)
        new_node = self.append(site_name, url, None, 1)
        
        for link in links:
            self.create_graph(link, new_node)
    
    def get_score(self):
        ancestor = {}
        return self.dfs_visit(self.start_node, ancestor)
    
    def get_top_sites(self):
        pq = [(-site.node.T_node, site.url) for site in self.list_of_sites.values()]
        heapq.heapify(pq)
        
        result = []
        for _ in range(5):
            if not pq:
                break
            score, url = heapq.heappop(pq)
            result.append((url, -score))
        
        return result
    
    def clear_graph(self):
        self.adj_list.clear()
        self.list_of_sites.clear()
        self.start_node = None
    
    def __del__(self):
        self.clear_graph()

if __name__ == "__main__":
    url = input("Enter URL to news: ")
    graph = Graph()
    graph.create(url)
    final_score = graph.get_score()
    top_sites = graph.get_top_sites()
    
    print(f"Trust Score: {final_score}")
    print("You can get the most trusted information from:")
    for site, score in top_sites:
        print(f"{site} {score:.2f}")