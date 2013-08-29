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

locale_conv = {
    'eng': 'en_US.UTF-8',
    'fra': 'fr_FR.UTF-8',
    'esp': 'es_ES.UTF-8',
    }


def clean_date(local_date, language):

    buf = re.sub(r"\r?\n? de", "", local_date)
    buf = re.sub(" +", " ", buf.strip())
    return buf.replace("\xc2\xa0", " ")

def clean_title(title):
    buf = re.sub(r"\r?\n?", "", title)
    return buf.strip()

def convert_to_mysql_date(local_date, language):
    locale.setlocale(locale.LC_ALL, locale_conv[language])
    try:
        date = datetime.strptime(local_date, '%d %B %Y')
    except ValueError:
        try:
            date = datetime.strptime(local_date, '%B %Y')
        except ValueError:
            date = datetime.strptime(local_date, '%Y')

    return date.strftime('%Y-%m-%d')

def download(url, language):
    html = urlopen(url).read()
    soup = BeautifulSoup(html, "html.parser")
    if soup.find("div", { "class" : "khaki2" }):
        main = soup.find("div", { "class" : "khaki2" })
    else:
        main = soup.find("div", id="main")
    try:
        urls = [(a.get('href'), a.get_text()) for a in main.find_all('a')]
    except IndexError as er:
        print er
        import pdb; pdb.set_trace()
    results = []
    for url, content in urls:
        news = {}
        local_date = clean_date(content.split(':')[0].encode('utf-8'), language)
        title = clean_title(content.split(':', 1)[-1])
        news['title'] = title
        news['link'] = url.strip()
        news['language'] = language
        try:
            date = convert_to_mysql_date(local_date, language)
        except ValueError as err:
            # We're apending text for duplicated links
            continue
        else:
            news['date'] = date

        results.append(news)

    return results

def extract_lang(url):
    for k, v in locale_conv.items():
        if k in url:
            return k

def to_json(filename, results):
    f = open(filename, "w")
    f.write(json.dumps(results, indent=4))
    f.close()

if __name__ == '__main__':

    parser = argparse.ArgumentParser()
    parser.add_argument('urls', nargs ='+', help="takes one or more urls")
    parser.add_argument('-o', '--output', default="external_news.json", help="output file where to save the data")
    args = parser.parse_args()

    news = []

    for url in args.urls:
        lang = extract_lang(url)
        news.extend(download(url, lang))

    to_json(args.output, news)
    print "{} records inserted".format(len(news))
