#!/usr/bin/python

import argparse
import json

from urllib2 import urlopen
from bs4 import BeautifulSoup
from urlparse import urljoin, urlsplit

BASE_URL = "http://www.cites.org/eng/news/pr/"

def download():
    import pdb;pdb.set_trace()
    html = urlopen(BASE_URL + 'index.php').read()
    soup = BeautifulSoup(html)
    urls = [a.get('href') for a in soup.find_all('a')]
    for url in urls:
        print url
    print len(urls)
    
#    absolute_urls = [urljoin(BASE_URL, url) for url in urls]
#    for url in absolute_urls:
#        print url


if __name__ == '__main__':

    parser = argparse.ArgumentParser()
    parser.add_argument('-u', '--url', default='')

    download()
