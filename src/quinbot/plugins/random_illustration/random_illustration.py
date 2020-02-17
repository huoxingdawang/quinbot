# -*- coding: utf-8 -*-

from bs4 import BeautifulSoup
import re
import requests
import lxml
from PIL import Image
import random
import os

http_proxy = '127.0.0.1:1080'
https_proxy = '127.0.0.1:1080'

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

proxies = {
    'http' : http_proxy,
    'https' : https_proxy
}

headers = {
    'User-Agent' : random.choice(user_agent_list)
}

def crop_image(path):
    img = Image.open(path)
    cropped = img.crop((5, 5, img.size[0] - 5, img.size[1] - 5))
    cropped.save(path)

def download_image(url, is_r18=False):
    r = requests.get('https://search.pstatic.net/common?type=origin&src=' + url, headers=headers, proxies=proxies, timeout=5)
    file_name = url[url.rfind('/') : ]
    r18 = True
    save_path = 'D:/CoolQ/CoolQ Pro/data/image/temp/pixiv/' + file_name
    if (r.status_code == 200):
        open(save_path, 'wb').write(r.content)
        if is_r18:
            crop_image(save_path)

def get_illustration_info(r18, num, proxy, keyword):
    try:
        api_url = 'https://api.lolicon.app/setu/'
        params = {
            'r18' : r18,
            'num' : num,
            'proxy' : proxy,
            'keyword' : keyword
        }

        r = requests.get(api_url, params=params, headers=headers, proxies=proxies, timeout=4)
        r.encoding = 'utf-8'
        data = r.json()
        result = []
        for info in data['data']:
            local_file_name = 'temp/pixiv/' + info['url'][info['url'].rfind('/') + 1 : ]
            if not os.path.exists('D:/CoolQ/CoolQ Pro/data/image/' + local_file_name):
                download_image(info['url'], info['r18'])
            result.append({
                'uid' : info['uid'],
                'artist' : info['author'],
                'pid' : info['pid'],
                'title' : info['title'],
                'url' : info['url'],
                'local_file_name' : local_file_name,
                'error_message' : ''
            })
        return result
    except Exception as e:
        return [
            {
                'uid' : -1,
                'artist' : '-1',
                'pid' : -1,
                'title' : '-1',
                'url' : '-1',
                'local_file_name' : '-1',
                'error_message' : str(e)
            }
        ]

#print(download_image('https://i.pixiv.cat/img-original/img/2019/07/13/18/13/56/75699656_p0.jpg'))
#print(get_illustration_info(1, 1, 'i.pixiv.cat', '-'))