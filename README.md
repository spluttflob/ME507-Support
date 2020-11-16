# ME507 Class Support Programs
The code in this repository is here to support the ME507 mechatronics course
at California Polytechnic State University, San Luis Obispo.  If anyone else
would like to use it, please do; we make it available under a GPL license. 

We have code which makes multitasking programs a bit easier to write using
FreeRTOS on either STM32 or ESP32 processors:
* `baseshare.*`
* `taskshare.h`
* `taskqueue.h`
* The examples `main.cpp` and `task_receive.*`

There are also some utility classes, such as a class that allows incremental 
encoders to be used with the encoder reading _hardware_ in the timers of STM32's 
(this seems not to be readily available from other sources).  The encoder
counter code is new, pre-alpha, barely tested, and only known to run on 
STM32L476RG processors, but hopefully it will work on others. 
* `encoder_counter.*`
* The example `encoder_test.cpp`

## Documentation
The author didn't write all those Doxygen comments for nothing. Have a look: 
<https://spluttflob.github.io/ME507-Support/>

