/* original obtained from Pat Estep
   does prefetch across hypernodes */
static void internal_pre_bcopy();
void read_prefetch_region();
extern int MPID_SHMEM_CNX_SAME_NODE;

void pre_bcopy(s1, s2, n)
    unsigned int s1;
    unsigned int s2;
    int n;
{
    int offset;

    if(MPID_SHMEM_CNX_SAME_NODE){
        bcopy((void *)s1, (void *)s2, n);
        return;
    }
    /* make life easy.  only work on chunks of at least 4 lines */
    if (n < 320) {
        read_prefetch_region(s1 & ~0x3f, n + (s1 & 0x3f));
        bcopy((void *)s1, (void *)s2, n);
        return;
    }
    /* force starting alignment */
    offset = (unsigned int)s1 & 0x3f;
    if (offset) {
        offset = 64 - offset;
        bcopy((void *)s1, (void *)s2, offset);
        s1 += offset;
        s2 += offset;
        n -= offset;
    }

    /* deal with case where n is not a multiple of 64 */
    offset = n & 0x3f;
    if (offset) {
        n &= ~0x3f;
        internal_pre_bcopy(s1, s2, n);
        s1 += n;
        s2 += n;
        bcopy((void *)s1, (void *)s2, offset);
        return;
    }

    /* n is a multiple of 64 */
    internal_pre_bcopy(s1, s2, n);
    return;
}

static void
internal_pre_bcopy(s1, s2, n)
    unsigned int s1;
    unsigned int s2;
    int n;
{
    read_prefetch_region(s1, 192);                /* prefetch 0, 1 & 2 */
    while (1) {
        read_prefetch_region(s1+192, 64);        /* prefetch 3 */
        if (n == 256)
            break;
        bcopy((void *)s1, (void *)s2, 64);                        /* copy 0 */
        s1 += 64; s2 += 64; n -= 64;
        read_prefetch_region(s1+192, 64);        /* prefetch 0 */
        if (n == 256)
            break;
        bcopy((void *)s1, (void *)s2, 64);                        /* copy 1 */
        s1 += 64; s2 += 64; n -= 64;
        read_prefetch_region(s1+192, 64);        /* prefetch 1 */
        if (n == 256)
            break;
        bcopy((void *)s1, (void *)s2, 64);                        /* copy 2 */
        s1 += 64; s2 += 64; n -= 64;
        read_prefetch_region(s1+192, 64);        /* prefetch 2 */
        if (n == 256)
            break;
        bcopy((void *)s1, (void *)s2, 64);                        /* copy 3 */
        s1 += 64; s2 += 64; n -= 64;
    }
    bcopy((void *)s1, (void *)s2, 256); /* last 4 lines */
}
