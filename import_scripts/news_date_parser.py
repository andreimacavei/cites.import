# coding=utf-8
#!/usr/bin/python

import re
from datetime import datetime
import locale
from urllib2 import urlopen
from urlparse import urljoin, urlsplit

import argparse
import json

from bs4 import BeautifulSoup

misspelled_months = {
    'fevrier': 'février',
    'january': 'janvier',
    'february': 'février',
    'march': 'mars',
    'april': 'avril',
    'may': 'mai',
    'june': 'juin',
    'july': 'juillet',
    'august': 'août',
    'september': 'septembre',
    'october': 'octobre',
    'november': 'novembre',
    'december': 'décembre',
    }

locale_dict = {
    'eng': 'en_US.UTF-8',
    'fra': 'fr_FR.UTF-8',
    'esp': 'es_ES.UTF-8',
    }

def clean_date(local_date, language):
    for k, v in misspelled_months.items():
        if local_date.lower().find(k) != -1 and language == 'fra':
            local_date = local_date.lower().replace(k, v)
        elif language == 'eng' and local_date.lower().find(v) != -1:
            local_date = local_date.lower().replace(v, k)

    buf = re.sub(r"\r?\n? de", "", local_date)
    buf = re.sub(" +", " ", buf.strip())
    return buf.replace("\xc2\xa0", " ")


def get_parent_directory(url):
    parent_url = '/'.join(url.split('/')[:-1]) + '/'
    return parent_url

def convert_to_mysql_date(local_date, language):
    locale.setlocale(locale.LC_ALL, locale_dict[language])
    try:
        date = datetime.strptime(local_date, '%d %B %Y')
    except ValueError:
        try:
            date = datetime.strptime(local_date, '%B %Y')
        except ValueError:
            local_date = local_date.split()[-1]
            if any(char.isdigit() for char in local_date):
                date = datetime.strptime(local_date, '%Y')
            else:
                return
    return date.strftime('%Y-%m-%d')

def download(url, language):
    html = urlopen(url).read()
    soup = BeautifulSoup(html, "html.parser")
    table = soup.find('table')

    parent_url = get_parent_directory(url)
    urls = [(urljoin(parent_url, a.get('href')), a.contents[0]) for a in table.find_all('a')]

    results = {}
    for url, title in urls:
        local_date = clean_date(title.split(':')[0].encode('utf-8'), language)
        rel_url = urlsplit(url).path
        try:
            results[rel_url] = convert_to_mysql_date(local_date, language)
        except ValueError:
            year = rel_url.split('/')[-2]
            if not year in local_date: 
                local_date = local_date + ' {}'.format(year)
            results[rel_url] = convert_to_mysql_date(local_date, language)

    return results

def to_json(filename, results, mode):
    f = open(filename, mode)
    f.write(json.dumps(results, indent=4))
    f.close()

def read_input(filename):
    json_data = open(filename)
    data = json.load(json_data)
    json_data.close()
    return data

if __name__ == '__main__':

    parser = argparse.ArgumentParser()
    parser.add_argument('-i', '--input', help="json file with urls and language")
    parser.add_argument('-u', '--url')
    parser.add_argument('-l', '--language')
    parser.add_argument('-o', '--output', default='news.json', help="output file where to save the data")
    args = parser.parse_args()

    news = {}
    write_mode = 'w'

    if args.input:
        input_data = read_input(args.input)
        for url, lang in input_data.items():
            news.update(download(url, lang))
    elif args.url:
        write_mode = 'a'
        news = download(args.url, args.language)

    to_json(args.output, news, write_mode)
    print "{} records inserted".format(len(news))
