#!/usr/bin/env python
import sys
sys.path.insert(1, sys.argv[1])
from duels import Subscriber
import pygame
from pygame.font import FontType
import os

game_path=os.path.abspath(os.path.dirname(__file__))

def get_image(filename):
    return pygame.image.load('{}/sprites/{}'.format(game_path, filename))

## images ================================================

p1_images = [get_image('p1_{}.png'.format(i)) for i in range(4)] + [get_image('p1_wins.png')]
p2_images = [get_image('p2_{}.png'.format(i)) for i in range(4)] + [get_image('p2_wins.png')]
background = get_image("background.png")
sonar = get_image("sonar.png")
bloc = get_image("bloc.png")
treasure = get_image("treasure.png")

## init =============================================================

game = Subscriber()
init_msg = game.get_init()

box_width = 48 #pix
box_height = 48 #pix
mid_height = box_height//2

screen_width = init_msg.width*box_width
screen_height = (init_msg.height+1)*box_height #pix

x_size = screen_width//box_width-1

# init pygame
pygame.init()
screen = pygame.display.set_mode((screen_width, screen_height))

# window name and icon
pygame.display.set_caption("Treasure hunt")
pygame.display.set_icon(p1_images[0])

game.ready()

## utilitary functions =============================================

def addText(text, x, y , col, fontSize=40, pos='midleft'):
    this_font = pygame.font.SysFont(None, fontSize)

    textobj = this_font.render(text, 1, col) # creates the text in memory (it's not on a surface yet).
    textrect = textobj.get_rect()
    setattr(textrect, pos, (x,y))        
    return textobj, textrect
    
def convert_coordinates(coords):
    """converts grid coordinates to images coordinates"""

    x = coords.x*box_width - 1
    y = (coords.y+1)*box_height - 1
    
    return (x, y)

def header_x(x, yp=0):
    return (x * box_width, yp)

def blit_players(pose1, pose2):
    """generates blit sequence for players"""
    im1 = p1_images[pose1.t]
    im2 = p2_images[pose2.t]
    ret = [(im1, header_x(1)), (im2, header_x(x_size-1)), (im1, convert_coordinates(pose1)), (im2, convert_coordinates(pose2))]
    return ret

## main loop ========================================================

background_blits = [(background, (0,0)), (treasure, convert_coordinates(init_msg.treasure)), (bloc, (0, 0)), (bloc, header_x(x_size))]
for obs in init_msg.obs:
    background_blits.append((bloc, convert_coordinates(obs)))
    
# names
x_offset = 2*box_width + int(box_width/4)
background_blits.append(addText(init_msg.name1, x_offset, mid_height, (255,255,0), pos='midleft'))
background_blits.append(addText(init_msg.name2, screen_width - x_offset, mid_height, (255,255,0), pos='midright'))

# obstacles where there is no name
xm = (background_blits[-2][1].right+box_width)//box_width
xM = (background_blits[-1][1].left-box_width)//box_width
for x in range(xm, xM+1):
    background_blits.append((bloc, header_x(x)))

    #screen.blit(textobj, textrect)
# display non changing elements
screen.blits(background_blits) 
pygame.display.update()

while True:
    msg = game.refresh()
    if game.winner:
        break

    # display non changing elements
    screen.blits(background_blits)

    # display elements on the screen
    screen.blits([(sonar, convert_coordinates(scan)) for scan in msg.scans])
    screen.blits(blit_players(msg.pose1, msg.pose2))

    pygame.display.update()

# display winner

screen.blits(background_blits)
screen.blits(blit_players(msg.pose1, msg.pose2))

game_over = [addText('GAME OVER', screen_width//2, int(screen_height*0.4), (255,255,0), pos='center', fontSize=100)]
if game.winner == 3:
    text = 'Draw'
elif game.winner < 0:
    text = 'A bug stopped the game'
else:
    text = game.winner_name(init_msg) + ' won the game'
    
game_over.append(addText(text, screen_width//2, int(screen_height*0.6), (255,255,0), pos='center', fontSize=120))
    
screen.blits(game_over)
pygame.display.update()

t0 = pygame.time.get_ticks()
while pygame.time.get_ticks() - t0 < 5000:
    if any(event.type == pygame.QUIT for event in pygame.event.get()):
        break
pygame.quit()
