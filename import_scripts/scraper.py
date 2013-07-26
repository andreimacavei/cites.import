#!/usr/bin/python

import argparse
import json

from urllib2 import urlopen
from bs4 import BeautifulSoup
from urlparse import urljoin, urlsplit

BASE_URL = "http://www.cites.org/eng/news/pr/"

def download():
    html = urlopen(BASE_URL + 'index.php').read()
    soup = BeautifulSoup(html, "html.parser")
    table = soup.find('table')
    urls = [(urljoin(BASE_URL, a.get('href')), a.contents[0]) for a in table.find_all('a')]

    for url in urls:
        print url
    print len(urls)


if __name__ == '__main__':

    parser = argparse.ArgumentParser()
    parser.add_argument('-u', '--url', default='')

    download()
