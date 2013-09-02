#!/bin/usr/python

import re
from datetime import datetime
from urllib2 import urlopen
from urlparse import urljoin, urlsplit

import json
import codecs

from pprint import pprint
from bs4 import BeautifulSoup, UnicodeDammit

URLS = [
    # 'http://www.cites.org/eng/resources/ref/suspend.php',
    'http://www.cites.org/fra/resources/ref/suspend.php',
    # 'http://www.cites.org/esp/resources/ref/suspend.php',
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

def convert_date(local_date):
    local_date = clean_text(local_date)
    try:
        date = datetime.strptime(local_date, '%d %B %Y')
    except ValueError:
        return local_date
    return date.strftime('%d/%m/%Y')

def update_records(records, notif):

    for rec in records:
        if rec['number'] in notif['no']:
            del notif['no']
            rec.update(notif)
            return

def download(url, lang):
    html = urlopen(url).read()
    soup = BeautifulSoup(html, "html.parser")
    table = soup.find("table")

    records = read_json(lang)
    table_rows = [tr for tr in table.find_all("tr")][1:]

    rec = []
    notif_row = {}
    for row in table_rows:
        if row.find('\n') == 0:
            import pdb; pdb.set_trace()
        cells = row.findChildren("td")
        if len(cells) < 3:
            scope = {}
            key = clean_text(cells[0].get_text())
            try:
                scope[key] = convert_date(cells[1].get_text())
            except IndexError as err:
                scope[key] = notif_row['valid_from']
            except ValueError:
                date = ' '.join(clean_text(cells[1].get_text()).split(' ')[:3])
                scope[key] = convert_date(date)

            try:
                notif_row['scope'].append(scope)
            except AttributeError:
                key = notif_row['scope']
                value = notif_row['valid_from']
                notif_row['scope'] = []
                notif_row['scope'].append({key:value})
                notif_row['scope'].append(scope)
        elif len(cells) == 4:
            if notif_row:
                update_records(records, notif_row)
            notif_row = {}
            notif_row['country'] = country
            notif_row['no'] = clean_text(cells[0].get_text())
            notif_row['basis'] = clean_text(cells[1].get_text())
            notif_row['scope'] = clean_text(cells[2].get_text())
            notif_row['valid_from'] = convert_date(cells[3].get_text())
            rec.append(notif_row)
        else:
            if notif_row:
                update_records(records, notif_row)
            notif_row = {}
            country = clean_text(cells[0].get_text())
            notif_row['country'] = country
            notif_row['no'] = clean_text(cells[1].get_text())
            notif_row['basis'] = clean_text(cells[2].get_text())
            notif_row['scope'] = clean_text(cells[3].get_text())
            notif_row['valid_from'] = convert_date(cells[4].get_text())
            rec.append(notif_row)

    update_records(records, notif_row)
    return records

def write_json(filename, results):
    f = open(filename, "w")
    f.write(codecs.BOM_UTF8)
    json.dump(results, f, indent=4)
    f.close()

def read_json(lang):
    json_data = open(lang)
    data = json.load(json_data, encoding="latin-1")
    return data

if __name__ == '__main__':
    for url in URLS:
        lang = url.split('/')[3]
        new_records = download(url, lang)
        filename = lang + '.json'
        write_json(filename, new_records)
