#define HASH_VAR 193508814
#define HASH_FUNCTION 7572387384277067
#define HASH_IF 5863476
#define HASH_ELIF 6385191717
#define HASH_ELSE 6385192046
#define HASH_WHILE 210732529790
#define HASH_END 193490716
#define HASH_RETURN 6953974653989
#define HASH_TYPE_INT 193495088
#define HASH_TYPE_REAL 6385651017
#define HASH_TYPE_STR 193506174

unsigned long hash(unsigned char *str)
{
    unsigned long hash = 5381;
    int c;

    while (c = *str++)
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash;
}