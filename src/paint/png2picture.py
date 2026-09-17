#!/usr/bin/env python3
#
# Copyright (c) 2026 Rumbledethumps
#
# SPDX-License-Identifier: BSD-3-Clause
# SPDX-License-Identifier: Unlicense

"""Convert the logo PNG into the paint picture.

The picture is 320x240 pixels at 4 bits per pixel in the 16 built-in ANSI
colors. The logo's ring is made grey with black outside it, then the image is
scaled to fit, centered, and dithered to those colors.

usage: png2picture.py <in.png> <out.bin>
"""

import struct
import sys
import zlib

WIDTH = 320
HEIGHT = 240

# The first 16 colors of the built-in palette.
PALETTE = [
    (0, 0, 0), (205, 0, 0), (0, 205, 0), (205, 205, 0),
    (0, 0, 238), (205, 0, 205), (0, 205, 205), (229, 229, 229),
    (127, 127, 127), (255, 0, 0), (0, 255, 0), (255, 255, 0),
    (92, 92, 255), (255, 0, 255), (0, 255, 255), (255, 255, 255),
]


def read_png(path):
    """Return the width, the height, and rows of (r, g, b) pixels, with any
    transparency blended onto black."""
    with open(path, "rb") as f:
        data = f.read()
    if data[:8] != b"\x89PNG\r\n\x1a\n":
        sys.exit(f"{path}: not a PNG file")

    pos = 8
    idat = []
    plte = trns = b""
    while pos < len(data):
        length, kind = struct.unpack(">I4s", data[pos:pos + 8])
        body = data[pos + 8:pos + 8 + length]
        pos += 12 + length
        if kind == b"IHDR":
            width, height, depth, color_type, _, _, interlace = struct.unpack(
                ">IIBBBBB", body)
        elif kind == b"PLTE":
            plte = body
        elif kind == b"tRNS":
            trns = body
        elif kind == b"IDAT":
            idat.append(body)

    if depth != 8 or interlace:
        sys.exit(f"{path}: only 8-bit, non-interlaced PNG files are supported")

    # Bytes per pixel for gray, RGB, palette, gray+alpha and RGBA.
    bpp = {0: 1, 2: 3, 3: 1, 4: 2, 6: 4}[color_type]
    lines = unfilter(zlib.decompress(b"".join(idat)), width, height, bpp)

    rows = []
    for line in lines:
        row = []
        for i in range(0, width * bpp, bpp):
            if color_type == 0:
                r = g = b = line[i]
                a = 255
            elif color_type == 2:
                r, g, b = line[i:i + 3]
                a = 255
            elif color_type == 3:
                index = line[i]
                r, g, b = plte[index * 3:index * 3 + 3]
                a = trns[index] if index < len(trns) else 255
            elif color_type == 4:
                r = g = b = line[i]
                a = line[i + 1]
            else:
                r, g, b, a = line[i:i + 4]
            row.append((r * a / 255, g * a / 255, b * a / 255))
        rows.append(row)
    return width, height, rows


def unfilter(raw, width, height, bpp):
    """Undo the filter that starts each scanline. Each filter predicts a byte
    from its neighbors to the left, above, and above-left."""
    stride = width * bpp
    lines = []
    above = bytearray(stride)
    pos = 0
    for _ in range(height):
        kind = raw[pos]
        line = bytearray(raw[pos + 1:pos + 1 + stride])
        pos += 1 + stride
        for i in range(stride):
            left = line[i - bpp] if i >= bpp else 0
            up = above[i]
            up_left = above[i - bpp] if i >= bpp else 0
            if kind == 0:
                predict = 0
            elif kind == 1:
                predict = left
            elif kind == 2:
                predict = up
            elif kind == 3:
                predict = (left + up) // 2
            else:
                p = left + up - up_left
                pa, pb, pc = abs(p - left), abs(p - up), abs(p - up_left)
                if pa <= pb and pa <= pc:
                    predict = left
                elif pb <= pc:
                    predict = up
                else:
                    predict = up_left
            line[i] = (line[i] + predict) & 0xFF
        lines.append(line)
        above = line
    return lines


# The logo is a dark ring on white, with the cow inside it.
RING = (36, 47, 52)
GREY = PALETTE[8]
BLACK = (0, 0, 0)


def is_ring(color):
    """The ring is the only dark gray in the logo. The fur and the nose are
    brown, so they are never taken for it."""
    r, g, b = color
    return max(color) - min(color) < 30 and 0.299 * r + 0.587 * g + 0.114 * b < 150


def outside_of(ring):
    """Mark everything that can be reached from the edge of the image without
    crossing the ring."""
    height, width = len(ring), len(ring[0])
    outside = [[False] * width for _ in range(height)]
    todo = [(x, y) for x in range(width) for y in (0, height - 1)]
    todo += [(x, y) for y in range(height) for x in (0, width - 1)]
    while todo:
        x, y = todo.pop()
        if outside[y][x] or ring[y][x]:
            continue
        outside[y][x] = True
        for nx, ny in ((x - 1, y), (x + 1, y), (x, y - 1), (x, y + 1)):
            if 0 <= nx < width and 0 <= ny < height:
                todo.append((nx, ny))
    return outside


def grow(mask, n):
    """Widen every marked area by n pixels in each direction."""
    wide = [[any(row[max(0, x - n):x + n + 1]) for x in range(len(row))]
            for row in mask]
    columns = [list(column) for column in zip(*wide)]
    tall = [[any(column[max(0, y - n):y + n + 1]) for y in range(len(column))]
            for column in columns]
    return [list(row) for row in zip(*tall)]


def restyle_edge(rows, edge, outside, x, y):
    """Recolor a pixel on the edge of the ring. Antialiasing mixed the ring's
    color with what is beside it, so the pixel keeps that mix with grey in
    place of the ring's color, and with black in place of what is outside."""
    color = rows[y][x]
    beside = [(rows[ny][nx], outside[ny][nx])
              for ny in range(max(0, y - 4), min(len(rows), y + 5))
              for nx in range(max(0, x - 4), min(len(rows[0]), x + 5))
              if not edge[ny][nx]]
    if not beside:
        return GREY
    other = [sum(c[i] for c, _ in beside) / len(beside) for i in range(3)]
    to_ring = [other[i] - RING[i] for i in range(3)]
    share = sum((other[i] - color[i]) * to_ring[i] for i in range(3))
    share = min(1.0, max(0.0, share / sum(v * v for v in to_ring)))
    if beside[0][1]:
        return tuple(GREY[i] * share for i in range(3))
    return tuple(min(255.0, color[i] + (GREY[i] - RING[i]) * share)
                 for i in range(3))


def restyle(width, height, rows):
    """Make the ring grey and everything outside it black, then move the
    picture down so the ring's top and bottom margins swap."""
    ring = [[is_ring(color) for color in row] for row in rows]
    outside = outside_of(ring)
    edge = grow(ring, 2)

    styled = []
    for y in range(height):
        row = []
        for x in range(width):
            if edge[y][x]:
                row.append(restyle_edge(rows, edge, outside, x, y))
            elif outside[y][x]:
                row.append(BLACK)
            else:
                row.append(rows[y][x])
        styled.append(row)

    ring_rows = [y for y in range(height) if any(ring[y])]
    shift = (height - 1 - ring_rows[-1]) - ring_rows[0]
    blank = [BLACK] * width
    moved = [blank] * shift + styled + [blank] * -shift
    return moved[max(0, -shift):][:height]


def fit(width, height, rows):
    """Scale to fit the picture and center on black. Each picture pixel is the
    average of the image pixels it covers."""
    scale = min(WIDTH / width, HEIGHT / height)
    fit_w = round(width * scale)
    fit_h = round(height * scale)
    left = (WIDTH - fit_w) // 2
    top = (HEIGHT - fit_h) // 2

    picture = [[[0.0, 0.0, 0.0] for _ in range(WIDTH)] for _ in range(HEIGHT)]
    for y in range(fit_h):
        y0 = y * height // fit_h
        y1 = max(y0 + 1, (y + 1) * height // fit_h)
        for x in range(fit_w):
            x0 = x * width // fit_w
            x1 = max(x0 + 1, (x + 1) * width // fit_w)
            total = [0.0, 0.0, 0.0]
            for sy in range(y0, y1):
                for sx in range(x0, x1):
                    for c in range(3):
                        total[c] += rows[sy][sx][c]
            count = (y1 - y0) * (x1 - x0)
            picture[top + y][left + x] = [t / count for t in total]
    return picture


def distance(color, r, g, b):
    """The "redmean" color distance, which weighs red, green and blue closer
    to how the eye does than plain RGB distance."""
    mean = (color[0] + r) / 2
    dr, dg, db = color[0] - r, color[1] - g, color[2] - b
    return ((2 + mean / 256) * dr * dr + 4 * dg * dg +
            (2 + (255 - mean) / 256) * db * db)


def nearest(r, g, b):
    return min(range(len(PALETTE)), key=lambda i: distance(PALETTE[i], r, g, b))


def dither(picture):
    """Floyd-Steinberg dithering. Each pixel takes the nearest palette color,
    and the difference is shared with the neighbors not yet converted.

    Only three quarters of the difference is shared. With so few colors,
    sharing all of it scatters colored specks through areas that are almost a
    palette color, such as the white background."""
    indices = []
    for y in range(HEIGHT):
        row = []
        for x in range(WIDTH):
            color = picture[y][x]
            index = nearest(*color)
            row.append(index)
            error = [color[c] - PALETTE[index][c] for c in range(3)]
            for dx, dy, share in ((1, 0, 7), (-1, 1, 3), (0, 1, 5), (1, 1, 1)):
                nx, ny = x + dx, y + dy
                if 0 <= nx < WIDTH and ny < HEIGHT:
                    for c in range(3):
                        picture[ny][nx][c] += error[c] * share / 16 * 3 / 4
        indices.append(row)
    return indices


def pack(indices):
    """Two pixels a byte, the left one in the high nibble."""
    out = bytearray()
    for row in indices:
        for x in range(0, WIDTH, 2):
            out.append(row[x] << 4 | row[x + 1])
    return out


def main():
    if len(sys.argv) != 3:
        sys.exit(__doc__)
    width, height, rows = read_png(sys.argv[1])
    rows = restyle(width, height, rows)
    picture = fit(width, height, rows)
    with open(sys.argv[2], "wb") as f:
        f.write(pack(dither(picture)))


if __name__ == "__main__":
    main()
