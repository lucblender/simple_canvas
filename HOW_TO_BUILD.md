# Instructions on how to build the simple micro easel

## Before you start
On this document you'll find the instructions to assemble the micro easel. The first step is to go check the [simple micro easel wiring cheat sheet](simple_micro_easel_wiring_cheat_sheet.pdf) print it, keep it by your side the whole building time and add little checkmark when you did put a wire. In this document I'll refer it as the "cheat sheet".

On this cheat sheet you have the main table with all the wiring and two side tables showing the pinout of the daisy and the MCP23017 gpio expander pinout.
The main table top pin and bottom pin refer to the simple board pins (top and bottom rows). The "Daisy Pin" collum give the real pin name. Note : there is also MCP23017 pin name in this collum, all the \_A0 to \_A7 and \_B0 to \_B7.

## Test Code

A test code to help you debug your hardware can be found in the software folder and is named [simple_micro_easel_hardware_test.ino](software/simple_micro_easel_hardware_test/simple_micro_easel_hardware_test.ino). In the loop function, you can comment and uncomment the function to test the different part of the simple micro easel. In this document I'll refer this code as the "test code".

### Final result
Here is the final result: 
![Main board Top](ressources/pictures/Instructions%20(1).jpg)

### General tips
As you can see there is a lot of wires everywhere. And those wires are going under some 3D prints part used to hold the LEDs in place. To make it simple for you to solder try to use colour wires for example use different color for analog, digitals and data signals. Also use the thinner wires you have to not create chuncky wire bloc that would not let you put the front pannel. Also as you saw on the picture above, lot of wires are under the 3D printed parts. To avoid this problem (I didn't) you can solder as many wire as possible on the bottom of the PCB.

## Time to solder

### Power and output jack
After putting all the components in their correct slot accoarding to the following table (TODO TABLE), you can start to wire the power of the daisy seed. They correspond to last three line of the cheat sheet.
You can also solder the two output jack, you can find them in the cheat sheet by searching for jack in the component type collumn.

### Linear Potentiometer

Lin pot and 5x lin pots on the cheat sheet.

The 5 linear potentiometers uses only one analog gpio and 5 digital gpios. The firmware will select one potentiometer at a time with a digital gpio and then read its value. To avoid potentiometers that are not selected, we need to add diodes to their ouptput before combining them.
You'll need to solder each output of the potentiometers to a diode anode (**top** pin 5, 10, 15, 20, 25). Then solder all those diodes cathodes together and wire it to the **bottom** pin 30. (cheat sheet 5x lin pots)
![Diodes Linear potentiometers](ressources/pictures/Instructions%20(3).jpg)

You can now wire the 5 digitals gpios to the inputs of the 5 potentiometers. (cheat sheet lin pot). I first did a little wrap on the bottom to route the linear potentiometers pin to pins that are connected to the bottom row. 
![Potentiometers digital gpios](ressources/pictures/Instructions%20(7).jpg)
Finish by wiring the 5 top pins to the 5 bottom pins by following the cheat sheet.

You can test all the linear potentiometer with the test code by uncommenting ```sequencerStepsRead();```.

### Rotary potentiometers

Next step are the rotary potentiometers. Simply search for _pot_ in Component type collumn of the cheat sheet and solder their corresponding top to bottom row.
You can test all the linear potentiometer with the test code by uncommenting ```analogsRead();();```.

### Digital pins connected to the daisy

Only 5 digital pins are connected from the board to the daisy. You can find them in the cheat sheet by their Signal Name _Step0 to Step4_. You can solder their corresponding top to bottom row.

You can test all the linear potentiometer with the test code by uncommenting ```digitalPinsread();```.

### Digital pins connected to the MCP23017 

The rest of the digital pins are connected to the gpio expander. The 1st step is to connect all the 3 position switch second position to an accessible top row gpio.

You can find them by searching _switch 3 pos 0_ and _switch 3 pos 1_ in Component type collumn of the cheat sheet. The _switch 3 pos 0_ are already connected by the footprint, you'll need to connect all the _switch 3 pos 1_  wires from the footprint to their corresponding Pin top. You can refer to the following pictures, it's the 6 little yellow wires next to the switches in the middle of the board. 

![3 position switches](ressources/pictures/Instructions%20(8).jpg)

After this step, you can wire the MCP23017. Search for _gpioExpander_ in the Component type collumn. You'll have two interrupts signal and i2c pins. Don't forget to also put 3V3 and GND to the MCP23017!

You can now wire all the digital Gpios from the top row to the MCP23071 Gpios.  Search for _\_A0 to \_A7 and \_B0 to \_B7_ in the cheat sheet.

You can test all the MCP23017 gpios with the test code by uncommenting ```mcpPinsRead();```.

### Capacitive touch sensor

//TODO

### Leds

//TODO

#### 3D Printing interludes

You can find all the part to print inside the [hardware](hardware) folder. All the parts are in 3mf and stl. You'll find:
- Led_holder_main : main leds holder
- Led_holder_source : holder of the 4 sources leds
- Blocker_source : piece used to block light diffusion for the source leds
- Blocker_destination : piece used to block light diffusion for the destination leds
