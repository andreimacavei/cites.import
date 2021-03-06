#!/bin/usr/python

import re

from urllib2 import urlopen
from urlparse import urljoin, urlsplit
import json

from bs4 import BeautifulSoup, UnicodeDammit
from bs4.element import Tag

URLS = [
    "http://www.cites.org/eng/res/",
    "http://www.cites.org/esp/res/",
    "http://www.cites.org/fra/res/",
]

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
    tables = soup.find_all("table", { "class": "khaki"})

    records = []
    for table in tables:
        table_data = [tr for tr in table.find_all("tr")]
        meeting = {}
        for row in table_data:
            if row.find('\n') == 0:
                continue
            if row.th:
                table_header = str(clean_text(row.th.get_text()))
                if not meeting:
                    meeting[table_header] = []
                else:
                    records.append(meeting)
                    meeting = {}
                    meeting[table_header] = []
            else:
                resolution = {}
                cells = row.findChildren("td")
                if not cells:
                    print "No <td> tags found in row %s\n" % (row, )
                    continue
                resolution['number'] = clean_text(cells[0].get_text())
                resolution['title'] = clean_text(cells[1].get_text())
                document = {}
                document['description'] = clean_text(cells[1].get_text())

                base_url = url.split('/', 3)[-1]
                rel_url = cells[1].a.get('href')
                if len(rel_url.split('/')) > 3:
                    abs_url = url + '/'.join(rel_url.split('/')[2:])
                    rel_link = base_url + '/'.join(rel_url.split('/')[2:])
                else:
                    abs_url = url + rel_url
                    rel_link = base_url + rel_url
                resolution['link'] = rel_link

                doc_rel = cells[2].a.get('href')
                if len(doc_rel.split('/')) > 3:
                    doc_abs = base_url + '/'.join(doc_rel.split('/')[2:])
                else:
                    doc_abs = base_url + doc_rel
                document['link'] = doc_abs
                if abs_url.find('.pdf') < 0:
                    resolution['body'] = clean_text(get_resolution(abs_url))
                resolution['documents'] = []
                resolution['documents'].append(document)
                meeting[table_header].append(resolution)

    records.append(meeting)
    return records

def to_json(filename, results):
    f = open(filename, "w")
    json.dump(results, f, ensure_ascii=False, indent=4)
    f.close()

if __name__ == '__main__':

    for url in URLS:
        records = download(url)
        filename = url.split('/')[3] + "_resolutions.json"
        to_json(filename, records)
