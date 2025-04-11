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
        
if __name__ == "__main__":
    url = input("Enter URL:")
    graph = Graph()
    graph.create(url)

    final_score = graph.get_score()
    top_sites = graph.get_top_sites()
    
    print(final_score)