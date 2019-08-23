# NaTE - Not a True Emulator

A codegolf challenge captured my attention, [Emulate an Intel 8086 CPU](https://codegolf.stackexchange.com/questions/4732/emulate-an-intel-8086-cpu).

Here is the result:

![Test Program](https://i.ibb.co/88Z2cMK/codegolf.png)

## Project Files

`ild.c` is the core that fulfill the "fetch and decode" step. It has been extracted from a personal project that will be soon available.

`cpu.c` offers a basic CPU initialization.

`cpu_exec.c` emulates an instruction previously disassembled by ild.

Please note that the video memory is not properly emulated; features such as segments and interrupts are not supported.

#### LICENSE
The program is licensed under GPL v.3.
