import sys,glob
from PIL import Image
# sheet.py PREFIX OUT [cols]
pre,out=sys.argv[1],sys.argv[2]; cols=int(sys.argv[3]) if len(sys.argv)>3 else 3
fs=sorted(glob.glob(f'/root/smoke/{pre}*.png'),key=lambda p:int(''.join(c for c in p.split('/')[-1][len(pre):] if c.isdigit()) or 0))
ims=[Image.open(f).resize((240,160)) for f in fs]
rows=(len(ims)+cols-1)//cols
W=Image.new('RGB',(240*cols,160*rows),'white')
for i,im in enumerate(ims): W.paste(im,((i%cols)*240,(i//cols)*160))
W.save(out); print(len(ims),'frames')
