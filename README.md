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
