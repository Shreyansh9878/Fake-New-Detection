import requests
from bs4 import BeautifulSoup
from fake_useragent import UserAgent

def get_random_user_agent():
    try:
        ua = UserAgent()
        return ua.random
    except Exception as e:
        print(f"Error generating random User-Agent: {e}")
        return "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/58.0.3029.110 Safari/537.36"

def scrape_news(url, filename):
    headers = {
        'User-Agent': get_random_user_agent(),
        'Accept-Language': 'en-US,en;q=0.9',
        'Accept-Encoding': 'gzip, deflate, br',
        'Connection': 'keep-alive',
        'Referer': 'https://www.google.com'
    }
    
    response = requests.get(url, headers=headers)
    
    if response.status_code == 200:
        soup = BeautifulSoup(response.text, 'html.parser')

        # Remove unwanted tags
        for tag in soup(['script', 'style', 'video', 'iframe', 'source', 'nav', 'footer', 'aside']):
            tag.decompose()

        # Extract the summary dynamically
        summary_section = None
        summary_heading = soup.find(lambda tag: tag.name in ["h2", "h3"] and "Summary" in tag.get_text())

        if summary_heading:
            summary_section = summary_heading.find_next_sibling()
        
        summary_text = summary_section.get_text(separator='\n', strip=True) if summary_section else "No summary available."

        # Try to extract the main article content
        main_content = soup.find('article') or soup.find('main')
        if not main_content:
            print("Could not find main article content. Saving full cleaned text instead.")
            main_content = soup.body

        # Extract and clean text
        text_content = main_content.get_text(separator='\n', strip=True)

        with open(filename, 'w', encoding='utf-8') as file:
            file.write(f"Summary:\n{summary_text}\n\nNews Content:\n{text_content}")

        print(f"Saved news content and summary to {filename}")
    else:
        print(f"Failed to retrieve the webpage. Status code: {response.status_code}")

if __name__ == "__main__":
    news_url = "https://www.bbc.com/news/live/cz9e875gd11t"  # Example news URL
    output_file = "NEWSDATA.txt"
    scrape_news(news_url, output_file)
