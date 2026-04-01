/*
 * Copyright (c) 2025 Rumbledethumps
 *
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-License-Identifier: Unlicense
 */

#include <rp6502.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>

// VERY simple benchmark for mass storage.
// Final score discards fastest and slowest 1/sec bucket.

#define BENCH_FILE "BENCH.OK_TO_DEL"
#define NUM_PASSES 9
#define CHUNK_SIZE 1024

static long score(long *sec, int count)
{
    long vmin, vmax, sum;
    int i;
    if (count < 3)
        return 0;
    vmin = vmax = sum = sec[0];
    for (i = 1; i < count; i++)
    {
        sum += sec[i];
        if (sec[i] < vmin) vmin = sec[i];
        if (sec[i] > vmax) vmax = sec[i];
    }
    return (sum - vmin - vmax) / (1024L * (count - 2));
}

static int run_pass(int fd, char label, int writing, long *sec)
{
    long bucket_bytes = 0;
    clock_t start;
    clock_t now;
    int bucket = 0;
    int n;
    start = clock();
    while (writing ? bucket < NUM_PASSES : 1)
    {
        n = writing ? write_xram(0, CHUNK_SIZE, fd) : read_xram(0, CHUNK_SIZE, fd);
        if (n <= 0)
            break;
        bucket_bytes += n;
        now = clock();
        if (now - start >= (clock_t)(bucket + 1) * CLOCKS_PER_SEC)
        {
            sec[bucket] = bucket_bytes;
            printf("%c%d:%5ld KB/s\n", label, bucket + 1, bucket_bytes / 1024);
            bucket_bytes = 0;
            bucket++;
        }
    }
    return bucket;
}

void main()
{
    int fd;
    int i;
    long write_sec[NUM_PASSES] = {0};
    long read_sec[NUM_PASSES] = {0};
    int write_count, read_count;
    long ws, rs;

    printf("MSC BENCHMARK\n-------------\n");

    // Prepare random data
    _randomize();
    RIA.addr0 = 0;
    RIA.step0 = 1;
    for (i=0;i<CHUNK_SIZE;i++)
        RIA.rw0 = rand();

    unlink(BENCH_FILE);
    fd = open(BENCH_FILE, O_WRONLY | O_CREAT | O_TRUNC);
    if (fd < 0)
    {
        printf("Open write failed: %d\n", errno);
        return;
    }
    write_count = run_pass(fd, 'W', 1, write_sec);
    close(fd);

    fd = open(BENCH_FILE, O_RDONLY);
    if (fd < 0)
    {
        printf("Open read failed: %d\n", errno);
        unlink(BENCH_FILE);
        return;
    }
    read_count = run_pass(fd, 'R', 0, read_sec);
    close(fd);

    printf("\n   ");
    for (i = 0; i < NUM_PASSES; i++)
        printf("    %ds", i + 1);
    printf("\n");
    printf("Wr:");
    for (i = 0; i < NUM_PASSES; i++)
        printf(" %5ld", write_sec[i] / 1024);
    printf(" KB/s\n");
    printf("Rd:");
    for (i = 0; i < NUM_PASSES; i++)
        printf(" %5ld", read_sec[i] / 1024);
    printf(" KB/s\n");

    ws = score(write_sec, write_count);
    rs = score(read_sec, read_count);
    printf(ws > 0 ? "\nWrite: %ld KB/s\n" : "\nWrite: N/A\n", ws);
    printf(rs > 0 ? "Read:  %ld KB/s\n" : "Read:  N/A\n", rs);

    unlink(BENCH_FILE);
}
