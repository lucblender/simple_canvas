# Instructions on how to build the simple micro easel

## Before you start
On this document you'll find the instructions to assemble the micro easel. The first step is to go check the [simple micro easel wiring cheat sheet](simple_micro_easel_wiring_cheat_sheet.pdf) print it, keep it by your side the whole building time and add little checkmark when you did put a wire. In this document I'll refer it as the "cheat sheet".

On this cheat sheet you have the main table with all the wiring and two side tables showing the pinout of the daisy and the MCP23017 gpio expander pinout.
The main table top pin and bottom pin refer to the simple board pins (top and bottom rows). The "Daisy Pin" collum give the real pin name. Note : there is also MCP23017 pin name in this collum, all the \_A0 to \_A7 and \_B0 to \_B7.

### Final result
Here is the final result: 
![Main board Top](ressources/pictures/Instructions%20(1).jpg)

### General tips
As you can see there is a lot of wires everywhere. And those wires are going under some 3D prints part used to hold the LEDs in place. To make it simple for you to solder try to use colour wires for example use different color for analog, digitals and data signals. Also use the thinner wires you have to not create chuncky wire bloc that would not let you put the front pannel. Also as you saw on the picture above, lot of wires are under the 3D printed parts. To avoid this problem (I didn't) you can solder as many wire as possible on the bottom of the PCB.

## Time to solder

### Power 
After putting all the components in their correct slot accoarding to the following table (TODO TABLE), you can start to wire the power of the daisy seed. They correspond to last three line of the cheat sheet.

### Linear Potentiometer

Lin pot and 5x lin pots on the cheat sheet.

The 5 linear potentiometers uses only one analog gpio and 5 digital gpios. The firmware will select one potentiometer at a time with a digital gpio and then read its value. To avoid potentiometers that are not selected, we need to add diodes to their ouptput before combining them.
You'll need to solder each output of the potentiometers to a diode anode (**top** pin 5, 10, 15, 20, 25). Then solder all those diodes cathodes together and wire it to the **bottom** pin 30. (cheat sheet 5x lin pots)
![Diodes Linear potentiometers](ressources/pictures/Instructions%20(3).jpg)

You can now wire the 5 digitals gpios to the inputs of the 5 potentiometers. (cheat sheet lin pot). I first did a little wrap on the bottom to route the linear potentiometers pin to pins that are connected to the bottom row. 
![Potentiometers digital gpios](ressources/pictures/Instructions%20(7).jpg)
Finish by wiring the 5 top pins to the 5 bottom pins by following the cheat sheet.

