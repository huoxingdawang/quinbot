# -*- coding: utf-8 -*-

from pixivpy3 import *
import random
import requests
import os
from PIL import Image
from shutil import copyfile

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
    'http' : '127.0.0.1:1080',
    'https' : '127.0.0.1:1080'
}

headers = {
    'User-Agent' : random.choice(user_agent_list)
}

pixiv_api = PixivAPI()
pixiv_api.set_proxies(proxies)

pixiv_api.login("1748065414@qq.com", "1748065414Bread")

def crop_image(path):
    img = Image.open(path)
    cropped = img.crop((5, 5, img.size[0] - 5, img.size[1] - 5))
    cropped.save(path)

def pixiv_format_ranking_save_path(save_type, mode, date, file_name):
    path = 'D:/CoolQ/CoolQ Pro/data/image/pixiv/%s/%s/%s/' % (save_type, mode, date)
    if not os.path.exists(path):
        os.mkdir(path)
    path = path + file_name
    return path

def pixiv_format_cache_save_path(save_type, uid, file_name):
    path = 'D:/CoolQ/CoolQ Pro/data/image/pixiv/%s/%d/' % (save_type, uid)
    if not os.path.exists(path):
        os.mkdir(path)
    path = path + file_name
    return path

def pixiv_download_image(url, save_path, needcopy=None):
    if os.path.exists(save_path):
        return
    if needcopy != None and os.path.exists(needcopy):
        return
    url = url.replace('i.pximg.net', 'i.pixiv.cat', 1)
    r = requests.get('https://search.pstatic.net/common?type=origin&src=' + url, headers=headers, proxies=proxies)
    if (r.status_code == 200):
        open(save_path, 'wb').write(r.content)
        crop_image(save_path)
        if needcopy != None:
            copyfile(save_path, needcopy)
    else:
        raise Exception('Fail to download image')

def pixiv_ranking_search(count, mode, date=None):
    try:
        json_result = pixiv_api.ranking(ranking_type='illust', mode=mode, per_page=count, date=date)
        result = []
        if json_result['status'] == 'success':
            for info in json_result['response'][0]['works']:
                work = info['work'] 
                rank = info['rank']
                url = work['image_urls']['large']
                uid = work['user']['id']
                file_name = url[url.rfind('/') + 1 : ]
                save_path = pixiv_format_ranking_save_path('ranking', mode, date, file_name)
                pixiv_download_image(url, save_path, pixiv_format_cache_save_path('cache', uid, file_name))
                
                result.append({
                    'rank' : rank,
                    'uid' : uid,
                    'author' : work['user']['name'],
                    'pid' : work['id'],
                    'title' : work['title'],
                    'local_file_name' : 'pixiv/cache/%d/%s' % (uid, file_name),
                    'url' : url,
                    'tags' : ','.join(work['tags']),
                    'error_message' : ''
                })
            return result
        else:
            raise Exception('Fai to get illustration by ranking')
    except Exception as e:
        return [
            {
                'rank' : -1,
                'uid' : -1,
                'author' : '-1',
                'pid' : -1,
                'title' : '-1',
                'url' : '-1',
                'local_file_name' : '-1',
                'tags' : '-1',
                'error_message' : str(e)
            }
        ]

def pixiv_search_by_uid(uid, count):
    try:
        json_result = pixiv_api.users_works(uid, 1, count, ['large'])
        if json_result['status'] == 'success':
            result = []
            for info in json_result['response']:
                url = info['image_urls']['large']
                file_name = url[url.rfind('/') + 1 : ]
                save_path = pixiv_format_cache_save_path('cache', uid, file_name)
                pixiv_download_image(url, save_path)
                result.append({
                    'rank' : -1,
                    'uid' : uid,
                    'author' : info['user']['name'],
                    'pid' : info['id'],
                    'title' : info['title'],
                    'url' : url,
                    'local_file_name' : 'pixiv/cache/%d/%s' % (uid, file_name),
                    'tags' : ','.join(info['tags']),
                    'error_message' : ''
                })
            return result
        else:
            raise Exception('Fail to get illustration by uid')
    except Exception as e:
        return [
            {
                'rank' : -1,
                'uid' : -1,
                'author' : '-1',
                'pid' : -1,
                'title' : '-1',
                'url' : '-1',
                'local_file_name' : '-1',
                'tags' : '-1',
                'error_message' : str(e)
            }
        ]
        

def pixiv_search_by_keyword(query, mode, period='all', count=10):
    try:
        json_result = pixiv_api.search_works(query, mode=mode, period=period, page=1, per_page=count, types=['illustration'])
        if json_result['status'] == 'success':
            result = []
            for info in json_result['response']:
                url = info['image_urls']['large']
                uid = info['user']['id']
                file_name = url[url.rfind('/') + 1 : ]
                save_path = pixiv_format_cache_save_path('cache', uid, file_name)
                pixiv_download_image(url, save_path)
                result.append({
                    'rank' : -1,
                    'uid' : uid,
                    'author' : info['user']['name'],
                    'pid' : info['id'],
                    'title' : info['title'],
                    'url' : url,
                    'local_file_name' : 'pixiv/cache/%d/%s' % (uid, file_name),
                    'tags' : ','.join(info['tags']),
                    'error_message' : ''
                })
            return result
        else:
            raise Exception('Fail to get illustration by uid')
    except Exception as e:
        return [
            {
                'rank' : -1,
                'uid' : -1,
                'author' : '-1',
                'pid' : -1,
                'title' : '-1',
                'url' : '-1',
                'local_file_name' : '-1',
                'tags' : '-1',
                'error_message' : str(e)
            }
        ]

#print(pixiv_search_by_keyword('东方 3000users入り', 'tag', 'all', count=5))
#json_result = pixiv_api.ranking('illust', 'weekly', 1, per_page=2)
#print(json_result)

#print(pixiv_ranking_search(10, 'weekly', '2020-02-14'))
#print(pixiv_search_by_uid(490219, 2))