# RP6502 Examples

Examples for your Picocomputer 6502.
The install steps for the tools are in RP6502-SDK:<br/>
https://picocomputer.github.io/sdk.html#sdk-install

Select a CMake configure preset to choose a compiler, then change
the CMake launch target to select which example to run.
Press F5 or "Debug: Start Debugging" to build the target and run it
in the emulator. Choose "RP6502 (Hardware)" in the Run and Debug side
panel to run it on your Picocomputer instead.

## Play in your browser

<!-- rp6502
preset: cc65/Release
target: adventure
title: Colossal Cave Adventure
overlay: no
footer: Type a command, then press Enter.
-->
[Colossal Cave Adventure](https://picocomputer.github.io/examples/adventure/)
by Will Crowther and Don Woods.

<!-- rp6502
preset: cc65/Release
target: furelise
title: Fur Elise
-->
[Fur Elise](https://picocomputer.github.io/examples/furelise/) by Ludwig
van Beethoven on the PSG.

<!-- rp6502
preset: cc65/Release
target: gamepad
title: Gamepads
overlay: no
-->
[Gamepads](https://picocomputer.github.io/examples/gamepad/) shows the
sticks, triggers, hat and buttons of up to four gamepads.

<!-- rp6502
preset: cc65/Release
target: mandelbrot
title: Mandelbrot
overlay: no
-->
[Mandelbrot](https://picocomputer.github.io/examples/mandelbrot/) draws the
Mandelbrot set on a 320x240 canvas.

<!-- rp6502
preset: cc65/Release
target: paint
title: Paint
overlay: no
footer: Paint with the mouse, a pen or a finger.
-->
[Paint](https://picocomputer.github.io/examples/paint/) paints with a
tablet, a touchscreen or a mouse.
