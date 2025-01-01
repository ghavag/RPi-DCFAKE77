# DCFAKE77

This program turns a device with a serial port into a DFC77 signal generator. This program is based on [this project](http://www.raspibo.org/wiki/index.php?title=DCFake77:_How_to_radio-control_your_clock_by_yourself) but heavy modified. The signal will not be transmitted over the air but one can think of the serial port as an replacement for a DFC77 receiver one can connect to their electronics project. The DFC77 signal will be provided by the DTR signal line and ground (pin 4 and 5 on DE-9 connector).


## How to compile

```
gcc dcfake77.c -o dcfake77
```
