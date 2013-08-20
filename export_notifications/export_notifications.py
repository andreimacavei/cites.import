#!/usr/bin/python

import re

from urllib2 import urlopen
from urlparse import urljoin, urlsplit
import json

from bs4 import BeautifulSoup
from bs4.element import Tag

from tidylib import tidy_document

BASE_URL = "http://www.cites.org/eng/notif/"

def clean_text(text):
    buf = re.sub(r"\r?\n?\t?", "", text)
    buf = re.sub(" +", " ", buf.strip())
    return buf

def get_correct_table(soup):
    table = soup.find("table", { "class": "knoir" })
    if not table:
        table = soup.find("table", id="demo_table")
        if not table:
            table = soup.find_all("table")[2]
    return table

def download(url):
    html = urlopen(url).read()
    html_cleaned, errors = tidy_document(html)
    soup = BeautifulSoup(html_cleaned, "html.parser")
    table = get_correct_table(soup)
    # depending if table has class="knoir" attr, we get the right table data
    if table.has_attr("class") and table['class'] == "knoir":
        table_data = [tr for tr in table.find_all("tr")]
    else:
        table_data = [tr for tr in table.find_all("tr")][1:]

    results = []
    notif = {}
    for row in table_data:
        # we skip new lines
        if row.find('\n') == 0:
            continue
        cells = row.findChildren("td")
        if not cells:
            continue
        # verify if current row expands on all columns or just only Title column
        document = {}
        if len(cells) > 3:
            if notif:
                results.append(notif)
                notif = {}
            notif['number'] = clean_text(cells[0].text)
            notif['date'] = clean_text(cells[1].text)
            status = cells[2].img.get('src')

            if '/valid' in status:
                notif['status'] = 'valid'
            elif '/invalid' in status:
                notif['status'] = 'invalid'
            notif['documents'] = []

            if cells[3].a:
                rel_link = cells[3].a.get('href')
            else:
                try:
                    rel_link = cells[4].a.get('href')
                except:
                    document['link'] = ''

            if '/common/' in rel_link:
                parent_url = '/'.join(url.split('/')[:3])
                document['link'] = urljoin(parent_url, '/'.join(rel_link.split('/')[2:]))
            elif rel_link:
                parent_url = '/'.join(url.split('/')[:-1])
                document['link'] = urljoin(parent_url, rel_link)

            document['title'] = clean_text(cells[3].text)
        else:
            try:
                rel_link = cells[0].a.get('href')
            except AttributeError:
                rel_link = cells[1].a.get('href')

            if 'common' in rel_link:
                parent_url = '/'.join(url.split('/')[:3])
                document['link'] = urljoin(parent_url, '/'.join(rel_link.split('/')[2:]))
                document['title'] = clean_text(cells[0].text)
            else:
                parent_url = '/'.join(url.split('/')[:-1])
                document['link'] = urljoin(parent_url, rel_link)
                document['title'] = clean_text(cells[0].text)
        notif['documents'].append(document)
    return results

def to_json(filename, results):
    f = open(filename, "w")
    f.write(json.dumps(results, indent=4))
    f.close()

if __name__ == '__main__':

    notifications = download("http://www.cites.org/eng/notif/2010.shtml")
    to_json('eng_notif_2010', notifications)
