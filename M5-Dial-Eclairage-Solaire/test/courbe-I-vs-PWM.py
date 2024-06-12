# -*- coding: utf-8 -*-
"""
Created on Sat May 25 11:39:03 2024

@author: Tristan Briant
"""

import matplotlib.pyplot as plt
import numpy as np
import pandas as pd

data=pd.read_csv('power-pwm.csv')
power = data['power']
pwm= data['pwm']
i = data['i']
v = data['v']


plt.plot(power,pwm/512.0)
plt.plot(power,i*0.001)

plt.plot(power,v/24.0)

