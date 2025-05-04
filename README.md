# RP6502 Examples

Examples for your Picocomputer 6502.
For detailed setup information, see:<br/>
https://github.com/picocomputer/rp6502-vscode

You must have on your development system:
 * [VSCode](https://code.visualstudio.com/). This has its own installer.
 * A source install of [CC65](https://cc65.github.io/getting-started.html).
 * The following suite of tools for your specific OS.
```
$ sudo apt-get install cmake python3 pip git build-essential
$ pip install pyserial
```

Change the CMake launch target to select which example to run.
Press F5 or "Debug: Start Debugging" to build target and upload
it to your Picocomputer.
