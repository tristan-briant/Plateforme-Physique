# -*- coding: utf-8 -*-
"""
Created on Sat Jul 13 10:30:13 2024

@author: Tristan Briant
Plateforme de Physique
Sorbonne Université
"""

def sendstring(x):
    '''
    Send a command to the motor driver

    Parameters
    ----------
    x : string

    Returns
    -------
    None.

    '''
    
    import serial
    import serial.tools.list_ports
    import time
    
    ports = serial.tools.list_ports.comports()
    address=''

    for port, desc, hwid in ports:
            if('1A86:55D4' in hwid):    # hwid of the M5CORE2
                address = port
                break
    
    if(address != ''):
        with serial.Serial(address, 115200) as ser:
            b = bytes(x, 'utf-8')
            ser.write(b)
            ser.flush()
            time.sleep(0.01)    # needed on slow computers ?
    
    else:
        print('Robot non connecté')    
        

def query(x,timeout=1.0):
    import serial
    import serial.tools.list_ports
    import time
    
    ports = serial.tools.list_ports.comports()
    address=''

    for port, desc, hwid in ports:
            if('1A86:55D4' in hwid):    # hwid of the M5CORE2
                address = port
                break
    
    if(address != ''):
        with serial.Serial(address, 115200,timeout=timeout) as ser:
           
           b = bytes(x, 'utf-8')
           ser.write(b)
           ser.flush()
           time.sleep(0.1)    # needed on slow computers ?
           
           answer = ser.readline(10)
           # print(answer)
           
           return answer[:-2]
           
    
    else:
        print('Robot non connecté') 
        return None
    

def sensor(side='RIGHT'):
    '''
    Move the translation robot

    Parameters
    ----------
    side : String
        'RIGHT','LEFT','BOTH'
    
    Returns
    -------
    right,left : BOOL
        True if level high

    '''
    
    answer=query('SENSOR?\n',0.5).decode('utf-8')
    
    if side=='BOTH':
        return answer[0]=='1' , answer[2]=='1' 
    if side=='RIGHT':
        return answer[0]=='1'
    if side=='LEFT':
        return answer[2]=='1'
    

def move(x_right,x_left=None) : 
    '''
    Move the robot

    Parameters
    ----------
    x_right,x_left : FLOAT
        Distance to travel in mm for right and left wheel.

    Returns
    -------
    None.

    '''
    import time


    if(x_left==None):
        sendstring("MOVE {}\n".format(x_right))
    else:
        sendstring("MOVE {},{}\n".format(x_right,x_left))
        
    while(True):
        if(query("MOVING?\n").decode('utf-8')=='0'):
            break
        else:
            time.sleep(0.1)
            
    
   
        
def turn(angle):
    '''
    Move the translation stage to a defined position

    Parameters
    ----------
    x : FLOAT
        position to reach in mm.

    Returns
    -------
    None.

    '''
    
    sendstring("TURN {}\n".format(angl))
    
   
