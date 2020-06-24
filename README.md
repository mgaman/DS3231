# DS3231
## Improved version of the Arduino library DS3131 Simple example
The orginal example has a builtin flaw in that the DS3231 is initialized to the COMPILE time of the code instead of the actual time.<br>
On my work machine the difference between compilation time and actual execution time can be around 20 seconds.<br>
I have added the possibility of initializing the DS3231 from an NTP server.<br>
Additionally I have added the possibilty of just reading the DS3231 and not initializing it.<br>
At the top of the source file are 2 define's. The user must enable (uncomment) just 1 or neither of these lines.
* If neither line is uncommented, code that just reads the DS3231 is compiled
* If COMPILE_TIME_SETUP is uncommented, the DS3231 is initialized to the time that the code was compiled.
* If NTP_TIME_SETUP is uncommented, the DS3131 is initialized to the actual time the code is executed.
Should you choose NTP_TIME_SETUP then you have to modify the lines setting the Daylight Saving Time rules see https://github.com/sstaub/NTP
