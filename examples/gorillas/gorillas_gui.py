#!/usr/bin/env python
import sys
sys.path.insert(1, sys.argv[1])
from duels import Subscriber

game = Subscriber(1000)
init_msg = game.get_init()

def flip(y):
    return 335 - y

import pygame
import gorillas as bgd
from time import time

def raiseArm(turn, raised = True, side = -1):
    if side == -1:
        side = turn-1
    pos = init_msg.gor1 if turn == 1 else init_msg.gor2
    if raised:
        bgd.drawGorilla(skylineSurf, pos, bgd.GOR_ARM_SURF[side])
    else:
        bgd.drawGorilla(skylineSurf, pos)
        
# prepare display
radius = init_msg.radius

pygame.init()
winSurface = pygame.display.set_mode((bgd.SCR_WIDTH, bgd.SCR_HEIGHT), 0, 32)
pygame.display.set_caption('Gorillas')
skylineSurf = bgd.makeCityScape(init_msg.yb)
bgd.drawSun(skylineSurf)

bgd.drawGorilla(skylineSurf, init_msg.gor1)
bgd.drawGorilla(skylineSurf, init_msg.gor2)
bgd.drawText(init_msg.name1, skylineSurf, 2, 2, bgd.WHITE_COLOR, bgd.SKY_COLOR)
bgd.drawText(init_msg.name2, skylineSurf, 538, 2, bgd.WHITE_COLOR, bgd.SKY_COLOR)
winSurface.blit(skylineSurf, (0, 0))
pygame.display.update()

game.ready()

orient = 1
t0 = time()

turn = 2

iter_count = 0

while True:
    t0 = time()
    
    if iter_count == 0:
        turn = 3-turn
    
    msg = game.refresh()    
    if game.winner:
        break
    
    # update display from fields
    ban_x = msg.banana.x
    ban_y = 335 - msg.banana.y
        
    # check if it is a new turn
    if iter_count == 0:
        raiseArm(3-turn, False)
        raiseArm(turn, True)
        
    iter_count += 1
    
    if iter_count == 10:
        raiseArm(turn, False)
            
    wind = round(msg.wind)
    wind_str = wind > 0 and '>' or '<'
    bgd.drawText(wind_str*(abs(wind)), winSurface, bgd.SCR_WIDTH / 2, 40, bgd.WHITE_COLOR, bgd.SKY_COLOR, fontSize=20, pos='center')
        
    if msg.hit:
        if 0 <= ban_x < bgd.SCR_WIDTH:
            raiseArm(turn, False)
            pygame.draw.circle(skylineSurf, bgd.SKY_COLOR, (ban_x, ban_y), radius)
        iter_count = 0
    else:
        bgd.displayBanana(winSurface, orient, ban_x, ban_y)
        orient = orient < 3 and orient+1 or 0
        
    bgd.drawSun(skylineSurf, ban_x, ban_y)
    winSurface.blit(skylineSurf, (0, 0))
    pygame.display.update()

# victory dance
if game.winner in (1,2):
    for step in range(10):
        raiseArm(game.winner, True, step % 2)
        winSurface.blit(skylineSurf, (0, 0))
        pygame.display.update()
        pygame.time.wait(500)

# no possible draw in this game
if game.winner == -1:
    endword = "A bug stopped the game"
else:
    endword = "{} won the game".format(game.winner_name(init_msg))

pygame.time.wait(1000)
winSurface.fill(bgd.BLACK_COLOR)
bgd.drawText('G A M E   O V E R', winSurface, bgd.SCR_WIDTH / 2, 15, bgd.WHITE_COLOR, bgd.BLACK_COLOR, fontSize=40, pos='center')
bgd.drawText(endword, winSurface, bgd.SCR_WIDTH / 2, 110, bgd.WHITE_COLOR, bgd.BLACK_COLOR, pos='center')
pygame.display.update()

# wait for escape key or exit after 5000
t0 = pygame.time.get_ticks()
while pygame.time.get_ticks() - t0 < 5000:
    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            break
pygame.quit()
sys.exit()
