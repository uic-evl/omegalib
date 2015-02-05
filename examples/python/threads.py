import time
import threading
import urllib2

def get_url(url):
    for i in range(1,20):
        s = urllib2.urlopen(url).read()
        print "Read {0}: size {1}".format(url, len(s))

theurls = ['http://google.com', 'http://yahoo.com', 'http://github.com']

for u in theurls:
    print(u)
    t = threading.Thread(target=get_url, args=(u,))
    t.start()
    
fpsStat = Stat.find('fps')
# Add an initial fps sample or onDraw will fail when querying for fps on the
# first frame
fpsStat.addSample(0)

def onDraw(displaySize, tileSize, camera, painter):
    font = painter.getDefaultFont()
    painter.drawText("fps: " + str(fpsStat.getCur()), font, Vector2(5, 5), TextAlign.HALeft | TextAlign.VATop, Color(1, 1, 0, 1))

setDrawFunction(onDraw)