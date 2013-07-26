#!/usr/bin/python

import re
from datetime import datetime
import locale
from urllib2 import urlopen
from urlparse import urljoin, urlsplit

import argparse
import json

from bs4 import BeautifulSoup


def clean_date(local_date):
     buf = re.sub(r"\r?\n? de", "", local_date)
     return re.sub(" +", " ", buf.strip())

def get_parent_directory(url):
    parent_url = '/'.join(url.split('/')[:-1]) + '/'
    return parent_url

def convert_to_mysql_date(local_date, curr_locale):
    locale.setlocale(locale.LC_ALL, curr_locale)
    try:
        date = datetime.strptime(local_date, '%d %B %Y')
    except ValueError:
        date = datetime.strptime(local_date, '%B %Y')
    return date.strftime('%Y-%m-%d')

def download(url, current_locale):
    html = urlopen(url).read()
    soup = BeautifulSoup(html, "html.parser")
    table = soup.find('table')

    parent_url = get_parent_directory(url)
    urls = [(urljoin(parent_url, a.get('href')), a.contents[0]) for a in table.find_all('a')]

    results = {}
    for url, title in urls:
        local_date = clean_date(title.split(':')[0])
        rel_url = urlsplit(url).path
        try:
            results[rel_url] = convert_to_mysql_date(local_date, current_locale)
        except ValueError:
            year = rel_url.split('/')[-2]
            if not year in local_date: 
                local_date = local_date + ' {}'.format(year)
            results[rel_url] = convert_to_mysql_date(local_date, current_locale)

    return results

def to_json(filename, results):
    f = open(filename, "w")
    f.write(json.dumps(results, indent=4))
    f.close()

if __name__ == '__main__':

    parser = argparse.ArgumentParser()
    parser.add_argument('-u', '--url', default='http://www.cites.org/eng/news/pr/index.php', help="absolute path")
    parser.add_argument('-l', '--locale', default='en_US.UTF-8', help="example for english: 'en_US.UTF-8'")
    parser.add_argument('-o', '--output', default='news.json', help="output file to save the data")
    args = parser.parse_args()
    
    
    news = download(args.url, args.locale)
    to_json(args.output, news)

    print "{0} records inserted".format(len(news))
