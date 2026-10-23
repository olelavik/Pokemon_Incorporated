import os, sys; sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from economy import BASE, INDEX
def sell_value(bst, lvl, badges, base=50):
    steps=bst-250; v=base
    if steps>=0: v=(v<<(steps//35))*(35+steps%35)//35
    else: v=(v>>((-steps)//35))*35//(35+(-steps)%35)
    return v*(10+lvl)//20*INDEX[badges]//100
def p_catch(rate, ball, hp_frac=0.15):
    a = (3 - 2*hp_frac)/3 * rate * ball          # FRLG formula (status none)
    return min(1, a/255)
def ev(rate, value, price, mult, flee):
    p = p_catch(rate, mult)
    end = p + (1-p)*flee                          # chance the encounter ends each throw
    throws = 1/end
    win = p/end
    return win, throws, win*value - throws*price
targets = [  # name, bst, level, catch rate, badges when met
    ('Ponyta  (Route 1, 2%)', 410, 5, 190, 0),
    ('Scyther (Forest, 2%)', 500, 6, 45, 0),
    ('Eevee   (Route 22, 2%)', 325, 5, 45, 0),
    ('Abra    (Route 24)', 310, 9, 200, 1),
    ('Chansey (Safari)', 450, 25, 30, 4),
    ('Lapras  (gift->wild)', 535, 25, 45, 5),
    ('Snorlax (static)', 540, 30, 25, 5),
    ('Dratini (Safari)', 300, 20, 45, 5),
]
for flee in (0.0, 0.25):
    print(f'\n=== flee chance after a failed throw: {int(flee*100)}% ===')
    print(f"{'target':24}{'value':>9}  {'Poké: win/throws/profit':>26}  {'Great':>22}  {'Ultra':>22}")
    for name,bst,lvl,rate,b in targets:
        v=sell_value(bst,lvl,b)
        row=f'{name:24}{v:>9,}'
        for ball,mult in (('POKE_BALL',1),('GREAT_BALL',1.5),('ULTRA_BALL',2)):
            price=BASE[ball]*INDEX[b]//100
            w,t,e=ev(rate,v,price,mult,flee)
            row+=f'   {w*100:4.0f}% {t:4.1f} {e:>9,.0f}'
        print(row)
