import requests
from bs4 import BeautifulSoup
import re
from fake_useragent import UserAgent
from googlesearch import search
import urllib.parse
import time

def get_random_user_agent():
    try:
        ua = UserAgent()
        return ua.random
    except Exception:
        return "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/58.0.3029.110 Safari/537.36"

def extract_external_citations(soup, main_content, base_url):
    base_domain = urllib.parse.urlparse(base_url).netloc.replace("www.", "")
    citations = []

    for a_tag in main_content.find_all("a", href=True):
        href = a_tag["href"]
        text = a_tag.get_text(strip=True)
        full_link = urllib.parse.urljoin(base_url, href)
        link_domain = urllib.parse.urlparse(full_link).netloc.replace("www.", "")

        if len(text) > 3 and base_domain not in link_domain and link_domain:
            citations.append((text, full_link))
    
    return citations

def scrape_news(url):
    headers = {
        'User-Agent': get_random_user_agent(),
        'Accept-Language': 'en-US,en;q=0.9',
        'Accept-Encoding': 'gzip, deflate, br',
        'Connection': 'keep-alive',
        'Referer': 'https://www.google.com'
    }

    response = requests.get(url, headers=headers)
    if response.status_code != 200:
        raise LookupError("Failed to retrieve page")

    soup = BeautifulSoup(response.text, 'html.parser')

    # Remove noise
    for tag in soup(['script', 'style', 'video', 'iframe', 'source', 'nav', 'footer', 'aside']):
        tag.decompose()

    # Headline
    headline = soup.title.string.strip() if soup.title else "No title found"

    # Main content
    main_content = soup.find('article') or soup.find('main') or soup.body
    if not main_content:
        return {"headline": headline, "content": "", "citations": []}

    text_content = main_content.get_text(separator='\n', strip=True)
    if not text_content.strip():
        return {"headline": headline, "content": "", "citations": []}

    # Citations
    citations = extract_external_citations(soup, main_content, url)

    return {
        "headline": headline,
        "content": text_content,
        "citations": citations
    }

def find_news_sources(news_title, num_results=10):
    query = f'"{news_title}"'  # Using quotes for exact match search
    results = search(query, num_results=num_results)
    time.sleep(2)
    
    sources = []
    for url in results:
        sources.append(url)

    return sources

def get_page(url):
    data = scrape_news(url)
    flag = True
    
    if len(data["citations"]) == 0:
        data["citations"] = find_news_sources(data["headline"])
        flag = True

    return data, flag
