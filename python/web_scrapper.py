import requests
from bs4 import BeautifulSoup
from fake_useragent import UserAgent
import re
import nltk
from nltk.tokenize import sent_tokenize
from nltk.corpus import stopwords

nltk.download("punkt")
nltk.download("stopwords")

def scrape_news(url, filename):
    ua = UserAgent()
    headers = {
        'User-Agent': ua.random,
        'Accept-Language': 'en-US,en;q=0.9',
        'Accept-Encoding': 'gzip, deflate, br',
        'Connection': 'keep-alive',
        'Referer': 'https://www.google.com'
    }
    
    response = requests.get(url, headers=headers)
    
    if response.status_code == 200:
        soup = BeautifulSoup(response.text, 'html.parser')
        
        # Remove unwanted tags like scripts, styles, and videos
        for tag in soup(['script', 'style', 'video', 'iframe', 'source', 'nav', 'footer', 'aside', 'form']):
            tag.decompose()
        
        # Extract only main article content
        article = soup.find('article') or soup.find('main') or soup.body
        text_content = article.get_text(separator='\n', strip=True) if article else soup.get_text(separator='\n', strip=True)
        
        # Cleanup extra whitespace and newlines
        text_content = re.sub(r'\n+', '\n', text_content).strip()
        
        # Save text data to a file
        with open(filename, 'w', encoding='utf-8') as file:
            file.write(text_content)
        
        print(f"Saved text data to {filename}")
        
        # Extract citations, quotes, and sources
        citations = extract_citations(soup)
        quotes = extract_quotes(text_content)
        sources = extract_organizations(text_content)
        
        print("\nExtracted Citations:")
        for text, link in citations:
            print(f"{text}: {link}")
        
        print("\nExtracted Quotes:")
        for quote in quotes:
            print(f"- {quote}")
        
        print("\nExtracted Sources:")
        for source in sources:
            print(f"- {source}")
    else:
        print(f"Failed to retrieve the webpage. Status code: {response.status_code}")

def extract_citations(soup):
    citations = []
    for a_tag in soup.find_all("a", href=True):
        link = a_tag["href"]
        text = a_tag.get_text(strip=True)
        if link.startswith("http") and len(text) > 3:
            citations.append((text, link))
    return citations

def extract_quotes(text):
    return re.findall(r'\"([^\"]+)\"|‘([^’]+)’', text)

def extract_organizations(text):
    words = text.split()
    stop_words = set(stopwords.words("english"))
    capitalized_words = [word for word in words if word.istitle() and word.lower() not in stop_words]
    return set(capitalized_words)

def get_data(url):
    link = []
    content = ''

    return link, content

# Example usage
if __name__ == "__main__":
    news_url = "https://www.bbc.com/news/live/cz9e875gd11t"
    output_file = "NEWSDATA.txt"
    scrape_news(news_url, output_file)


