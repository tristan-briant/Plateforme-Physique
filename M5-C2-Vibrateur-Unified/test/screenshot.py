# -*- coding: utf-8 -*-
"""
Created on Sat Jul 13 10:30:13 2024

@author: Tristan Briant
Plateforme de Physique
Sorbonne Université
"""

import time

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
            if('1A86:55D4' in hwid or '303A:1001' in hwid):    # hwid of the M5CORE2
                address = port
                break
    
    if(address != ''):
        with serial.Serial(address, 115200, rtscts=False) as ser:
            # ser.setDTR(0)
            # ser.setRTS(0)
            b = bytes(x, 'utf-8')
            ser.write(b)
            ser.write(b'\n')
            ser.flush()
            time.sleep(0.01)    # needed on slow computers ?
    
    else:
        print('Robot non connecté')    
        

def query(x,timeout=0.1):
    
    import serial
    #import serial
    import serial.tools.list_ports
    import time
    
    ports = serial.tools.list_ports.comports()
    address=''

    for port, desc, hwid in ports:
            if('1A86:55D4' in hwid or '303A:1001' in hwid):    # hwid of the M5CORE2
                address = port
                break
    
    if(address != ''):
        with serial.Serial(address, 115200,timeout=timeout,rtscts=False) as ser:
           # ser.setDTR(0)
           # ser.setRTS(0)
           b = bytes(x, 'utf-8')
           ser.write(b)
           ser.write(b'\n')
           #ser.flush()
           # time.sleep(0.1)    # needed on slow computers ?
           
           #while ser.in_waiting:
               
           # answer = ser.readline()
           answer = ser.read(960)
           
           # for line in answer:
           #     print(line[:-1].decode('utf-8'))
           
           return answer # [].decode('utf-8')
           
    
    else:
        print('Robot non connecté') 
        return None
    

def screenshot(filename='image.ppm'):
    file = open(filename,'wb')
    
    file.write(b'P6\n')
    file.write(b'320 240\n')
    file.write(b'255\n')
    
    for i in range(240):
        ans = query('ss? {}'.format(i))
        print(i, '  ' ,len(ans))
        # time.sleep(0.1)
        #for line in ans:
        # file.write(ans.decode('utf-8'))
        file.write(ans)
    
    # file.write(0)
    file.close()
        
    
    