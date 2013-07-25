#!/usr/bin/python

from urllib2 import urlopen
import argparse
import os
from bs4 import BeautifulSoup
from urlparse import urljoin, urlsplit
from build_path import *

BASE_URL = "http://www.cites.org/eng/res/"

def download():
    html = urlopen(BASE_URL + 'index.php').read()
    soup = BeautifulSoup(html, "lxml")
    urls = [a.get('href') for a in soup.find_all('a')]

    absolute_urls = [urljoin(BASE_URL, url) for url in urls]

    # Get urls in "res" directory only -- not keeping duplicates
    resources = set([url for url in absolute_urls if is_resource(url)])

    for res in resources:
        print "URL:", res
        #html = urlopen(res).read()
        get_resource(res)


def is_resource(url):
    url_pieces = urlsplit(url)
    return url_pieces.path.find('/res/') != -1

def get_resource(url):
    # Get the page name
    url_pieces = os.path.split(url)
    file_name = url_pieces[-1]

    # Get the absolute url path without page
    path = os.path.join(*url_pieces[:-1])
    path_pieces = path.split('/')

    # Get the url path which starts from /res/
    index = path_pieces.index('res')
    parent_tree = os.path.join(*path_pieces[index:])        

if __name__ == "__main__":

    #parser = argparse.ArgumentParser()
    #parser.add_argument('a', '--address', default='')
    download()
