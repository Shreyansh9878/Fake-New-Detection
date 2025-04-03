import requests
from bs4 import BeautifulSoup
from fake_useragent import UserAgent

def scrape_news(url, filename):
    # Generate a random User-Agent
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
        
        # Extract all text content while avoiding video-related tags
        unwanted_tags = ['script', 'style', 'video', 'iframe', 'source']
        for tag in soup(unwanted_tags):
            tag.decompose()
        
        text_content = soup.get_text(separator='\n', strip=True)
        
        # Save text data to a file
        with open(filename, 'w', encoding='utf-8') as file:
            file.write(text_content)
        
        print(f"Saved text data to {filename}")
    else:
        print(f"Failed to retrieve the webpage. Status code: {response.status_code}")

# Example usage
if _name_ == "_main_":
    news_url = "https://www.bbc.com/news/live/cz9e875gd11t"  # Example news site
    output_file = "NEWSDATA.txt"
    scrape_news(news_url, output_file)