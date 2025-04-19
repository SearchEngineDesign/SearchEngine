import time
import random
import json
import argparse
import os
from selenium import webdriver
from selenium.webdriver.chrome.service import Service
from selenium.webdriver.common.by import By
from selenium.webdriver.chrome.options import Options
from selenium.webdriver.support.ui import WebDriverWait
from selenium.webdriver.support import expected_conditions as EC
from webdriver_manager.chrome import ChromeDriverManager


class SearchCrawler:
    def __init__(self, proxy=None):
        self.proxy = proxy
        self.init_driver()
        self.seach_engine = ["google", "bing", "yahoo", "ecosia"]

    def init_driver(self):
        """Initialize the Selenium driver"""
        chrome_options = Options()
        chrome_options.add_argument("--no-sandbox")
        chrome_options.add_argument("--disable-dev-shm-usage")
        chrome_options.add_argument("--disable-blink-features=AutomationControlled")
        chrome_options.add_experimental_option("excludeSwitches", ["enable-automation"])
        chrome_options.add_experimental_option("useAutomationExtension", False)
        chrome_options.add_argument("user-agent=Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/111.0.0.0 Safari/537.36")
        if self.proxy:
            chrome_options.add_argument(f'--proxy-server={self.proxy}')
            # print(f"🔌 Using proxy: {self.proxy}")

        
        service = Service(ChromeDriverManager().install())
        self.driver = webdriver.Chrome(service=service, options=chrome_options)

        # Anti-bot: Execute CDP commands
        self.driver.execute_cdp_cmd("Network.setUserAgentOverride", {
            "userAgent": "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/111.0.0.0 Safari/537.36"
        })
        self.driver.execute_cdp_cmd("Page.addScriptToEvaluateOnNewDocument", {
            "source": """
                Object.defineProperty(navigator, 'webdriver', { get: () => undefined })
            """
        })

    def restart_driver(self):
        """Restart the browser driver"""
        try:
            self.driver.quit()
        except Exception:
            pass
        self.init_driver()
    
    def find_engine(self, query_index):
        """Find the search engine to use based on the query index"""
        if query_index % 4 == 0:
            return self.seach_engine[0]
        elif query_index % 4 == 1:
            return self.seach_engine[1]
        elif query_index % 4 == 2:
            return self.seach_engine[2]
        # elif query_index % 4 == 3:
        #     return self.seach_engine[3]
        else:
            return self.seach_engine[3]


    def parse(self, engine_name, results, page_num, query):
        if engine_name == "google":
            start = page_num * 10
            url = f"https://www.{engine_name}.com/search?q={query.replace(' ', '+')}&start={start}"
            # print(f"query: {query}, page {page_num + 1}: {url}")

            self.driver.get(url)

            WebDriverWait(self.driver, 10).until(
                EC.presence_of_element_located((By.CSS_SELECTOR, "div.MjjYud"))
            )
            
            # time.sleep(random.uniform(3, 10))
            # WebDriverWait(self.driver, 10).until(
            #     EC.presence_of_element_located((By.CSS_SELECTOR, "#search"))
            # )
            mjjyud_elements = self.driver.find_elements(By.CSS_SELECTOR, "div.MjjYud")
            
            if mjjyud_elements:
                for position, mjjyud_element in enumerate(mjjyud_elements):
                    try:
                        specific_elements = mjjyud_element.find_elements(By.CSS_SELECTOR, '[jsname="UWckNb"].zReHs')
                        for specific_element in specific_elements:
                            href = specific_element.get_attribute('href')
                            if href:
                                results.append({
                                    "href": href
                                })
                    except Exception as e:
                        print(f"Error processing a result: {e}")

        elif engine_name == "bing":
            start = page_num * 10 + 1
            url = f"https://www.{engine_name}.com/search?q={query.replace(' ', '+')}&first={start}"
            # print(f"query: {query}, page {page_num + 1}: {url}")

            self.driver.get(url)

            # time.sleep(random.uniform(3, 10))
            WebDriverWait(self.driver, 10).until(
                EC.presence_of_element_located((By.CSS_SELECTOR, "li.b_algo"))
            )
            mjjyud_elements = self.driver.find_elements(By.CSS_SELECTOR, "li.b_algo")
            # with open("raw_results.html", "w", encoding="utf-8") as f:
            #     for element in mjjyud_elements:
            #         html = element.get_attribute("outerHTML")
            #         f.write(html + "\n\n")

            if mjjyud_elements:
                for position, mjjyud_element in enumerate(mjjyud_elements):
                    try:
                        specific_elements = mjjyud_element.find_elements(By.TAG_NAME, "h2")
                        specific_elements = specific_elements[0].find_elements(By.TAG_NAME, "a")

                        for specific_element in specific_elements:
                            href = specific_element.get_attribute('href')
                            if href:
                                results.append({
                                    "href": href
                                })
                    except Exception as e:
                        print(f"Error processing a result: {e}")
        elif engine_name == "yahoo":  
            start = page_num * 10 + 1
            url = f"https://www.search.{engine_name}.com/search?p={query.replace(' ', '+')}&b={start}"
            # print(f"query: {query}, page {page_num + 1}: {url}")

            self.driver.get(url)

            WebDriverWait(self.driver, 10).until(
                EC.presence_of_element_located((By.CSS_SELECTOR, "div.compTitle.options-toggle"))
            )

            mjjyud_elements = self.driver.find_elements(By.CSS_SELECTOR, "div.compTitle.options-toggle")

            if (mjjyud_elements == []):
                print("not found")

            # Find all result elements
            if mjjyud_elements:
                for position, mjjyud_element in enumerate(mjjyud_elements):
                    try:
                        specific_elements = mjjyud_element.find_elements(By.TAG_NAME, "h3")
                        specific_elements = specific_elements[0].find_elements(By.TAG_NAME, "a")
                        # with open("raw_results_2.html", "w", encoding="utf-8") as f:
                        #     for element in specific_elements:
                        #         html = element.get_attribute("outerHTML")
                        #         f.write(html + "\n\n")
                        for specific_element in specific_elements:
                            href = specific_element.get_attribute('href')
                            # print("Anchor URL:", href)
                            if href:
                                results.append({
                                    "href": href
                                })
                    except Exception as e:
                        print(f"Error processing a result: {e}")

        # elif engine_name == "brave":
        #     start = page_num
        #     url = f"https://search.{engine_name}.com/search?q={query.replace(' ', '+')}&offset={start}"
        #     # print(f"query: {query}, page {page_num + 1}: {url}")

        #     self.driver.get(url)

        #     WebDriverWait(self.driver, 10).until(
        #         EC.presence_of_element_located((By.CSS_SELECTOR, "#results"))
        #     )

        #     mjjyud_elements = self.driver.find_elements(By.CSS_SELECTOR, 'div.snippet[data-type="web"]')
            

        #     # Find all result elements
        #     if mjjyud_elements:
        #         for position, mjjyud_element in enumerate(mjjyud_elements):
        #             try:
        #                 specific_elements = mjjyud_element.find_elements(By.CSS_SELECTOR, "a[target='_self']")
                        
        #                 for specific_element in specific_elements:
        #                     href = specific_element.get_attribute('href')
        #                     # print("Anchor URL:", href)
        #                     if href:
        #                         results.append({
        #                             "href": href
        #                         })
        #             except Exception as e:
        #                 print(f"Error processing a result: {e}")

        elif engine_name == "ecosia":
            start = page_num
            # https://www.ecosia.org/search?q=test&p=10
            url = f"https://www.{engine_name}.org/search?q={query.replace(' ', '+')}&p={start}"
            # print(f"query: {query}, page {page_num + 1}: {url}")

            self.driver.get(url)

            WebDriverWait(self.driver, 10).until(
                EC.presence_of_element_located((By.CSS_SELECTOR, 'div[data-test-id="mainline-result-web"]'))
            )
            
            mjjyud_elements = self.driver.find_elements(By.CSS_SELECTOR, 'div[data-test-id="mainline-result-web"]')
            if mjjyud_elements:
                for position, mjjyud_element in enumerate(mjjyud_elements):
                    try:
                        specific_elements = mjjyud_element.find_elements(By.CSS_SELECTOR, "div.result__title")
                        for specific_element in specific_elements:
                            element = specific_element.find_elements(By.TAG_NAME, "a")
                            href = element[0].get_attribute('href')
                            if href:
                                results.append({
                                    "href": href
                                })
                    except Exception as e:
                        print(f"Error processing a result: {e}")
                        
        else:
            print(f"Unknown search engine: {engine_name}")
        

    def crawl_results(self, query_index, query, page_num):
        engine_name = self.find_engine(query_index)

        time_start = time.time()
        results = []
        try:
            print(f"Search Engine: '{engine_name}'")
            # parse
            self.parse(engine_name, results, page_num, query)
                
            # time.sleep(random.uniform(4, 8))

            # print(f"Time taken for query '{query}': {time.time() - time_start:.2f} seconds")
            return results

        except Exception as e:
            print(f"Error crawling search results: {e}")
            raise e


def write_index_to_file(query_index, page_num, filename):
    with open(filename, 'w') as f:
        f.write(str(query_index) + '\n')
        f.write(str(page_num) + '\n')
        print(f"Writing failed index to {filename} at index {query_index} and page {page_num}")
        f.close()




def run_crawler():
    parser = argparse.ArgumentParser(description='Search Results Crawler')
    parser.add_argument('query_file', type=str, help='Path to .txt file containing search queries (one per line)')
    parser.add_argument('--output_file', type=str, default='search_results.jsonl', help='Output file path for results (JSONL format)')
    args = parser.parse_args()
    
    
    try:
        crawler = SearchCrawler()
    except Exception as e:
        print(f"Failed to initialize the crawler: {e}")
        raise


    # The first line is the index of the last completed query
    # The second line is the page number of the last completed query
    failed_index_file = 'recovery_info.txt'
    if os.path.exists(failed_index_file):
        with open(failed_index_file, 'r') as f:
            lines = f.readlines()
            prev_completed_index = int(lines[0].strip())
            prev_completed_page = int(lines[1].strip())
        print(f"Resuming from index: {prev_completed_index}, page: {prev_completed_page}")
    else:
        prev_completed_index = 0
        prev_completed_page = 0

    crt_completed_index = 0
    crt_completed_page = 0

    print(f"Already completed queries: {prev_completed_index}")

    with open(args.query_file, 'r', encoding='utf-8') as f:
        queries = [line.strip() for line in f if line.strip()]
    
    try:
        with open(args.output_file, 'a', encoding='utf-8') as outfile:
            page_num_max = 8
            for page_num in range(0, page_num_max):
                if page_num < prev_completed_page:
                    print(f"Skipping page {page_num + 1} (already completed)")
                    continue
                crt_completed_page = page_num
                for query_index, query in enumerate(queries):
                    time_start = time.time()
                    
                    if query_index < prev_completed_index:
                        continue
                    crt_completed_index = query_index
                    MAX_RETRIES = 4
                    success = False
                    has_results = False
                    for attempt in range(1, MAX_RETRIES + 1):
                        try:
                            print(f"Attempt {attempt}/{MAX_RETRIES} for query #{query_index}: '{query}'")
                            results = crawler.crawl_results(query_index, query, page_num)
                            success = True
                            if results:
                                has_results = True
                                break
                            # break
                             
                        except Exception as e:
                            print(f"Attempt {attempt} failed: {e}")
                            try:
                                crawler.restart_driver()
                            except Exception as e:
                                print(f"Restart Deriver failed: {e}")
                                write_index_to_file(crt_completed_index, crt_completed_page, failed_index_file)
                                crawler.driver.quit()
                                # exit(1)
                                raise

                            # time.sleep(random.uniform(3, 7))  # Optional small delay before retry

                    if not success:
                        print(f"‼️ All {MAX_RETRIES} attempts failed for query #{query_index}: '{query}'")
                        write_index_to_file(crt_completed_index, crt_completed_page, failed_index_file)
                        crawler.driver.quit()
                        # exit(1)
                        raise
                    
                    if not has_results:
                        print(f"‼️ No results found for query #{query_index}: '{query}'")
                        
                    output_line = {
                        'query': query,
                        'results': results
                    }
                    json.dump(output_line, outfile, ensure_ascii=False)
                    outfile.write('\n')
                    # print(f"Results for query '{query}' saved to {args.output_file}")
                   

                    time.sleep(random.uniform(1, 2))
                    # print(f"Time taken for query with save '{query}': {time.time() - time_start:.2f} seconds")
                    print(f"==================\n==================\n")
                
        
        crawler.driver.quit()
    except KeyboardInterrupt:
        print("\n[!] KeyboardInterrupt received. Exiting gracefully.")
        write_index_to_file(crt_completed_index, crt_completed_page, failed_index_file)
        crawler.driver.quit()
        # exit(1)
        raise
    except Exception as e:
        print(f"[!] Error during execution: {e}")
        write_index_to_file(crt_completed_index, crt_completed_page, failed_index_file)
        crawler.driver.quit()
        # exit(1)
        raise
    finally:
        print("[*] Final cleanup...")
        crawler.driver.quit()


def main():
    while True:
        try:
            run_crawler()
            break  # only break if everything finished successfully
        except KeyboardInterrupt:
            print("\n[!] KeyboardInterrupt received. Exiting gracefully.")
            return
            # exit(1)
        except Exception as e:
            print(f"[!] Error occurred: {e}")
            print("[*] Restarting the crawler automatically...\n")
            time.sleep(3)  # optional: wait before restarting

if __name__ == "__main__":
    main()