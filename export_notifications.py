

import re
from datetime import datetime
import locale

from urllib2 import urlopen
from urllib2 import HTTPError
from urlparse import urljoin, urlsplit

from bs4 import BeautifulSoup
from bs4.element import Tag

BASE_URL = "http://www.cites.org/eng/notif/"


def get_correct_table(soup):
    table = soup.find("table", { "class": "knoir" })
    return table

def download(url):
    html = urlopen(url).read()
    soup = BeautifulSoup(html, "html.parser")
    table = get_correct_table(soup)
    for row in table.children:
        notif = {}
        # we skip new lines
        if row.find('\n') != 0:
            cells = row.findChildren("td")
            if not cells:
                continue
            #verify if current row is not only a section from Title
            if len(cells) >= 3:
                print "\n################################################################\n"
                print cells
            else:
                print "\n****************************************************************\n"
                print cells





if __name__ == '__main__':

    download("http://www.cites.org/eng/notif/2013.php")
