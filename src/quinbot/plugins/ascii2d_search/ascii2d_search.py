# -*- coding: utf-8 -*-

from bs4 import BeautifulSoup
import re
import requests
import lxml
import random

proxies = {
    'http' : '127.0.0.1:1080',
    'https' : '127.0.0.1:1080'
}

user_agent_list = [
    'Mozilla/5.0 (Windows NT 6.1; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) ',
    'Chrome/45.0.2454.85 Safari/537.36 115Browser/6.0.3',
    'Mozilla/5.0 (Macintosh; U; Intel Mac OS X 10_6_8; en-us) AppleWebKit/534.50 (KHTML, like Gecko) Version/5.1 Safari/534.50',
    'Mozilla/5.0 (Windows; U; Windows NT 6.1; en-us) AppleWebKit/534.50 (KHTML, like Gecko) Version/5.1 Safari/534.50',
    'Mozilla/4.0 (compatible; MSIE 8.0; Windows NT 6.0; Trident/4.0)',
    'Mozilla/4.0 (compatible; MSIE 7.0; Windows NT 6.0)',
    'Mozilla/5.0 (Windows NT 6.1; rv:2.0.1) Gecko/20100101 Firefox/4.0.1',
    'Opera/9.80 (Windows NT 6.1; U; en) Presto/2.8.131 Version/11.11',
    'Mozilla/5.0 (Macintosh; Intel Mac OS X 10_7_0) AppleWebKit/535.11 (KHTML, like Gecko) Chrome/17.0.963.56 Safari/535.11',
    'Mozilla/4.0 (compatible; MSIE 7.0; Windows NT 5.1; Trident/4.0; SE 2.X MetaSr 1.0; SE 2.X MetaSr 1.0; .NET CLR 2.0.50727; SE 2.X MetaSr 1.0)',
    'Mozilla/5.0 (compatible; MSIE 9.0; Windows NT 6.1; Trident/5.0',
    'Mozilla/5.0 (Windows NT 6.1; rv:2.0.1) Gecko/20100101 Firefox/4.0.1',
]

headers = {
    'User-Agent' : random.choice(user_agent_list)
}

def download_image(url):
    r = requests.get(url, stream = True, headers = headers)
    file_name = url[url.rfind('/') + 1 : ]
    if (r.status_code == 200):
        open('D:/CoolQ/CoolQ Pro/data/image/temp/' + file_name, 'wb').write(r.content)
    else:
        raise Exception()

def ascii2d_search(search_type, url):
    try:
        full_url = 'https://ascii2d.net/search/url/' + url
        r = requests.get(full_url, headers = headers)
        r.encoding = 'utf-8'
        full_url = r.url

        if search_type == 1:
            full_url.replace('color', 'bovw', 1)
            r = requests.get(full_url, headers = headers)
            r.encoding = 'utf-8'
            
        soup = BeautifulSoup(r.text, 'lxml')
        for item_box in soup.select('.item-box'):
            detail_links = item_box.select('.detail-box a')
            if len(detail_links) == 0:
                continue
            preview_link = "https://ascii2d.net/" + item_box.select('.image-box img')[0]['src']
            art_link = detail_links[0]['href']
            art_name = detail_links[0].string
            artist_link = detail_links[1]['href']
            artist_name = detail_links[1].string
            download_image(preview_link)
            return {
                'failed' : 0,
                'art_name' : art_name,
                'art_link' : art_link,
                'artist_name' : artist_name,
                'artist_link' : artist_link,
                'preview_link' : preview_link
            }

    except Exception:
        return {
            'failed' : 1,
            'art_name' : 'error',
            'art_link' : 'error',
            'artist_name' : 'error',
            'artist_link' : 'error',
            'preview_link' : 'error'
        }

#print(ascii2d_search(0, 'https://gchat.qpic.cn/gchatpic_new/759831458/712548068-2310429950-0A8D93EE82B2AB59C326198DBA073F64/0?term=2'))