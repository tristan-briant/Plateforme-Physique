import robot
import time
import keyboard

print('press esc to quit')

while True:
    oeil_G , oeil_D = robot.sensor()  # Que voient les yeux ? blanc -> True, noir -> False
    
    if oeil_G == True and oeil_D == True :
        robot.speed(100,100)          # les deux moteurs à 100%
    
    if oeil_G == True and oeil_D == False :
        robot.speed(100,0)            # 100% à gauche 0% à droite
    
    if oeil_G == False and oeil_D == True :
        robot.speed(0,100)            # 0% à gauche 100% à droite
    
    if oeil_G == False and oeil_D == False :
        robot.speed(0,0)             # le robot s'arrête
        
    
    if keyboard.is_pressed('esc'):  # presser la touche 'echap' pour quitter le programme
        break
    
    time.sleep(0.01)     # attend 0.1 secondes
    