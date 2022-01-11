
import matplotlib.pyplot as plt
import scipy.stats
import numpy as np


x_min = 0.0
x_max = 16.0

mean = 8.0 
std = 3.0

x = np.linspace(x_min, x_max, 200)

y = scipy.stats.norm.pdf(x,mean,std)
plt.figure(1)
plt.plot(x,y, color='black')
plt.show()
for  i in y:
    print(1000*i/max(y),',')