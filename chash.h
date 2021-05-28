
#define HASH_SIZE 1000

// 灵感来自nanshan图书馆书架

typedef struct
{
    node_t **buckets;
    unsigned int node_count;
    unsigned int collision_count;
    pthread_mutex_t mutex;
} bookshelf_t;


// node like one book
typedef struct
{
    const char *key;
    void *value;
    size_t value_len;
    node_t *next;  // maybe there is some type chain
} node_t;
