#!/bin/usr/python

import re

from urllib2 import urlopen
from urlparse import urljoin, urlsplit
import json

from bs4 import BeautifulSoup, UnicodeDammit
from bs4.element import Tag

def utStripMSWordUnicode(buf):
    buf = buf.replace(u"\u2013", "-")
    buf = buf.replace(u"\u2014", "-")
    buf = buf.replace(u"\u2011", "-")
    buf = buf.replace(u"\u2019", "'")
    buf = buf.replace(u"\u2018", "'")
    buf = buf.replace(u"\u0153", "oe")
    buf = buf.replace(u"\u201a", ",")
    buf = buf.replace(u"\u201c", '"')
    buf = buf.replace(u"\u201d", '"')
    buf = buf.replace(u"\u2026", "...")
    return buf

def clean_text(text):
    buf = utStripMSWordUnicode(text)
    buf = re.sub(r"\r?\n?\t?", "", buf)
    buf = re.sub(" +", " ", buf.strip())
    try:
        return buf.encode('latin-1')
    except UnicodeEncodeError as err:
        return buf.encode('utf-8')

def get_resolution(url):
    html = urlopen(url).read()
    soup = BeautifulSoup(html, "html.parser")
    data = soup.find("div", id="milieu")
    if not data:
        data = soup.find("blockquote")
        if not data:
            data = soup.find_all("table")[1]
    return data.prettify(formatter="html")

def download(url):
    html = urlopen(url).read()
    soup = BeautifulSoup(html, "html.parser")
    table = soup.find("table")
    table_rows = [tr for tr in table.find_all("tr")]
    table_headers = [th for th in table.find_all('th')]

    records = [] # list of all records
    decisions = {} # main records (table header)
    group_decisions = {}
    contor = 0
    for row in table_rows:
        if row.find('\n') == 0:
            continue

        if contor in (0, 3, 8) and row.th:
            main_header = str(clean_text(row.th.get_text()))
            if not decisions:
                decisions[main_header] = []
            else:
                records.append(decisions)
                decisions = {}
                decisions[main_header] = []
            contor = contor + 1
        else:
            if row.th:
                table_header = str(clean_text(row.th.get_text()))
                if not group_decisions:
                    group_decisions[table_header] = []
                else:
                    decisions[main_header].append(group_decisions)
                    group_decisions = {}
                    group_decisions[table_header] = []
                contor = contor + 1
            else:
                resolution = {}
                cells = row.findChildren("td")
                resolution['number'] = clean_text(cells[0].get_text())
                resolution['title'] = clean_text(cells[1].get_text())
                rel_url = cells[1].a.get('href')
                abs_url = url + rel_url
                resolution['body'] = get_resolution(abs_url)
                group_decisions[table_header].append(resolution)
    # we're appending annexes
    decisions[main_header].append(group_decisions)
    records.append(decisions)
    return records

def to_json(filename, results):
    f = open(filename, "w")
    json.dump(results, f, ensure_ascii=False, indent=4)
    f.close()

if __name__ == '__main__':
    url = "http://www.cites.org/eng/dec/"
    records = download(url)
    filename = "decisions.json"
    to_json(filename, records)
