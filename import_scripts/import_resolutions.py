#!/bin/usr/python

import re

from urllib2 import urlopen
from urlparse import urljoin, urlsplit
import json

from bs4 import BeautifulSoup, UnicodeDammit
from bs4.element import Tag

def clean_text(text):
    buf = re.sub(r"\r?\n?\t?", "", text)
    buf = re.sub(" +", " ", buf.strip())
    return buf.encode('latin-1')

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
                try:
                    table_header = str(clean_text(row.th.get_text()))
                except:
                    import pdb; pdb.set_trace()
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
                resolution['number'] = cells[0].get_text()
                resolution['title'] = cells[1].get_text()
                document = {}
                document['description'] = cells[1].get_text()
                document['link'] = cells[2].a.get('href')
                resolution['documents'] = []
                resolution['documents'].append(document)

                try:
                    meeting[table_header].append(resolution)
                except KeyError as err:
                    print err
                    import pdb; pdb.set_trace()
    records.append(meeting)
    return records

if __name__ == '__main__':
    url = "http://www.cites.org/eng/res/"
    records = download(url)
    print records
