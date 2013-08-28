#!/usr/bin/python

import re

from urllib2 import urlopen
from urlparse import urljoin, urlsplit
import json

from bs4 import BeautifulSoup, UnicodeDammit
from bs4.element import Tag

from tidylib import tidy_document

BASE_OPTIONS = {

}

BASE_URLS = [
    "http://www.cites.org/eng/notif/2013.php",
    "http://www.cites.org/fra/notif/2013.php",
    "http://www.cites.org/esp/notif/2013.php",
]

def utStripMSWordUTF8(s):
    """ replace MSWord characters """
    s = s.replace('\\xe2\\x80\\xa6', '...') #ellipsis
    s = s.replace('\\xe2\\x80\\x93', '-')   #long dash
    s = s.replace('\\xe2\\x80\\x94', '-')   #long dash
    s = s.replace('\\xe2\\x80\\x98', '\'')  #single quote opening
    s = s.replace('\\xe2\\x80\\x99', '\'')  #single quote closing
    s = s.replace('\\xe2\\x80\\x9c', '"')  #single quote closing
    s = s.replace('\\xe2\\x80\\x9d', '"')  #single quote closing
    s = s.replace('\\xe2\\x80\\xa2', '*')  #dot used for bullet points
    return s

def utStripMSWordUnicode(buf):
    buf = buf.replace(u"\u2013", "-")
    buf = buf.replace(u"\u2019", "'")
    buf = buf.replace(u"\u2018", "'")
    buf = buf.replace(u"\u0153", "oe")
    buf = buf.replace(u"\u201a", ",")
    return buf

def clean_text(text):
    # buf = text.replace(u'\xe2\u20ac\u201c', '-')
    buf = utStripMSWordUnicode(text)
    buf = re.sub(r"\r?\n?\t?", "", buf)
    buf = re.sub(" +", " ", buf.strip())
    return buf.encode('latin-1')

def get_correct_table(soup, url):
    langs = [ 'esp', 'fra' ]
    lang = url.split('/')[3]
    table = soup.find("table", { "class": "knoir" })
    if not table:
        table = soup.find("table", id="demo_table")
        # if table has no class or id we get the corect table order depending on language
        if not table and lang in langs:
            table = soup.find_all("table")[3]
        elif not table:
            table = soup.find_all("table")[2]
    return table

def get_url_by_year(url):
    html = urlopen(url).read()
    soup = BeautifulSoup(html, "html.parser")
    table = soup.find("table", { "class": "nectar"})
    if not table:
        table = soup.find("table", { "class": "bggreen"})
    rel_urls = [a.get('href') for a in table.find_all('a')][1:]

    parent_url = '/'.join(url.split('/'))[:-1]
    urls = []
    for rel_url in rel_urls:
        url = urljoin(parent_url, rel_url)
        urls.append(url)
    return urls

def download(url):
    html = urlopen(url).read()
    # html_cleaned, errors = tidy_document(html)
    # html_cleaned = UnicodeDammit.detwingle(html)
    soup = BeautifulSoup(html, "html.parser")
    table = get_correct_table(soup, url)

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
            try:
                status = cells[2].img.get('src')
            except AttributeError as err:
                status  = ''

            if '/valid' in status:
                notif['status'] = 'valid'
            elif '/invalid' in status:
                notif['status'] = 'invalid'
            notif['documents'] = []

            if cells[3].a:
                rel_link = cells[3].a.get('href')
            else:
                # Sometimes we can find titles with no link atached and we get the url from last column then
                try:
                    rel_link = cells[4].a.get('href')
                except:
                    document['link'] = ''
                    rel_link = ''
            if '/common/' in rel_link or '../../' in rel_link:
                document['link'] = '/'.join(rel_link.split('/')[2:])
            elif rel_link:
                parent_url = '/'.join(url.split('/')[3:5]) + '/'
                document['link'] = urljoin(parent_url, rel_link)

            title = clean_text(cells[3].text)
            notif['title'] = title
            document['title'] = title
        else:
            try:
                rel_link = cells[0].a.get('href')
            except AttributeError:
                rel_link = cells[1].a.get('href')

            if 'common' in rel_link or '../..' in rel_link:
                document['link'] = '/'.join(rel_link.split('/')[2:])
                document['title'] = clean_text(cells[0].text)
            elif rel_link:
                parent_url = '/'.join(url.split('/')[3:5]) + '/'
                document['link'] = urljoin(parent_url, rel_link)
                document['title'] = clean_text(cells[0].text)
        notif['documents'].append(document)
    results.append(notif)
    return results

def to_json(filename, results):
    f = open(filename, "w")
    json.dump(results, f, ensure_ascii=False, indent=4)
    f.close()

if __name__ == '__main__':


    for base_url in BASE_URLS:
        results = []
        url_years = get_url_by_year(base_url)
        for url in url_years:
            results.extend(download(url))
        lang = base_url.split('/')[3]
        to_json(lang, results)
