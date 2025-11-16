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
    try :
        import serial
    except:
        print('pyserial not installed')
        return

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
            ser.write(b'\n')
            ser.flush()
            time.sleep(0.01)    # needed on slow computers ?
    
    else:
        print('Robot non connecté')    
        

def query(x,timeout=1.0):
    
    import serial
    #import serial
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
           ser.write(b'\n')
           ser.flush()
           time.sleep(0.01)    # needed on slow computers ?
           
           answer = ser.readline()
           # print(answer)
           
           return answer[:-2].decode('utf-8')
           
    
    else:
        print('Robot non connecté') 
        return None
    

def sensor(side='both'):
    '''
    Return the state of the sensor

    Parameters
    ----------
    side : String
        'RIGHT','LEFT','BOTH'
    
    Returns
    -------
    right,left : BOOL
        True if level high

    '''
    
    answer=query('SENSOR?',0.5)

    if side.upper()=='BOTH':
        return answer[0]=='1' , answer[2]=='1' 
    if side.upper()=='RIGHT':
        return answer[2]=='1'
    if side.upper()=='LEFT':
        return answer[0]=='1'
    

def move(x_left,x_right=None, non_blocking=False) : 
    '''
    Move the robot

    Parameters
    ----------
    x_right,x_left : FLOAT
        Distance to travel in mm for right and left wheel.
    
    non_blocking : BOOL
        if True return imediatly otherwize wait the motion to complete
    Returns
    -------
    None.

    '''
    import time


    if(x_right==None):
        sendstring("MOVE {}".format(x_left))
    else:
        sendstring("MOVE {},{}".format(x_left,x_right))
      
    if not non_blocking:
        while(True):
            if(query("MOVING?")=='0'):
                break
            else:
                time.sleep(0.01)
            
def stop():
    '''
    Stop the motors
    '''
    sendstring("SPEED 0")


def speed(speed_left,speed_right=None):
    '''
    Set the speed of right and left motor

    Parameters
    ----------
    speed_right, speed_left : FLOAT
        speed in % for right and left wheel.

    Returns
    -------
    None.

    '''
    
    if(speed_left==None):
        sendstring("SPEED {}".format(speed_left))
    else:
        sendstring("MOVE {},{}".format(speed_left,speed_right))
        
    
   
        
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
    
    sendstring("TURN {}".format(angl))
    
   
