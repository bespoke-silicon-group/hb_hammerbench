#!/usr/bin/env python3
# same as serial parser
import sys
import re

pattern = re.compile(r"0x([0-9a-fA-F]+)")
TAG_WIDTH = 4
TG_ID_WIDTH = 14
X_WIDTH = 6
Y_WIDTH = 6
TYPE_WIDTH = 2
TAG_SHIFT = 0
TG_ID_SHIFT = TAG_SHIFT + TAG_WIDTH
X_SHIFT = TG_ID_SHIFT + TG_ID_WIDTH
Y_SHIFT = X_SHIFT + X_WIDTH
TYPE_SHIFT = Y_SHIFT + Y_WIDTH
mask = lambda w: (1 << w) - 1
print("tag,tg_id,x,y,type,cycles")
for line in sys.stdin:
    m = pattern.search(line)
    if not m: continue
    word = int(m.group(1), 16)
    tag = (word >> TAG_SHIFT) & mask(TAG_WIDTH)
    tg_id = (word >> TG_ID_SHIFT) & mask(TG_ID_WIDTH)
    x = (word >> X_SHIFT) & mask(X_WIDTH)
    y = (word >> Y_SHIFT) & mask(Y_WIDTH)
    typ = (word >> TYPE_SHIFT) & mask(TYPE_WIDTH)
    cycles = word >> (TYPE_SHIFT + TYPE_WIDTH)
    print(f"{tag},{tg_id},{x},{y},{typ},{cycles}")
