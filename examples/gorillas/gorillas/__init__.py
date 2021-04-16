import pygame, random
from pygame.font import FontType


SCR_WIDTH = 640
SCR_HEIGHT = 350

SUN_X = 300
SUN_Y = 10

BUILDING_COLORS = ((173, 170, 173), (0, 170, 173), (173, 0, 0))
LIGHT_WINDOW = (255, 255, 82)
DARK_WINDOW = (82, 85, 82)
SKY_COLOR = (0, 0, 173)
GOR_COLOR = (255, 170, 82)
BAN_COLOR = (255, 255, 82)
EXPLOSION_COLOR = (255, 0, 0)
SUN_COLOR = (255, 255, 0)
DARK_RED_COLOR = (173, 0, 0)
BLACK_COLOR = (0, 0, 0)
WHITE_COLOR = (255, 255, 255)
GRAY_COLOR = (173, 170, 173)

RIGHT = 0
UP = 1
LEFT = 2
DOWN = 3



GOR_DOWN_ASCII = """

          XXXXXXXX
          XXXXXXXX
         XX      XX
         XXXXXXXXXX
         XXX  X  XX
          XXXXXXXX
          XXXXXXXX
           XXXXXX
      XXXXXXXXXXXXXXXX
   XXXXXXXXXXXXXXXXXXXXXX
  XXXXXXXXXXXX XXXXXXXXXXX
 XXXXXXXXXXXXX XXXXXXXXXXXX
 XXXXXXXXXXXX X XXXXXXXXXXX
XXXXX XXXXXX XXX XXXXX XXXXX
XXXXX XXX   XXXXX   XX XXXXX
XXXXX   XXXXXXXXXXXX   XXXXX
 XXXXX  XXXXXXXXXXXX  XXXXX
 XXXXX  XXXXXXXXXXXX  XXXXX
  XXXXX XXXXXXXXXXXX XXXXX
   XXXXXXXXXXXXXXXXXXXXXX
       XXXXXXXXXXXXX
     XXXXXX     XXXXXX
     XXXXX       XXXXX
    XXXXX         XXXXX
    XXXXX         XXXXX
    XXXXX         XXXXX
    XXXXX         XXXXX
    XXXXX         XXXXX
     XXXXX       XXXXX
"""

GOR_LEFT_ASCII = """
   XXXXX
  XXXXX   XXXXXXXX
 XXXXX    XXXXXXXX
 XXXXX   XX      XX
XXXXX    XXXXXXXXXX
XXXXX    XXX  X  XX
XXXXX     XXXXXXXX
 XXXXX    XXXXXXXX
 XXXXX     XXXXXX
  XXXXXXXXXXXXXXXXXXXX
   XXXXXXXXXXXXXXXXXXXXXX
      XXXXXXXX XXXXXXXXXXX
      XXXXXXXX XXXXXXXXXXXX
      XXXXXXX X XXXXXXXXXXX
      XXXXXX XXX XXXXX XXXXX
      XXX   XXXXX   XX XXXXX
        XXXXXXXXXXXX   XXXXX
        XXXXXXXXXXXX  XXXXX
        XXXXXXXXXXXX  XXXXX
        XXXXXXXXXXXX XXXXX
       XXXXXXXXXXXXXXXXXX
       XXXXXXXXXXXXX
     XXXXXX     XXXXXX
     XXXXX       XXXXX
    XXXXX         XXXXX
    XXXXX         XXXXX
    XXXXX         XXXXX
    XXXXX         XXXXX
    XXXXX         XXXXX
     XXXXX       XXXXX
"""

GOR_RIGHT_ASCII = """
                    XXXXX
          XXXXXXXX   XXXXX
          XXXXXXXX    XXXXX
         XX      XX   XXXXX
         XXXXXXXXXX    XXXXX
         XXX  X  XX    XXXXX
          XXXXXXXX     XXXXX
          XXXXXXXX    XXXXX
           XXXXXX     XXXXX
      XXXXXXXXXXXXXXXXXXXX
   XXXXXXXXXXXXXXXXXXXXXX
  XXXXXXXXXXXX XXXXXXX
 XXXXXXXXXXXXX XXXXXXX
 XXXXXXXXXXXX X XXXXXX
XXXXX XXXXXX XXX XXXXX
XXXXX XXX   XXXXX   XX
XXXXX   XXXXXXXXXXXX
 XXXXX  XXXXXXXXXXXX
 XXXXX  XXXXXXXXXXXX
  XXXXX XXXXXXXXXXXX
   XXXXXXXXXXXXXXXXX
       XXXXXXXXXXXXX
     XXXXXX     XXXXXX
     XXXXX       XXXXX
    XXXXX         XXXXX
    XXXXX         XXXXX
    XXXXX         XXXXX
    XXXXX         XXXXX
    XXXXX         XXXXX
     XXXXX       XXXXX
"""


BAN_RIGHT_ASCII = """
     XX
    XXX
   XXX
   XXX
   XXX
   XXX
   XXX
    XXX
     XX
"""

BAN_LEFT_ASCII = """
XX
XXX
 XXX
 XXX
 XXX
 XXX
 XXX
XXX
XX
"""

BAN_UP_ASCII = """
XX     XX
XXXXXXXXX
 XXXXXXX
  XXXXX
"""

BAN_DOWN_ASCII = """
  XXXXX
 XXXXXXX
XXXXXXXXX
XX     XX
"""

SUN_NORMAL_ASCII = """
                    X
                    X
            X       X       X
             X      X      X
             X      X      X
     X        X     X     X        X
      X        X XXXXXXX X        X
       XX      XXXXXXXXXXX      XX
         X  XXXXXXXXXXXXXXXXX  X
          XXXXXXXXXXXXXXXXXXXXX
  X       XXXXXXXXXXXXXXXXXXXXX       X
   XXXX  XXXXXXXXXXXXXXXXXXXXXXX  XXXX
       XXXXXXXXXX XXXXX XXXXXXXXXX
        XXXXXXXX   XXX   XXXXXXXX
        XXXXXXXXX XXXXX XXXXXXXXX
XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
        XXXXXXXXXXXXXXXXXXXXXXXXX
        XXXXXXXXXXXXXXXXXXXXXXXXX
       XXXXXX XXXXXXXXXXXXX XXXXXX
   XXXX  XXXXX  XXXXXXXXX  XXXXX  XXXX
  X       XXXXXX  XXXXX  XXXXXX       X
          XXXXXXXX     XXXXXXXX
         X  XXXXXXXXXXXXXXXXX  X
       XX      XXXXXXXXXXX      XX
      X        X XXXXXXX X        X
     X        X     X     X        X
             X      X      X
             X      X      X
            X       X       X
                    X
                    X
"""


SUN_SHOCKED_ASCII = """
                    X
                    X
            X       X       X
             X      X      X
             X      X      X
     X        X     X     X        X
      X        X XXXXXXX X        X
       XX      XXXXXXXXXXX      XX
         X  XXXXXXXXXXXXXXXXX  X
          XXXXXXXXXXXXXXXXXXXXX
  X       XXXXXXXXXXXXXXXXXXXXX       X
   XXXX  XXXXXXXXXXXXXXXXXXXXXXX  XXXX
       XXXXXXXXXX XXXXX XXXXXXXXXX
        XXXXXXXX   XXX   XXXXXXXX
        XXXXXXXXX XXXXX XXXXXXXXX
XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
        XXXXXXXXXXXXXXXXXXXXXXXXX
        XXXXXXXXXXXXXXXXXXXXXXXXX
       XXXXXXXXXXXXXXXXXXXXXXXXXXX
   XXXX  XXXXXXXXX     XXXXXXXXX  XXXX
  X       XXXXXXX       XXXXXXX       X
          XXXXXXX       XXXXXXX
         X  XXXXXX     XXXXXX  X
       XX      XXXXXXXXXXX      XX
      X        X XXXXXXX X        X
     X        X     X     X        X
             X      X      X
             X      X      X
            X       X       X
                    X
                    X
"""

def makeSurfaceFromASCII(ascii, fgColor=(255,255,255), bgColor=(0,0,0)):
    """Returns a new pygame.Surface object that has the image drawn on it as specified by the ascii parameter.
    The first and last line in ascii are ignored. Otherwise, any X in ascii marks a pixel with the foreground color
    and any other letter marks a pixel of the background color. The Surface object has a width of the widest line
    in the ascii string, and is always rectangular."""

    """Try experimenting with this function so that you can specify more than two colors. (Pass a dict of
    ascii letters and RGB tuples, say."""
    ascii = ascii.split('\n')[1:-1]
    width = max([len(x) for x in ascii])
    height = len(ascii)
    surf = pygame.Surface((width, height))
    surf.fill(bgColor)

    pArr = pygame.PixelArray(surf)
    for y in range(height):
        for x in range(len(ascii[y])):
            if ascii[y][x] == 'X':
                pArr[x][y] = fgColor
    return surf

GOR_ARM_SURF    = (makeSurfaceFromASCII(GOR_LEFT_ASCII,    GOR_COLOR,      SKY_COLOR), makeSurfaceFromASCII(GOR_RIGHT_ASCII,   GOR_COLOR,      SKY_COLOR))
GOR_DOWN_SURF    = makeSurfaceFromASCII(GOR_DOWN_ASCII,    GOR_COLOR,      SKY_COLOR)
BAN_RIGHT_SURF   = makeSurfaceFromASCII(BAN_RIGHT_ASCII,   BAN_COLOR,      SKY_COLOR)
BAN_LEFT_SURF    = makeSurfaceFromASCII(BAN_LEFT_ASCII,    BAN_COLOR,      SKY_COLOR)
BAN_UP_SURF      = makeSurfaceFromASCII(BAN_UP_ASCII,      BAN_COLOR,      SKY_COLOR)
BAN_DOWN_SURF    = makeSurfaceFromASCII(BAN_DOWN_ASCII,    BAN_COLOR,      SKY_COLOR)
SUN_NORMAL_SURF  = makeSurfaceFromASCII(SUN_NORMAL_ASCII,  SUN_COLOR,      SKY_COLOR)
SUN_SHOCKED_SURF = makeSurfaceFromASCII(SUN_SHOCKED_ASCII, SUN_COLOR,      SKY_COLOR)

#assert GOR_DOWN_SURF.get_size() == GOR_LEFT_SURF.get_size() == GOR_RIGHT_SURF.get_size()
"""Create the pygame.Surface objects from the ASCII strings."""

sunRect = pygame.Rect(SUN_X, SUN_Y, SUN_NORMAL_SURF.get_width(), SUN_NORMAL_SURF.get_height())
"""sunRect will be a global value so we'll always know where the sun is."""

def drawSun(screenSurf, x = 0, y = 0):
    """Draws the sun sprite onto the screenSurf surface. If shocked is True, then use the shocked-looking face,
    otherwise use the normal smiley face. This function does not call python.display.update()"""    
    if sunRect.collidepoint(x, y):
        screenSurf.blit(SUN_SHOCKED_SURF, (SUN_X, SUN_Y))
    else:
        screenSurf.blit(SUN_NORMAL_SURF, (SUN_X, SUN_Y))

def drawGorilla(screenSurf, pos, gorSurf = GOR_DOWN_SURF):
    """Draws the gorillas sprite onto the screenSurf surface at a specific x, y coordinate. The x,y coordinate
    is for the top left corner of the gorillas sprite. Note that all three gorillas surfaces are the same size."""
    xAj = gorSurf.get_rect().width
    yAdj = gorSurf.get_rect().height
    screenSurf.blit(gorSurf, (pos.x - int(xAj/2), 335 - pos.y - yAdj - 1))

def makeCityScape(buildingCoords):

    screenSurf = pygame.Surface((SCR_WIDTH, SCR_HEIGHT))  # first make the new surface the same size of the screen.
    screenSurf.fill(SKY_COLOR)  # fill in the surface with the background sky color

    bottomLine = 335
    BuildWidth = int(SCR_WIDTH/10)
    windowWidth = 4  # the width of each window in pixels
    windowHeight = 7  # the height of each window in pixels
    windowSpacingX = 10  # how many pixels apart each window's left edge is
    windowSpacingY = 15  # how many pixels apart each window's top edge is
    BuildHeight = []

    i = 0
    for iteration in range(0, len(buildingCoords), int(SCR_WIDTH/10)) :
        BuildHeight.append(buildingCoords[iteration])
        buildcolor = BUILDING_COLORS[random.randint(0,len(BUILDING_COLORS)-1)]
        pygame.draw.rect(screenSurf,buildcolor,(i*int(SCR_WIDTH/10), bottomLine-buildingCoords[iteration], int(SCR_WIDTH/10), buildingCoords[iteration]), 0)
        i += 1

    x = 0
    BuildHeight = [int(i) for i in BuildHeight]
    for buildHeight in BuildHeight :
        for winx in range(windowSpacingX, BuildWidth - windowSpacingX + windowWidth, windowSpacingX):
            for winy in range(windowSpacingY, buildHeight - windowSpacingY, windowSpacingY):
                if random.randint(1, 4) == 1:
                    winColor = DARK_WINDOW
                else:
                    winColor = LIGHT_WINDOW
                pygame.draw.rect(screenSurf, winColor, (x + winx, (bottomLine - buildHeight) + 1 + winy, windowWidth, windowHeight))
        x += BuildWidth

    return screenSurf

def displayBanana(screenSurf, orient, x, y):
    """Draws the banana shape to the screenSurf surface with its top left corner at the x y coordinate provided.
    "orient" is one of the RIGHT, UP, LEFT, or DOWN values (which are the integers 0 to 3 respectively)"""
    WaitTime = 40
    if orient == DOWN:
        xAj = BAN_DOWN_SURF.get_rect().width
        yAdj = BAN_DOWN_SURF.get_rect().height
        x_ = x-xAj//2+1
        y_ = y-yAdj//2+1
        screenSurf.blit(BAN_DOWN_SURF, (x_, y_))
        pygame.display.update()
        pygame.time.wait(WaitTime)
        
    elif orient == UP:
        xAj = BAN_UP_SURF.get_rect().width
        yAdj = BAN_UP_SURF.get_rect().height
        x_ = x - xAj // 2 + 1
        y_ = y - yAdj // 2 + 1
        screenSurf.blit(BAN_UP_SURF, (x_, y_))
        pygame.display.update()
        pygame.time.wait(WaitTime)
       
    elif orient == LEFT:
        xAj = BAN_LEFT_SURF.get_rect().width
        yAdj = BAN_LEFT_SURF.get_rect().height
        x_ = x - xAj // 2 + 1
        y_ = y - yAdj // 2 + 1
        screenSurf.blit(BAN_LEFT_SURF, (x_, y_))
        pygame.display.update()
        pygame.time.wait(WaitTime)
        
    elif orient == RIGHT:
        xAj = BAN_RIGHT_SURF.get_rect().width
        yAdj = BAN_RIGHT_SURF.get_rect().height
        x_ = x - xAj // 2 + 1
        y_ = y - yAdj // 2 + 1
        screenSurf.blit(BAN_RIGHT_SURF, (x_, y_))
        pygame.display.update()
        pygame.time.wait(WaitTime)
      

def drawText(text, surfObj, x, y , fgcol, bgcol,fontSize=20, pos='left'):
    """A generic function to draw a string to a pygame.Surface object at a certain x,y location. This returns
    a pygame.Rect object which describes the area the string was drawn on.

    If the pos parameter is "left", then the x,y parameter specifies the top left corner of the text rectangle.
    If the pos parameter is "center", then the x,y parameter specifies the middle top point of the text rectangle."""
    GAME_FONT = pygame.font.SysFont(None, fontSize)

    textobj = GAME_FONT.render(text, 1, fgcol, bgcol) # creates the text in memory (it's not on a surface yet).
    textrect = textobj.get_rect()

    if pos == 'left':
        textrect.topleft = (x, y)
    elif pos == 'center':
        textrect.midtop = (x, y)
    surfObj.blit(textobj, textrect) # draws the text onto the surface
    """Remember that the text will only appear on the screen if you pass the pygame.Surface object that was
    returned from the call to pygame.display.set_mode(), and only after pygame.display.update() is called."""
    return textrect
