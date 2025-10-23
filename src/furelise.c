/*
 * Copyright (c) 2023 Rumbledethumps
 *
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-License-Identifier: Unlicense
 */

#include "ezpsg.h"
#include <rp6502.h>
#include <stdlib.h>
#include <stdio.h>

#define wait(duration) (duration)
#define piano(note, duration) (-1), (note), (duration)
#define end() (0)

#define bar_0 piano(e5, 2),  \
              wait(2),       \
              piano(ds5, 2), \
              wait(2)
#define bar_1 piano(e5, 2),  \
              wait(2),       \
              piano(ds5, 2), \
              wait(2),       \
              piano(e5, 2),  \
              wait(2),       \
              piano(b4, 2),  \
              wait(2),       \
              piano(d5, 2),  \
              wait(2),       \
              piano(c5, 2),  \
              wait(2)
#define bar_2 piano(a4, 4),  \
              piano(a2, 12), \
              wait(2),       \
              piano(e3, 10), \
              wait(2),       \
              piano(a3, 8),  \
              wait(2),       \
              piano(c4, 6),  \
              wait(2),       \
              piano(e4, 4),  \
              wait(2),       \
              piano(a4, 2),  \
              wait(2)
#define bar_3 piano(b4, 4),  \
              piano(e2, 12), \
              wait(2),       \
              piano(e3, 10), \
              wait(2),       \
              piano(gs3, 8), \
              wait(2),       \
              piano(e4, 6),  \
              wait(2),       \
              piano(gs4, 4), \
              wait(2),       \
              piano(b4, 2),  \
              wait(2)
#define bar_4 piano(c5, 4),  \
              piano(a2, 12), \
              wait(2),       \
              piano(e3, 10), \
              wait(2),       \
              piano(a3, 8),  \
              wait(2),       \
              piano(e4, 6),  \
              wait(2),       \
              piano(e5, 4),  \
              wait(2),       \
              piano(ds5, 2), \
              wait(2)
#define bar_5 piano(e5, 2),  \
              wait(2),       \
              piano(ds5, 2), \
              wait(2),       \
              piano(e5, 2),  \
              wait(2),       \
              piano(b4, 2),  \
              wait(2),       \
              piano(d5, 2),  \
              wait(2),       \
              piano(c5, 2),  \
              wait(2)
#define bar_6 piano(a4, 4),  \
              piano(a2, 12), \
              wait(2),       \
              piano(e3, 10), \
              wait(2),       \
              piano(a3, 8),  \
              wait(2),       \
              piano(c4, 6),  \
              wait(2),       \
              piano(e4, 4),  \
              wait(2),       \
              piano(a4, 2),  \
              wait(2)
#define bar_7 piano(b4, 4),  \
              piano(e2, 12), \
              wait(2),       \
              piano(e3, 10), \
              wait(2),       \
              piano(gs3, 8), \
              wait(2),       \
              piano(d4, 6),  \
              wait(2),       \
              piano(c5, 4),  \
              wait(2),       \
              piano(b4, 2),  \
              wait(2)
#define bar_8_1 piano(a4, 8), \
                piano(a2, 4), \
                wait(2),      \
                piano(e3, 4), \
                wait(2),      \
                piano(a3, 4), \
                wait(4)
#define bar_8_2 piano(a4, 4), \
                piano(a2, 2), \
                wait(2),      \
                piano(e3, 2), \
                wait(2),      \
                piano(a3, 2), \
                wait(2),      \
                piano(b4, 2), \
                wait(2),      \
                piano(c5, 2), \
                wait(2),      \
                piano(d5, 2), \
                wait(2)
#define bar_9 piano(e5, 6),  \
              piano(c3, 12), \
              wait(2),       \
              piano(g3, 10), \
              wait(2),       \
              piano(c4, 8),  \
              wait(2),       \
              piano(g4, 6),  \
              wait(2),       \
              piano(f5, 4),  \
              wait(2),       \
              piano(e5, 2),  \
              wait(2)
#define bar_10 piano(d5, 6),  \
               piano(g2, 12), \
               wait(2),       \
               piano(g3, 10), \
               wait(2),       \
               piano(b3, 8),  \
               wait(2),       \
               piano(f4, 6),  \
               wait(2),       \
               piano(e5, 4),  \
               wait(2),       \
               piano(d5, 2),  \
               wait(2)
#define bar_11 piano(c5, 6),  \
               piano(a2, 12), \
               wait(2),       \
               piano(e3, 10), \
               wait(2),       \
               piano(a3, 8),  \
               wait(2),       \
               piano(e4, 8),  \
               wait(2),       \
               piano(d5, 8),  \
               wait(2),       \
               piano(c5, 8),  \
               wait(2)
#define bar_12 piano(b4, 8), \
               piano(e2, 8), \
               wait(2),      \
               piano(e3, 8), \
               wait(2),      \
               piano(e4, 8), \
               wait(2),      \
               piano(e4, 8), \
               wait(2),      \
               piano(e5, 8), \
               wait(2),      \
               piano(e4, 8), \
               wait(2)
#define bar_13 piano(e5, 8),  \
               wait(2),       \
               piano(e5, 8),  \
               wait(2),       \
               piano(e6, 8),  \
               wait(2),       \
               piano(ds5, 6), \
               wait(2),       \
               piano(e5, 4),  \
               wait(2),       \
               piano(ds5, 2), \
               wait(2)
#define bar_14 piano(e5, 4),  \
               wait(2),       \
               piano(ds5, 2), \
               wait(2),       \
               piano(e5, 2),  \
               wait(2),       \
               piano(ds5, 2), \
               wait(2),       \
               piano(e5, 2),  \
               wait(2),       \
               piano(ds5, 2), \
               wait(2)
#define bar_15 piano(e5, 2),  \
               wait(2),       \
               piano(ds5, 2), \
               wait(2),       \
               piano(e5, 2),  \
               wait(2),       \
               piano(b4, 2),  \
               wait(2),       \
               piano(d5, 2),  \
               wait(2),       \
               piano(c5, 2),  \
               wait(2)
#define bar_16 piano(a4, 4),  \
               piano(a2, 12), \
               wait(2),       \
               piano(e3, 10), \
               wait(2),       \
               piano(a3, 8),  \
               wait(2),       \
               piano(c4, 8),  \
               wait(2),       \
               piano(e4, 8),  \
               wait(2),       \
               piano(a4, 8),  \
               wait(2)
#define bar_17 piano(b4, 4),  \
               piano(e2, 12), \
               wait(2),       \
               piano(e3, 10), \
               wait(2),       \
               piano(gs3, 8), \
               wait(2),       \
               piano(e4, 8),  \
               wait(2),       \
               piano(gs4, 8), \
               wait(2),       \
               piano(b4, 8),  \
               wait(2)
#define bar_18 piano(c5, 4),  \
               piano(a2, 12), \
               wait(2),       \
               piano(e3, 10), \
               wait(2),       \
               piano(a3, 8),  \
               wait(2),       \
               piano(e4, 8),  \
               wait(2),       \
               piano(e5, 8),  \
               wait(2),       \
               piano(ds5, 8), \
               wait(2)
#define bar_21 piano(b4, 4),  \
               piano(e2, 12), \
               wait(2),       \
               piano(e3, 10), \
               wait(2),       \
               piano(gs3, 8), \
               wait(2),       \
               piano(d4, 8),  \
               wait(2),       \
               piano(c5, 8),  \
               wait(2),       \
               piano(b4, 8),  \
               wait(2)
#define bar_22_1 piano(a4, 4), \
                 piano(a2, 2), \
                 wait(2),      \
                 piano(e3, 2), \
                 wait(2),      \
                 piano(a3, 2), \
                 wait(2),      \
                 piano(b4, 2), \
                 wait(2),      \
                 piano(c5, 2), \
                 wait(2),      \
                 piano(d5, 2), \
                 wait(2)
#define bar_22_2 piano(a4, 4),  \
                 piano(a2, 2),  \
                 wait(2),       \
                 piano(e3, 2),  \
                 wait(2),       \
                 piano(a3, 2),  \
                 wait(2),       \
                 piano(as3, 2), \
                 piano(c4, 2),  \
                 piano(e4, 2),  \
                 piano(c5, 2),  \
                 wait(2),       \
                 piano(a3, 2),  \
                 piano(c4, 2),  \
                 piano(f4, 2),  \
                 piano(c5, 2),  \
                 wait(2),       \
                 piano(g3, 2),  \
                 piano(as3, 2), \
                 piano(c4, 2),  \
                 piano(e4, 2),  \
                 piano(g4, 2),  \
                 piano(c5, 2),  \
                 wait(2)
#define bar_23 piano(f4, 2), \
               piano(a4, 2), \
               piano(c5, 8), \
               piano(f3, 2), \
               wait(2),      \
               piano(a3, 2), \
               wait(2),      \
               piano(c4, 2), \
               wait(2),      \
               piano(a3, 2), \
               wait(2),      \
               piano(f5, 3), \
               piano(c4, 2), \
               wait(2),      \
               piano(a3, 2), \
               wait(1),      \
               piano(e5, 1), \
               wait(1)
#define bar_24 piano(e5, 4),  \
               piano(d3, 2),  \
               wait(2),       \
               piano(as3, 2), \
               wait(2),       \
               piano(d5, 2),  \
               piano(d4, 2),  \
               wait(2),       \
               piano(as3, 2), \
               wait(2),       \
               piano(as5, 3), \
               piano(d4, 2),  \
               wait(2),       \
               piano(as3, 2), \
               wait(1),       \
               piano(a5, 1),  \
               wait(1)
#define bar_25 piano(a5, 2),  \
               piano(d3, 2),  \
               wait(2),       \
               piano(g5, 2),  \
               piano(e4, 2),  \
               wait(2),       \
               piano(f5, 2),  \
               piano(d3, 2),  \
               piano(e3, 2),  \
               piano(fs3, 2), \
               wait(2),       \
               piano(e5, 2),  \
               piano(e4, 2),  \
               wait(2),       \
               piano(d5, 2),  \
               piano(d3, 2),  \
               piano(e3, 2),  \
               piano(fs3, 2), \
               wait(2),       \
               piano(c5, 2),  \
               piano(e4, 2),  \
               wait(2)
#define bar_26 piano(as4, 4), \
               piano(f3, 2),  \
               wait(2),       \
               piano(a3, 2),  \
               wait(2),       \
               piano(a4, 4),  \
               piano(c4, 2),  \
               wait(2),       \
               piano(as4, 4), \
               piano(a3, 2),  \
               wait(2),       \
               piano(a4, 1),  \
               piano(c4, 2),  \
               wait(1),       \
               piano(g4, 1),  \
               wait(1),       \
               piano(a4, 1),  \
               piano(a3, 2),  \
               wait(1),       \
               piano(as4, 1), \
               wait(1)
#define bar_27 piano(c5, 8),  \
               piano(f3, 2),  \
               wait(2),       \
               piano(a3, 2),  \
               wait(2),       \
               piano(c4, 2),  \
               wait(2),       \
               piano(a3, 2),  \
               wait(2),       \
               piano(d5, 2),  \
               piano(c4, 2),  \
               wait(2),       \
               piano(ds5, 2), \
               piano(a3, 2),  \
               wait(2)
#define bar_28 piano(e5, 6), \
               piano(e3, 2), \
               wait(2),      \
               piano(a3, 2), \
               wait(2),      \
               piano(c4, 2), \
               wait(2),      \
               piano(e5, 2), \
               piano(a3, 2), \
               wait(2),      \
               piano(f5, 2), \
               piano(d4, 2), \
               piano(d3, 2), \
               wait(2),      \
               piano(a4, 2), \
               piano(f3, 2), \
               wait(2)
#define bar_29 piano(c5, 8), \
               piano(g3, 2), \
               wait(2),      \
               piano(e4, 2), \
               wait(2),      \
               piano(g3, 2), \
               wait(2),      \
               piano(f4, 2), \
               wait(2),      \
               piano(d5, 3), \
               piano(g3, 2), \
               wait(2),      \
               piano(f4, 2), \
               wait(1),      \
               piano(b4, 1), \
               wait(1)
#define bar_30 piano(c5, 1), \
               piano(c4, 4), \
               piano(e4, 4), \
               wait(1),      \
               piano(g5, 1), \
               wait(1),      \
               piano(g4, 1), \
               wait(1),      \
               piano(g5, 1), \
               wait(1),      \
               piano(a4, 1), \
               wait(1),      \
               piano(g5, 1), \
               wait(1),      \
               piano(b4, 1), \
               piano(f4, 2), \
               piano(g4, 2), \
               wait(1),      \
               piano(g5, 1), \
               wait(1),      \
               piano(c5, 1), \
               piano(e4, 2), \
               piano(g4, 2), \
               wait(1),      \
               piano(g5, 1), \
               wait(1),      \
               piano(d5, 1), \
               piano(d4, 2), \
               piano(f4, 2), \
               piano(g4, 2), \
               wait(1),      \
               piano(g5, 1), \
               wait(1)
#define bar_31 piano(e5, 1), \
               piano(g4, 4), \
               piano(e4, 4), \
               piano(c4, 4), \
               wait(1),      \
               piano(g5, 1), \
               wait(1),      \
               piano(c6, 1), \
               wait(1),      \
               piano(b5, 1), \
               wait(1),      \
               piano(a5, 1), \
               piano(f3, 4), \
               piano(a3, 4), \
               wait(1),      \
               piano(g5, 1), \
               wait(1),      \
               piano(f4, 1), \
               wait(1),      \
               piano(e5, 1), \
               wait(1),      \
               piano(d5, 1), \
               piano(g3, 4), \
               piano(b3, 4), \
               wait(1),      \
               piano(g5, 1), \
               wait(1),      \
               piano(f5, 1), \
               wait(1),      \
               piano(d5, 1), \
               wait(1)
#define bar_32 piano(c5, 1), \
               piano(c4, 4), \
               wait(1),      \
               piano(g5, 1), \
               wait(1),      \
               piano(g4, 1), \
               wait(1),      \
               piano(g5, 1), \
               wait(1),      \
               piano(a4, 1), \
               wait(1),      \
               piano(g5, 1), \
               wait(1),      \
               piano(b4, 1), \
               piano(f4, 2), \
               piano(g4, 2), \
               wait(1),      \
               piano(g5, 1), \
               wait(1),      \
               piano(c5, 1), \
               piano(e4, 2), \
               piano(g4, 2), \
               wait(1),      \
               piano(g5, 1), \
               wait(1),      \
               piano(d5, 1), \
               piano(d4, 2), \
               piano(f4, 2), \
               piano(g4, 2), \
               wait(1),      \
               piano(g5, 1), \
               wait(1)
#define bar_33 piano(e5, 1), \
               piano(e4, 4), \
               piano(c4, 4), \
               wait(1),      \
               piano(g5, 1), \
               wait(1),      \
               piano(c6, 1), \
               wait(1),      \
               piano(b5, 1), \
               wait(1),      \
               piano(a5, 1), \
               piano(f3, 4), \
               piano(a3, 4), \
               wait(1),      \
               piano(g5, 1), \
               wait(1),      \
               piano(f4, 1), \
               wait(1),      \
               piano(e5, 1), \
               wait(1),      \
               piano(d5, 1), \
               piano(g3, 4), \
               piano(b3, 4), \
               wait(1),      \
               piano(g5, 1), \
               wait(1),      \
               piano(f5, 1), \
               wait(1),      \
               piano(d5, 1), \
               wait(1)
#define bar_34 piano(e5, 1),  \
               piano(b3, 4),  \
               piano(gs4, 4), \
               wait(1),       \
               piano(f5, 1),  \
               wait(1),       \
               piano(e5, 1),  \
               wait(1),       \
               piano(ds5, 1), \
               wait(1),       \
               piano(e5, 1),  \
               wait(1),       \
               piano(b4, 1),  \
               wait(1),       \
               piano(e5, 1),  \
               wait(1),       \
               piano(ds5, 1), \
               wait(1),       \
               piano(e5, 1),  \
               wait(1),       \
               piano(b4, 1),  \
               wait(1),       \
               piano(e5, 1),  \
               wait(1),       \
               piano(ds5, 1), \
               wait(1)
#define bar_35 piano(e5, 6),  \
               wait(6),       \
               piano(b4, 2),  \
               wait(2),       \
               piano(e5, 2),  \
               wait(2),       \
               piano(ds5, 2), \
               wait(2)
#define bar_36 piano(e5, 6),  \
               wait(6),       \
               piano(b4, 2),  \
               wait(2),       \
               piano(e5, 2),  \
               wait(2),       \
               piano(ds5, 2), \
               wait(2)
#define bar_37 piano(e5, 2),  \
               wait(2),       \
               piano(ds5, 2), \
               wait(2),       \
               piano(e5, 2),  \
               wait(2),       \
               piano(ds5, 2), \
               wait(2),       \
               piano(e5, 2),  \
               wait(2),       \
               piano(ds5, 2), \
               wait(2)
#define bar_38 piano(e5, 2),  \
               wait(2),       \
               piano(ds5, 2), \
               wait(2),       \
               piano(e5, 2),  \
               wait(2),       \
               piano(b4, 2),  \
               wait(2),       \
               piano(d5, 2),  \
               wait(2),       \
               piano(c5, 2),  \
               wait(2)
#define bar_39 piano(a4, 4),  \
               piano(a2, 12), \
               wait(2),       \
               piano(e3, 10), \
               wait(2),       \
               piano(a3, 8),  \
               wait(2),       \
               piano(c4, 6),  \
               wait(2),       \
               piano(e4, 4),  \
               wait(2),       \
               piano(a4, 2),  \
               wait(2)
#define bar_40 piano(b4, 4),  \
               piano(c2, 12), \
               wait(2),       \
               piano(e3, 10), \
               wait(2),       \
               piano(gs3, 8), \
               wait(2),       \
               piano(e4, 6),  \
               wait(2),       \
               piano(gs4, 4), \
               wait(2),       \
               piano(b4, 2),  \
               wait(2)
#define bar_41 piano(c5, 4),  \
               piano(a2, 12), \
               wait(2),       \
               piano(e3, 10), \
               wait(2),       \
               piano(a3, 8),  \
               wait(2),       \
               piano(e4, 6),  \
               wait(2),       \
               piano(e5, 4),  \
               wait(2),       \
               piano(ds5, 2), \
               wait(2)
#define bar_44 piano(b4, 4),  \
               piano(e2, 12), \
               wait(2),       \
               piano(e3, 10), \
               wait(2),       \
               piano(gs3, 8), \
               wait(2),       \
               piano(d4, 6),  \
               wait(2),       \
               piano(c5, 4),  \
               wait(2),       \
               piano(b4, 2),  \
               wait(2)
#define bar_45 piano(a4, 4), \
               piano(a2, 2), \
               wait(2),      \
               piano(e3, 2), \
               wait(2),      \
               piano(a3, 2), \
               wait(2),      \
               piano(b4, 2), \
               wait(2),      \
               piano(c5, 2), \
               wait(2),      \
               piano(d5, 2), \
               wait(2)
#define bar_46 piano(e5, 6),  \
               piano(c3, 12), \
               wait(2),       \
               piano(g3, 10), \
               wait(2),       \
               piano(c4, 8),  \
               wait(2),       \
               piano(g4, 6),  \
               wait(2),       \
               piano(f5, 4),  \
               wait(2),       \
               piano(e5, 2),  \
               wait(2)
#define bar_47 piano(d5, 6),  \
               piano(g2, 12), \
               wait(2),       \
               piano(g3, 10), \
               wait(2),       \
               piano(b3, 8),  \
               wait(2),       \
               piano(g4, 6),  \
               wait(2),       \
               piano(f5, 4),  \
               wait(2),       \
               piano(e5, 2),  \
               wait(2)
#define bar_48 piano(c5, 6),  \
               piano(a2, 12), \
               wait(2),       \
               piano(e3, 10), \
               wait(2),       \
               piano(a3, 8),  \
               wait(2),       \
               piano(e4, 6),  \
               wait(2),       \
               piano(d5, 4),  \
               wait(2),       \
               piano(c5, 2),  \
               wait(2)
#define bar_49 piano(b4, 4), \
               piano(e2, 2), \
               wait(2),      \
               piano(e3, 2), \
               wait(2),      \
               piano(e4, 2), \
               wait(2),      \
               piano(e4, 2), \
               wait(2),      \
               piano(e5, 2), \
               wait(2),      \
               piano(e4, 2), \
               wait(2)
#define bar_50 piano(e5, 2),  \
               wait(2),       \
               piano(e5, 2),  \
               wait(2),       \
               piano(e6, 2),  \
               wait(2),       \
               piano(ds5, 2), \
               wait(2),       \
               piano(e5, 2),  \
               wait(2),       \
               piano(ds5, 2), \
               wait(2)
#define bar_51 piano(e5, 2),  \
               wait(2),       \
               piano(ds5, 2), \
               wait(2),       \
               piano(e5, 2),  \
               wait(2),       \
               piano(ds5, 2), \
               wait(2),       \
               piano(e5, 2),  \
               wait(2),       \
               piano(ds5, 2), \
               wait(2)
#define bar_52 piano(e5, 2),  \
               wait(2),       \
               piano(ds5, 2), \
               wait(2),       \
               piano(e5, 2),  \
               wait(2),       \
               piano(b4, 2),  \
               wait(2),       \
               piano(d5, 2),  \
               wait(2),       \
               piano(c5, 2),  \
               wait(2)
#define bar_53 piano(a4, 4),  \
               piano(a2, 12), \
               wait(2),       \
               piano(e3, 10), \
               wait(2),       \
               piano(a3, 8),  \
               wait(2),       \
               piano(c4, 6),  \
               wait(2),       \
               piano(e4, 4),  \
               wait(2),       \
               piano(a4, 2),  \
               wait(2)
#define bar_54 piano(b4, 4),  \
               piano(e2, 12), \
               wait(2),       \
               piano(e3, 10), \
               wait(2),       \
               piano(gs3, 8), \
               wait(2),       \
               piano(e4, 6),  \
               wait(2),       \
               piano(gs4, 4), \
               wait(2),       \
               piano(b4, 2),  \
               wait(2)
#define bar_55 piano(c5, 4),  \
               piano(a2, 12), \
               wait(2),       \
               piano(e3, 10), \
               wait(2),       \
               piano(a3, 8),  \
               wait(2),       \
               piano(e4, 6),  \
               wait(2),       \
               piano(e5, 4),  \
               wait(2),       \
               piano(ds5, 2), \
               wait(2)
#define bar_58 piano(b4, 4),  \
               piano(e2, 12), \
               wait(2),       \
               piano(e3, 10), \
               wait(2),       \
               piano(gs3, 8), \
               wait(2),       \
               piano(d4, 6),  \
               wait(2),       \
               piano(c5, 4),  \
               wait(2),       \
               piano(b4, 2),  \
               wait(2)
#define bar_59 piano(a4, 4), \
               piano(a2, 2), \
               wait(2),      \
               piano(a2, 2), \
               wait(2),      \
               piano(a2, 2), \
               wait(2),      \
               piano(a2, 2), \
               wait(2),      \
               piano(a2, 2), \
               wait(2),      \
               piano(a2, 2), \
               wait(2)
#define bar_60 piano(a4, 12),  \
               piano(e4, 12),  \
               piano(g4, 12),  \
               piano(as4, 12), \
               piano(cs5, 2),  \
               wait(2),        \
               piano(a2, 2),   \
               wait(2),        \
               piano(a2, 2),   \
               wait(2),        \
               piano(a2, 2),   \
               wait(2),        \
               piano(a2, 2),   \
               wait(2),        \
               piano(a2, 2),   \
               wait(2)
#define bar_61 piano(f4, 8),  \
               piano(a4, 8),  \
               piano(d5, 8),  \
               piano(a2, 2),  \
               wait(2),       \
               piano(a2, 2),  \
               wait(2),       \
               piano(a2, 2),  \
               wait(2),       \
               piano(a2, 2),  \
               wait(2),       \
               piano(cs5, 2), \
               piano(e5, 2),  \
               piano(a2, 2),  \
               wait(2),       \
               piano(d5, 2),  \
               piano(f5, 2),  \
               piano(a2, 2),  \
               wait(2)
#define bar_62 piano(gs4, 8), \
               piano(d5, 8),  \
               piano(f5, 8),  \
               piano(a2, 2),  \
               wait(2),       \
               piano(a2, 2),  \
               wait(2),       \
               piano(a2, 2),  \
               wait(2),       \
               piano(a2, 2),  \
               wait(2),       \
               piano(gs4, 2), \
               piano(d5, 2),  \
               piano(f5, 2),  \
               piano(a2, 2),  \
               wait(2),       \
               piano(a2, 2),  \
               wait(2)
#define bar_63 piano(a4, 12), \
               piano(c5, 12), \
               piano(e5, 12), \
               piano(a2, 2),  \
               wait(2),       \
               piano(a2, 2),  \
               wait(2),       \
               piano(a2, 2),  \
               wait(2),       \
               piano(a2, 2),  \
               wait(2),       \
               piano(a2, 2),  \
               wait(2),       \
               piano(a2, 2),  \
               wait(2)
#define bar_64 piano(f4, 8), \
               piano(d5, 8), \
               piano(a2, 2), \
               piano(d2, 2), \
               wait(2),      \
               piano(a2, 2), \
               piano(d2, 2), \
               wait(2),      \
               piano(a2, 2), \
               piano(d2, 2), \
               wait(2),      \
               piano(a2, 2), \
               piano(d2, 2), \
               wait(2),      \
               piano(e4, 2), \
               piano(c5, 2), \
               piano(a2, 2), \
               piano(d2, 2), \
               wait(2),      \
               piano(d4, 2), \
               piano(b4, 2), \
               piano(a2, 2), \
               piano(d2, 2), \
               wait(2)
#define bar_65 piano(c4, 8),  \
               piano(fs4, 8), \
               piano(a4, 8),  \
               piano(a2, 2),  \
               piano(ds2, 2), \
               wait(2),       \
               piano(a2, 2),  \
               piano(ds2, 2), \
               wait(2),       \
               piano(a2, 2),  \
               piano(ds2, 2), \
               wait(2),       \
               piano(a2, 2),  \
               piano(ds2, 2), \
               wait(2),       \
               piano(c4, 4),  \
               piano(a4, 4),  \
               piano(a2, 2),  \
               piano(ds2, 2), \
               wait(2),       \
               piano(a2, 2),  \
               piano(ds2, 2), \
               wait(2)
#define bar_66 piano(c4, 4),  \
               piano(a4, 4),  \
               piano(a2, 2),  \
               piano(e2, 2),  \
               wait(2),       \
               piano(a2, 2),  \
               piano(e2, 2),  \
               wait(2),       \
               piano(e4, 4),  \
               piano(c5, 4),  \
               piano(a2, 2),  \
               piano(e2, 2),  \
               wait(2),       \
               piano(a2, 2),  \
               piano(e2, 2),  \
               wait(2),       \
               piano(d4, 4),  \
               piano(b4, 4),  \
               piano(gs2, 2), \
               piano(e2, 2),  \
               wait(2),       \
               piano(gs2, 2), \
               piano(e2, 2),  \
               wait(2)
#define bar_67 piano(c4, 12), \
               piano(a4, 12), \
               piano(a1, 2),  \
               piano(a2, 2),  \
               wait(2),       \
               piano(a2, 2),  \
               wait(2),       \
               piano(a2, 2),  \
               wait(2),       \
               piano(a2, 2),  \
               wait(2),       \
               piano(a2, 2),  \
               wait(2),       \
               piano(a2, 2),  \
               wait(2)
#define bar_70 piano(d5, 8), \
               piano(f5, 8), \
               piano(a2, 2), \
               wait(2),      \
               piano(a2, 2), \
               wait(2),      \
               piano(a2, 2), \
               wait(2),      \
               piano(a2, 2), \
               wait(2),      \
               piano(d5, 4), \
               piano(f5, 4), \
               piano(a2, 2), \
               wait(2),      \
               piano(a2, 2), \
               wait(2)
#define bar_71 piano(d5, 12), \
               piano(f5, 12), \
               piano(as2, 2), \
               wait(2),       \
               piano(as2, 2), \
               wait(2),       \
               piano(as2, 2), \
               wait(2),       \
               piano(as2, 2), \
               wait(2),       \
               piano(as2, 2), \
               wait(2),       \
               piano(as2, 2), \
               wait(2)
#define bar_72 piano(g4, 8),  \
               piano(ds5, 8), \
               piano(as2, 2), \
               wait(2),       \
               piano(as2, 2), \
               wait(2),       \
               piano(as2, 2), \
               wait(2),       \
               piano(as2, 2), \
               wait(2),       \
               piano(f4, 2),  \
               piano(d5, 2),  \
               piano(as2, 2), \
               wait(2),       \
               piano(ds4, 2), \
               piano(c5, 2),  \
               piano(as2, 2), \
               wait(2)
#define bar_73 piano(d4, 8),  \
               piano(f4, 8),  \
               piano(as4, 8), \
               piano(as2, 2), \
               wait(2),       \
               piano(as2, 2), \
               wait(2),       \
               piano(as2, 2), \
               wait(2),       \
               piano(as2, 2), \
               wait(2),       \
               piano(d4, 4),  \
               piano(f4, 4),  \
               piano(a4, 4),  \
               piano(as2, 2), \
               wait(2),       \
               piano(as2, 2), \
               wait(2)
#define bar_74 piano(d4, 8),  \
               piano(f4, 8),  \
               piano(gs4, 8), \
               piano(as2, 2), \
               wait(2),       \
               piano(as2, 2), \
               wait(2),       \
               piano(as2, 2), \
               wait(2),       \
               piano(as2, 2), \
               wait(2),       \
               piano(d4, 4),  \
               piano(f4, 4),  \
               piano(gs4, 4), \
               piano(as2, 2), \
               wait(2),       \
               piano(as2, 2), \
               wait(2)
#define bar_75 piano(c3, 8), \
               piano(c4, 8), \
               piano(e4, 8), \
               piano(a4, 8), \
               wait(12)
#define bar_76 piano(e3, 4),  \
               piano(gs3, 4), \
               piano(e4, 4),  \
               piano(b4, 4),  \
               wait(12)
#define bar_77 piano(a1, 4), \
               piano(a3, 6), \
               wait(1),      \
               piano(c4, 6), \
               wait(1),      \
               piano(e4, 6), \
               wait(2),      \
               piano(a4, 6), \
               wait(1),      \
               piano(c5, 6), \
               wait(1),      \
               piano(e5, 6), \
               wait(2),      \
               piano(a3, 4), \
               piano(c4, 4), \
               piano(e4, 4), \
               piano(d5, 6), \
               wait(1),      \
               piano(c5, 6), \
               wait(1),      \
               piano(b4, 6), \
               wait(2)
#define bar_78 piano(a3, 4), \
               piano(c4, 4), \
               piano(e4, 4), \
               piano(a4, 6), \
               wait(1),      \
               piano(c5, 6), \
               wait(1),      \
               piano(e5, 6), \
               wait(2),      \
               piano(a5, 6), \
               wait(1),      \
               piano(c6, 6), \
               wait(1),      \
               piano(e6, 6), \
               wait(2),      \
               piano(a3, 4), \
               piano(c4, 4), \
               piano(e4, 4), \
               piano(d6, 6), \
               wait(1),      \
               piano(c6, 6), \
               wait(1),      \
               piano(b5, 6), \
               wait(2)
#define bar_80 piano(a3, 4),  \
               piano(c4, 4),  \
               piano(e4, 4),  \
               piano(as5, 6), \
               wait(1),       \
               piano(a5, 6),  \
               wait(1),       \
               piano(gs5, 6), \
               wait(2),       \
               piano(g5, 6),  \
               wait(1),       \
               piano(fs6, 6), \
               wait(1),       \
               piano(f6, 6),  \
               wait(2),       \
               piano(e6, 6),  \
               wait(1),       \
               piano(ds6, 6), \
               wait(1),       \
               piano(d6, 6),  \
               wait(2)
#define bar_81 piano(cs6, 6), \
               wait(1),       \
               piano(c6, 6),  \
               wait(1),       \
               piano(b5, 6),  \
               wait(2),       \
               piano(as5, 6), \
               wait(1),       \
               piano(a5, 6),  \
               wait(1),       \
               piano(gs5, 6), \
               wait(2),       \
               piano(g5, 4),  \
               wait(1),       \
               piano(fs5, 3), \
               wait(1),       \
               piano(f5, 2),  \
               wait(2)
#define bar_82 piano(e5, 2),  \
               wait(2),       \
               piano(ds5, 2), \
               wait(2),       \
               piano(e5, 2),  \
               wait(2),       \
               piano(b4, 2),  \
               wait(2),       \
               piano(d5, 2),  \
               wait(2),       \
               piano(c5, 2),  \
               wait(2)
#define bar_83 piano(a4, 4),  \
               piano(a2, 12), \
               wait(2),       \
               piano(e3, 10), \
               wait(2),       \
               piano(a3, 8),  \
               wait(2),       \
               piano(c4, 6),  \
               wait(2),       \
               piano(e4, 4),  \
               wait(2),       \
               piano(a4, 2),  \
               wait(2)
#define bar_84 piano(b4, 4),  \
               piano(e2, 12), \
               wait(2),       \
               piano(e3, 10), \
               wait(2),       \
               piano(gs3, 8), \
               wait(2),       \
               piano(e4, 6),  \
               wait(2),       \
               piano(gs4, 4), \
               wait(2),       \
               piano(b4, 2),  \
               wait(2)
#define bar_85 piano(c5, 4),  \
               piano(a2, 12), \
               wait(2),       \
               piano(e3, 10), \
               wait(2),       \
               piano(a3, 8),  \
               wait(2),       \
               piano(e4, 6),  \
               wait(2),       \
               piano(e5, 4),  \
               wait(2),       \
               piano(ds5, 2), \
               wait(2)
#define bar_88 piano(b4, 4),  \
               piano(e2, 12), \
               wait(2),       \
               piano(e3, 10), \
               wait(2),       \
               piano(gs3, 8), \
               wait(2),       \
               piano(d4, 6),  \
               wait(2),       \
               piano(c5, 4),  \
               wait(2),       \
               piano(b4, 2),  \
               wait(2)
#define bar_89 piano(a4, 4), \
               piano(a2, 2), \
               wait(2),      \
               piano(e3, 2), \
               wait(2),      \
               piano(a3, 2), \
               wait(2),      \
               piano(b4, 2), \
               wait(2),      \
               piano(c5, 2), \
               wait(2),      \
               piano(d5, 2), \
               wait(2)
#define bar_90 piano(e5, 6),  \
               piano(c3, 12), \
               wait(2),       \
               piano(g3, 10), \
               wait(2),       \
               piano(c4, 8),  \
               wait(2),       \
               piano(g4, 6),  \
               wait(2),       \
               piano(f5, 4),  \
               wait(2),       \
               piano(e5, 2),  \
               wait(2)
#define bar_91 piano(d5, 6),  \
               piano(g2, 12), \
               wait(2),       \
               piano(g3, 10), \
               wait(2),       \
               piano(b3, 8),  \
               wait(2),       \
               piano(f4, 6),  \
               wait(2),       \
               piano(e5, 4),  \
               wait(2),       \
               piano(d5, 2),  \
               wait(2)
#define bar_92 piano(c5, 6), \
               piano(a2, 6), \
               wait(2),      \
               piano(e3, 6), \
               wait(2),      \
               piano(a3, 6), \
               wait(2),      \
               piano(e4, 6), \
               wait(2),      \
               piano(d5, 6), \
               wait(2),      \
               piano(c5, 6), \
               wait(2)
#define bar_93 piano(b4, 4), \
               piano(e2, 6), \
               wait(2),      \
               piano(e3, 6), \
               wait(2),      \
               piano(e4, 6), \
               wait(2),      \
               piano(e4, 6), \
               wait(2),      \
               piano(e5, 6), \
               wait(2),      \
               piano(e4, 6), \
               wait(2)
#define bar_94 piano(e5, 6),  \
               wait(2),       \
               piano(e5, 6),  \
               wait(2),       \
               piano(e6, 6),  \
               wait(2),       \
               piano(ds5, 6), \
               wait(2),       \
               piano(e5, 4),  \
               wait(2),       \
               piano(ds5, 2), \
               wait(2)
#define bar_95 piano(e5, 4),  \
               wait(2),       \
               piano(ds5, 2), \
               wait(2),       \
               piano(e5, 2),  \
               wait(2),       \
               piano(ds5, 2), \
               wait(2),       \
               piano(e5, 2),  \
               wait(2),       \
               piano(ds5, 2), \
               wait(2)
#define bar_97 piano(a4, 4),  \
               piano(a2, 12), \
               wait(2),       \
               piano(e3, 10), \
               wait(2),       \
               piano(a3, 8),  \
               wait(2),       \
               piano(c4, 6),  \
               wait(2),       \
               piano(e4, 4),  \
               wait(2),       \
               piano(a4, 2),  \
               wait(2)
#define bar_98 piano(b4, 4),  \
               piano(e2, 12), \
               wait(2),       \
               piano(e3, 10), \
               wait(2),       \
               piano(gs3, 8), \
               wait(2),       \
               piano(e4, 6),  \
               wait(2),       \
               piano(gs4, 4), \
               wait(2),       \
               piano(b4, 2),  \
               wait(2)
#define bar_99 piano(c5, 4),  \
               piano(a2, 12), \
               wait(2),       \
               piano(e3, 10), \
               wait(2),       \
               piano(a3, 8),  \
               wait(2),       \
               piano(e4, 6),  \
               wait(2),       \
               piano(e5, 4),  \
               wait(2),       \
               piano(ds5, 2), \
               wait(2)
#define bar_101 piano(a4, 4),  \
                piano(a2, 12), \
                wait(2),       \
                piano(e3, 10), \
                wait(2),       \
                piano(a3, 8),  \
                wait(2),       \
                piano(c4, 6),  \
                wait(2),       \
                piano(e4, 4),  \
                wait(2),       \
                piano(a4, 2),  \
                wait(2)
#define bar_102 piano(b4, 4),  \
                piano(e2, 12), \
                wait(2),       \
                piano(e3, 10), \
                wait(2),       \
                piano(gs3, 8), \
                wait(2),       \
                piano(d4, 6),  \
                wait(2),       \
                piano(c5, 4),  \
                wait(2),       \
                piano(b4, 2),  \
                wait(2)
#define bar_103 piano(a1, 4), \
                piano(a2, 4), \
                piano(a4, 4), \
                wait(12)

static const uint8_t song[] = {
    wait(6),

    bar_0,
    bar_1,
    bar_2,
    bar_3,
    bar_4,
    bar_5,
    bar_6,
    bar_7,
    bar_8_1,

    bar_0,
    bar_1,
    bar_2,
    bar_3,
    bar_4,
    bar_5,
    bar_6,
    bar_7,
    bar_8_2,

    bar_9,
    bar_10,
    bar_11,
    bar_12,
    bar_13,
    bar_14,
    bar_15,
    bar_16,
    bar_17,
    bar_18,
    bar_15,
    bar_16,
    bar_21,
    bar_22_1,

    bar_9,
    bar_10,
    bar_11,
    bar_12,
    bar_13,
    bar_14,
    bar_15,
    bar_16,
    bar_17,
    bar_18,
    bar_15,
    bar_16,
    bar_21,
    bar_22_2,

    bar_23,
    bar_24,
    bar_25,
    bar_26,
    bar_27,
    bar_28,
    bar_29,

    bar_30,
    bar_31,
    bar_32,
    bar_33,
    bar_34,
    bar_35,
    bar_36,
    bar_37,
    bar_38,
    bar_39,

    bar_40,
    bar_41,
    bar_38,
    bar_39,
    bar_44,
    bar_45,
    bar_46,
    bar_47,
    bar_48,
    bar_49,

    bar_50,
    bar_51,
    bar_52,
    bar_53,
    bar_54,
    bar_55,
    bar_52,
    bar_53,
    bar_58,
    bar_59,

    bar_60,
    bar_61,
    bar_62,
    bar_63,
    bar_64,
    bar_65,
    bar_66,
    bar_67,
    bar_60,
    bar_61,

    bar_70,
    bar_71,
    bar_72,
    bar_73,
    bar_74,
    bar_75,
    bar_76,
    bar_77,
    bar_78,
    bar_78,

    bar_80,
    bar_81,
    bar_82,
    bar_83,
    bar_84,
    bar_85,
    bar_82,
    bar_83,
    bar_88,
    bar_89,

    bar_90,
    bar_91,
    bar_92,
    bar_93,
    bar_94,
    bar_95,
    bar_82,
    bar_97,
    bar_98,
    bar_99,

    bar_82,
    bar_101,
    bar_102,
    bar_103,

    end()};

void ezpsg_instruments(const uint8_t **data)
{
    uint8_t note, vol_decay;
    switch ((int8_t)*(*data)++) // instrument
    {
    case -1: // piano
        // higher notes decay faster
        note = *(*data)++;
        vol_decay = 0xF9;
        if (note < c3)
            vol_decay = 0xFA;
        if (note > c6)
            vol_decay = 0xF8;
        ezpsg_play_note(note,       // note
                        *(*data)++, // duration
                        1,          // release
                        200,        // duty
                        0x11,       // vol_attack
                        vol_decay,  // vol_decay
                        0x34,       // wave_release
                        0);         // pan
        break;
#ifndef NDEBUG
    default:
        // The instrument you just added probably isn't
        // consuming the correct number of parameters.
        puts("Unknown instrument.");
        exit(1);
#endif
    }
}

void main(void)
{
    uint8_t v = RIA.vsync;

    int cp = code_page(0);
    char u = 'u';
    if (cp == 437 || cp == 850)
        u = 0x81; //  u diaeresis
    putchar('F');
    putchar(u);
    puts("r Elise");
    puts("by Ludwig van Beethoven");

    ezpsg_init(0xFF00);
    ezpsg_play_song(song);

    while (true)
    {
        if (RIA.vsync == v)
            continue;
        v = RIA.vsync;
        ezpsg_tick(5);
        if (!ezpsg_playing())
            break;
    }
}
