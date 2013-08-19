#!/usr/bin/python

import re
from datetime import datetime
import locale

from urllib2 import urlopen
from urllib2 import HTTPError
from urlparse import urljoin, urlsplit
import json

from bs4 import BeautifulSoup
from bs4.element import Tag

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
            import pdb; pdb.set_trace()
    return table

def download(url):
    html = urlopen(url).read()
    soup = BeautifulSoup(html, "html.parser")
    table = get_correct_table(soup)
    results = []
    notif = {}
    for row in table.children:
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
            try:
                status = cells[2].img.get('src')
            except:
                import pdb; pdb.set_trace()

            if '/valid' in status:
                notif['status'] = 'valid'
            elif '/invalid' in status:
                notif['status'] = 'invalid'
            notif['documents'] = []
            parent_url = '/'.join(url.split('/')[:-1])
            document['link'] = urljoin(parent_url, cells[3].a.get('href'))
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

def download_old_format(url):
    html = urlopen(url).read()
    soup = BeautifulSoup(html, "html.parser")
    table = get_correct_table(soup)
    table_data = [tr for tr in table.find_all("tr")][1:]
    results = []
    notif = {}
    for row in table_data:
        #import pdb; pdb.set_trace()
        if row.find('\n') == 0:
            continue
        cells = row.findChildren("td")
        if not cells:
            continue
        # verify if current row expands on all columns or just only Title column
        document = {}
        if len(cells) >= 3:
            if notif:
                results.append(notif)
                notif = {}
            notif['number'] = clean_text(cells[0].text)
            notif['date'] = clean_text(cells[1].text)
            status = cells[2].img.get('src')
            if 'valid' in status:
                notif['status'] = 'valid'
            elif 'invalid' in status:
                notif['status'] = 'invalid'
            notif['documents'] = []
            parent_url = '/'.join(url.split('/')[:-1])
            if cells[3].a:
                rel_link = cells[3].a.get('href')
            else:
                rel_link = cells[4].a.get('href')
            if 'common' in rel_link:
                parent_url = '/'.join(url.split('/')[:3])
                document['link'] = urljoin(parent_url, '/'.join(rel_link.split('/')[2:]))
            else:
                parent_url = '/'.join(url.split('/')[:-1])
                document['link'] = urljoin(parent_url, rel_link)
            document['title'] = clean_text(cells[3].text)
        else:
            rel_link = cells[0].a.get('href')
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

    #notifications = download_old_format("http://www.cites.org/eng/notif/2010.shtml")
    notifications = download("http://www.cites.org/eng/notif/2012.php")
    to_json('eng_notif_2012', notifications)
