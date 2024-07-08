# pico-delaygen

Delay generator will appear as a new serial device under `/dev/tty.usb*` with default baud rate of 115200.  
As of now, only the commands marked with a tick symbol below, are supported.

Recommended way of operation:
1. Set up serial connection
2. Soft reset (optional)
3. Set pulse width in clock cycles (default: 8ns each)
4. Set delay in clock cycles (default: 8ns each)
5. Arm the device using "glitch" command
6. Wait for state to return to 0 (idle)

## Compilation
```zsh
PICO_SDK_PATH=/path/to/pico-sdk cmake -S . -B build
cmake --build build
```

## Command list
Fully compatible with FPGA-based [chipfail-glitcher](https://github.com/unixb0y/chipfail-glitcher), but not all commands are functional yet. 
```
Soft Reset         = b"@" = 64 = 0x40 ✓ 
Toggle LED         = b"A" = 65 = 0x41 ✓ 
Power Cycle        = b"B" = 66 = 0x42  
Set Pulse Width    = b"C" = 67 = 0x43 ✓ 
Set Delay          = b"D" = 68 = 0x44 ✓  
Set Power Pulse    = b"E" = 69 = 0x45  
Glitch             = b"F" = 70 = 0x46 ✓  
Read GPIO 0-7      = b"G" = 71 = 0x47  
Enable power cycle = b"H" = 72 = 0x48  
Get state          = b"I" = 73 = 0x49 ✓  
```
