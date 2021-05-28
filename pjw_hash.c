#include <stdio.h>


// code from wiki
unsigned long ElfHash(const unsigned char *s)
{
    unsigned long h = 0, high;
    while(*s)
    {
        h = (h << 4) + *s++;
        if((high = h & 0xF0000000))
            h ^= high >> 24;
        h &= ~high;
    }
    return h;
}


int main(void)
{
    unsigned char test[] = "abcd";
    printf("hash %lu\n", ElfHash(test));

    return 0;
}
