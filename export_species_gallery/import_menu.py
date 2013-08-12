#!/bin/usr/python

import re
from datetime import datetime
import locale
from urllib2 import urlopen
from urllib2 import HTTPError
from urlparse import urljoin, urlsplit

import argparse
import json

from bs4 import BeautifulSoup
from bs4.element import Tag

BASE_URL = "http://www.cites.org/gallery/species/"

menu_entries = {
    "mammals": "mammal/mammal_list.html",
    "birds": "bird/bird_list.html",
    "reptiles": "reptile/reptiles_list.html",
    "amphibians": "amphibian/amphibians_list.html",
    "fishes": "fish/fish_list.html",
    "invertebrates": "invertibrate/invertebrate_list.html",
    "cacti": "cactus/cacti_list.html",
    "orchid": "orchid/orchid_list.html",
    "other_plants": "other_plant/otherplants_list.html",
}

menu_translations = {

}

text_pattern = re.compile('\W+', re.UNICODE)
html_pattern = re.compile('<[^<]+?>')


def clean_title(title):

    if isinstance(title, Tag):
        title = title.contents[0]

    html_cleaned = html_pattern.sub('', title).strip()
    return text_pattern.sub(' ', html_cleaned)

def get_translations(species_url, rel_url):

    url_esp = "/".join(BASE_URL.split("/")[:3]) + '/esp/' + "/".join(BASE_URL.split("/")[3:])  + species_url

    # For other_plants category the fra link differs by character 's' in "other_plants"
    if 'other_plant' in species_url:
            parent_url = species_url.split('/')[0] + 's'
            species_url = parent_url + '/' + species_url.split('/')[1]
            url_fra = "/".join(BASE_URL.split("/")[:3]) + '/fra/' + "/".join(BASE_URL.split("/")[3:])  + species_url
    else:
        url_fra = "/".join(BASE_URL.split("/")[:3]) + '/fra/' + "/".join(BASE_URL.split("/")[3:])  + species_url

    url_list = [ url_fra, url_esp]

    translations = {}
    for url in url_list:
        try:
            html = urlopen(url).read()
        except HTTPError as err:
            print err

        soup = BeautifulSoup(html, "html.parser")
        row = soup.find("a", href="{}".format(rel_url))
        lang = str(url.split('/')[3])[:-1]
        try:
            translations[lang] = clean_title(row.contents[0])
        except AttributeError as err:
            # if there is no translation for current link
            translations[lang] = ''


    return translations

def build_species_dict(data, species_order, species_url):
    species = {}
    species_order = species_order.lower()
    species["menu_name"] = species_order.lower()
    species['title'] = species_order[0].upper() + species_order[1:]
    species['i18n_mode'] = "I18N_MODE_MULTIPLE"
    species['contextual_menu'] = { "enabled": True }
    species['links'] = []
    return species

def build_subspecies_dict(data, species_url):
    subspecies = {}
    subspecies['link_path'] = "gallery/species/" + species_url.split('/')[0] + "/" + data.a.get('href')
    subspecies['link_title'] = clean_title(data.a.contents[0])
    rel_url = data.a.get('href')

    subspecies['translations'] = get_translations(species_url, rel_url)
    return subspecies

def download_menu_entry(species_url):
    html = urlopen(BASE_URL + species_url).read()
    soup = BeautifulSoup(html, "html.parser")
    table = soup.find("table")

    table_data = [td for td in table.find_all("td")][3:]

    results = []
    species = {}
    flag = False
    for data in table_data:
        if data.has_attr('class'):
            # if we get to the next list head item we append the dictionary to the result
            if species:
                results.append(species)
                species = {}
            species_order = clean_title(data.contents[0])
            species = build_species_dict(data, species_order, species_url)
        elif data.a:
            subspecies = build_subspecies_dict(data, species_url)
            try:
                species['links'].append(subspecies)
            except KeyError as err:
                # If species has not been initialized, we do it here
                species_order = species_url.split('/')[0]
                species = build_species_dict(data, species_order, species_url)
                species['links'].append(subspecies)
                flag = True
    if flag:
        results.append(species)
    return results

def to_json(filename, results):
    f = open(filename, "w")
    f.write(json.dumps(results, indent=4))
    f.close()

if __name__ == '__main__':

    for k, v in menu_entries.items():
        to_json(k, download_menu_entry(v))
    print "Wrote {} files".format(len(menu_entries))
