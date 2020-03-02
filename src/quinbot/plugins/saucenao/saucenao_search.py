# -*- coding: utf-8 -*-

from saucenao import SauceNao

sauce = SauceNao(directory='')
result = sauce.check_file('test.jpg')
print(result)