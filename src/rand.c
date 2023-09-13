#include <rp6502.h>
#include <stdio.h>
#include <stdlib.h>

void main()
{
    printf("Random number from CC65. Always 13492: %u\n", rand());
    _randomize(); // non-standard seeder added by CC65 gets seed from RIA
    printf("Random number from CC65 after _randomize(): %u\n", rand());
    printf("Random 32 from RIA: %lu\n", lrand());
}
