"""Find 4x4 spots for an Exchange building in a town layout.
A spot is good when all 16 tiles are walkable non-door ground with no events,
the tile below the door (x+2, y+4) is walkable, and rows above/below aren't map edge."""
import json, struct, sys, os
R=os.path.expanduser('~/pokefirered')
town, lid = sys.argv[1], sys.argv[2]
L={l['id']:l for l in json.load(open(f'{R}/data/layouts/layouts.json'))['layouts'] if 'id' in l}
l=L[lid]; b=open(f"{R}/{l['blockdata_filepath']}",'rb').read(); w,h=l['width'],l['height']
g=lambda x,y: struct.unpack_from('<H',b,(y*w+x)*2)[0]
m=json.load(open(f'{R}/data/maps/{town}/map.json'))
ev=set()
for k in ('object_events','warp_events','bg_events','coord_events'):
    for o in m.get(k) or []:
        ev.add((o['x'],o['y']))
        if k=='object_events':
            for dx in range(-o.get('movement_range_x',0),o.get('movement_range_x',0)+1):
                for dy in range(-o.get('movement_range_y',0),o.get('movement_range_y',0)+1): ev.add((o['x']+dx,o['y']+dy))
walk=lambda x,y: 0<=x<w and 0<=y<h and not (g(x,y)>>10)&3
res=[]
for y in range(2,h-6):
    for x in range(2,w-6):
        cells=[(x+c,y+r) for r in range(4) for c in range(4)]
        if all(walk(*p) and p not in ev for p in cells) and walk(x+2,y+4) and (x+2,y+4) not in ev:
            # keep at least a walkable ring around so we don't block paths: count walkable neighbours
            ring=[(x-1,yy) for yy in range(y,y+4)]+[(x+4,yy) for yy in range(y,y+4)]+[(xx,y+4) for xx in range(x,x+4)]
            score=sum(walk(*p) for p in ring)
            res.append((score,x,y))
res.sort(reverse=True)
print(town, len(res), res[:25])
