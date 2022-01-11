# 441_project
Little project on MCU for 411 class in M1 EEA at ENS Paris-Saclay

this project consist of the association of 3 MCU:

One is the master and play the melody (master_melody.c),
the second is a slave and play an harmonization (slave_chords.c),
the last one is also a slave and play a more rythmic accompagnment (slave_rythm.c)

When the button 1 of the master is pushed, the MCU send a 'l' by serial link to the slave and start to play the melody
When the slave receive the 'l' by serial link they also start to play their part.


The project is a bit simple because of a lack of time
But initially I tried to implement a beat with a drum sound on a MCU. 
So I undersampling a .wav file of a drum hit and implement it on the LPC608. 
However even if I used 2000 samples, the buzzer of the LPC608 could not retranscribe a sound without downsampling effects.
Conszquently I choose not to realize a beat accompagnment on a MCU.

In this depository I also add the python code which help me to create different sample list of different part of beat .wav
(sous_echant.py)

I also tried to synthetize a beat with a gaussian function but it was pretty much ugly so I did not used it but I join the python code to create a list of sample for this function. (gauss.py).

Finally the .wav used is also in this depository

I also started to code a version where the master send not only a signal to start but also the notes of the melody. And the slave with the melody note find the good hamronization by multiplying the frequency to play a fifth or a third. However this involve that the accompagnmenthas exactly the same rythm as the melody and it's not very beautiful, because a good acoompagnement play differently as the lead. So I stayed to the first versions of the programs even if they are more simple.

