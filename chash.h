
#define HASH_SIZE 1000



typedef struct mini_node
{
    char *key;
    void *value;
    unsigned int hash;
} mini_node_t;


typedef struct
{
    char *key;
    void *value;
    unsigned int hash;  //help to compare the key
    mini_node_t *next;  // point to mini node struct array
    unsigned char dyn_array_used; // in used element number
    unsigned char dyn_array_size;  //help to record the deep of collision chain
} node_t;


typedef struct string_hash
{
    node_t *buckets;
    unsigned int capacity;
    unsigned int used_count;
    unsigned int collision_count;
    double load_factor;
    size_t hash_value_size; // the global value malloc size
    pthread_mutex_t mutex;  // maybe rw_lock
    signed char rehash_flag;  // whether is rehashing
    struct string_hash *hash2;  // used for rehash
} hash_t;



hash_t *chash_new(unsigned int capacity);

double chash_get_load_factor(hash_t *hash);

int chash_add(hash_t *hash, char *key, void *value);

void *chash_get(hash_t *hash, char *key);

int chash_delete(hash_t *hash, char *key);

void chash_destory(hash_t *hash);


int chash_rehash(hash_t *hash);
 

