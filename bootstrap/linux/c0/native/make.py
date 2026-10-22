#!/usr/bin/env python3
import re, sys
src=open(sys.argv[1]).read(); helpers=open(sys.argv[2]).read()
old=''':start
E8 %main 
89 C7
B8 3C 00 00 00
0F 05'''
new=''':start
48 8B 3C 24 
48 8D 74 24 08 
48 8D 54 FE 08 
E8 %__c1_start 
89 C7 
B8 3C 00 00 00 
0F 05'''
if old not in src: raise SystemExit('startup pattern missing')
src=src.replace(old,new,1)
n=0
for line in helpers.splitlines():
    if line.startswith(':') or not line.strip(): continue
    t=re.sub(r'%[A-Za-z0-9_]+','00000000',line)
    n+=len(re.findall(r'[0-9A-Fa-f]{2}',t))
i=src.rfind('^00600000')
if i<0: raise SystemExit('data marker missing')
before=src[:i]
# final text padding is the last .zero before the data marker
m=list(re.finditer(r'^\.zero (\d+)\s*$', before, re.M))[-1]
pad=int(m.group(1))
if pad < n: raise SystemExit('text padding too small')
repl=helpers+'\n.zero %d\n' % (pad-n)
src=before[:m.start()]+repl+before[m.end():]+src[i:]
open(sys.argv[3],'w').write(src)
