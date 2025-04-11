import numpy as np
import pandas as pd
import heapq
from collections import defaultdict
from urllib.parse import urlparse
import Tokenizer_Trie as tk_t
import web_scrapper as ws

DEPTH_THRESHOLD = 5
INFLUENCE = 0.85

def is_valid_url(url):
    return isinstance(url, str) and url.startswith(('http://', 'https://'))

def get_site(url):
    if is_valid_url(url):
        parsed_url = urlparse(url)
        domain = parsed_url.netloc
        if domain.startswith("www."):
            domain = domain[4:]

        if domain.startswith("edition"): #cnn is edition.cnn
            domain = domain[8:]
        
        return domain.split('.')[0].lower()
    
    else:
        return ""

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
        
    def __load_csv(self, path):
        data = pd.read_csv(path)
        columns = data.columns.tolist()
        data = data.set_index(columns[0])
        return data
    
    def __get_score_from_data(self, site_name):
        data = self.__load_csv("data.csv")
        if site_name in data.index:
            return data.loc[site_name.lower(), "Scores"]
        else:
            return 0.5
    
    def __append(self, site_name, url, prev_node=None, edge_weight=0):
        # get score from data.csv
        score = self.__get_score_from_data(site_name)

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
    
    def __create_graph(self, url, prev_node=None, original_flag = False):
        if not is_valid_url(url):
            return
        
        else:
            site_name = get_site(url)
            
            if site_name in self.list_of_sites:
                flg = False
                for edge in self.adj_list[prev_node]:
                    if edge[0].site_name == site_name or site_name == prev_node.site_name:
                        flg = True
                        break
                if not flg:
                    self.adj_list[prev_node].append((self.list_of_sites[site_name].node, self.list_of_sites[site_name].weight))
                return
            
            else:
                data, flag = ws.get_page(url, original_flag)
                weight = 0.5
                links=[]

                if len(data["headline"])>0:
                    links = data["citations"]
                    content = data["content"]
                    weight = tk_t.compare_data(self.trie_obj, content)
                    
                new_node = self.__append(site_name, url, prev_node, weight)
                    
                if new_node.depth < DEPTH_THRESHOLD and not (original_flag and len(links)==0):
                    for link in links:
                        self.__create_graph(link, new_node, original_flag)
            
    def __dfs_visit(self, node, ancestor):
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
                propagated_score += self.__dfs_visit(neighbor, ancestor) * weight
        
        if total_weights:
            score += (1 - INFLUENCE) * (propagated_score / total_weights)
        
        node.T_node = score
        ancestor[node] = False
        return score
    
    def create(self, url):
        if not is_valid_url(url):
            return
        
        data, flag = ws.get_page(url)
        links = data["citations"]
        original_news = data["content"]
        self.trie_obj = tk_t.extract_keywords(original_news, True)

        site_name = get_site(url)
        new_node = self.__append(site_name, url, None, 1)
        
        for link in links:
            if (get_site(link) != site_name):
                self.__create_graph(link, new_node, flag)
    
    def get_score(self):
        ancestor = {}
        return self.__dfs_visit(self.start_node, ancestor)
    
    def get_top_sites(self):
        pq = [(-site.node.T_node, site.url) for site in self.list_of_sites.values()]
        heapq.heapify(pq)
        
        result = []
        i = 5
        while (i):
            if not pq:
                break
            score, url = heapq.heappop(pq)
            if url:
                i-=1
                result.append((url, -score))
        
        return result
    
    def clear_graph(self):
        self.adj_list.clear()
        self.list_of_sites.clear()
        self.start_node = None
    
    def __del__(self):
        self.clear_graph()

if __name__ == "__main__":
    url = input("Enter URL:")
    graph = Graph()
    graph.create(url)

    final_score = graph.get_score()
    top_sites = graph.get_top_sites()
    
    print(final_score)