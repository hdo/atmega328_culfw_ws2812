import os
import sys
import re

data = open('csscolors.html').read()

#names = re.findall('<td><a target="_blank" href="/tags/ref_color_tryit.asp?color=(.*?)">', data)
names = re.findall('<td><a target="_blank" href="/tags/ref_color_tryit.asp.color=(.*?)">', data)
hexvalues = re.findall('<td><a target="_blank" href="/tags/ref_color_tryit.asp.hex=(.*?)">', data)
print len(names)
print len(hexvalues)

for index in range(len(names)):
    current_hex = hexvalues[index]
    cmd = "    { 0x%s, 0x%s, 0x%s }, // %.3d -> %s" % (current_hex[2:4], current_hex[0:2], current_hex[4:], index, names[index])
    print cmd
