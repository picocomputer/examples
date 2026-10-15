import rp6502

# The program maps the tablet after it draws the picker, and the mapping
# clears earlier input, so the test starts after 60 frames.
with rp6502.Emu() as emu:
    emu.cmd("run 60")
    picker_x = (emu.sym["CANVAS_WIDTH"] - emu.sym["PICKER_WIDTH"]) // 2
    slot_3 = picker_x + 2 + 3 * 6
    emu.cmd(f"tablet at {slot_3} 4 1")
    emu.cmd("run 2")
    emu.cmd(f"tablet at {slot_3} 4 0")
    emu.cmd("run 2")
    emu.cmd("tablet at 160 120 1")
    emu.cmd("run 2")
    emu.cmd("peek draw_color $03")
