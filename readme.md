# HDSP2000_Display
HDSP2000 Display

## Connections:

Column Data:
- Arduino pin 2-6 -> UDN2981A inputs 1-5 (pins 1-5)
- UDN2981A outputs 1-5 (pins **18-14**) -> HDSP2000 columns 1-5 (pins 1-5)

Data:
- Arduino pin 8 -> HDSP2000 pin 12 (data input)
- HDSP2000 #1 pin 7 (data output) -> HDSP2000 #2 pin 12 (data input)

Clock:
- Arduino pin 9 -> HDSP2000 pin 10

Power:
- HDSP2000 pin 11 -> GND
- HDSP2000 pin 9 -> 5V
- UDN2981A pin 9 -> 3.3V
- UDN2981A pin 10 -> GND