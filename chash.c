#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "chash.h"


#define RET_OK 0
#define RET_FAIL -1


#define CHASH_FREE(ptr)  do{ \
    if(ptr != NULL)   \
    {                 \
        free(ptr);    \
        ptr = NULL;   \
    }                 \
}while(0)



//djb2
unsigned long
hash(unsigned char *str)
{
    unsigned long hash = 5381;
    int c;

    while (c = *str++)
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash;
}




unsigned int
JSHash(char *str)
{
    unsigned int hash = 1315423911;

    while (*str)
    {
        hash ^= ((hash << 5) + (*str++) + (hash >> 2));
    }

    return (hash & 0x7FFFFFFF);
}


// handle bucket key is null case
int chash_strcmp(const char *s1, const char *s2)
{
    if(s1 == NULL || s2 == NULL)
    {
        printf("some key is null\n");
        return -2;  // set to a not zero number
    }
    else
    {
        return strcmp(s1, s2);
    }
}


hash_t *chash_new(unsigned int capacity)
{

    if(capacity <= 0)
        return NULL;


    hash_t *hashmap = NULL;
    hashmap = malloc(sizeof(hash_t));
    if(hashmap == NULL)
    {
        perror("malloc fail");
        return NULL;
    }

    memset(hashmap, 0, sizeof(hash_t));

    hashmap->buckets = calloc(capacity, sizeof(node_t));
    if(hashmap->buckets == NULL)
    {
        perror("calloc fail");
        free(hashmap);
        return NULL;   
    }

    hashmap->capacity = capacity;

    return hashmap;
}



int chash_add(hash_t *hashmap, char *key, void *value)
{
    if(hashmap == NULL || key == NULL)
    {
        printf("parameter error at %s\n", __FUNCTION__);
        return RET_FAIL;
    }
        

    node_t *node = NULL;
    mini_node_t *mini_node_array = NULL;
    int last_index = 0; // last valid index of mini node array
    unsigned int hash_val = JSHash(key);
    unsigned int index = hash_val % (hashmap->capacity);
    node = &(hashmap->buckets[index]);

    
    if(node->key == NULL)
    { 
        node->key = key;
        node->value = value;
        node->hash = hash_val;  // ASSIGN HASH VALUE
        //KEEP next field unchange
    }
    else
    {

        if(node->hash == hash_val && strcmp(node->key, key) == 0)
        {
            //printf("update value\n");
            CHASH_FREE(node->value);  
            node->value = value;
            return RET_OK;
        }
            
        //check key within the mini node array
        int i = 0;
        for(i = 0; i < node->dyn_array_used; i++)
        {

            if(node->next[i].hash == hash_val && strcmp(node->next[i].key, key) == 0)
            {
                //printf("update value\n");
                CHASH_FREE(node->next[i].value);
                node->next[i].value = value;
                return RET_OK;
            }
        }

        // no memory space remain, have to allocate node memory
        if(node->dyn_array_used == node->dyn_array_size)
        {

            if(node->next == NULL)
            {
                // alloc the first mini node
                mini_node_array = calloc(1, sizeof(mini_node_t));
                if(mini_node_array == NULL)
                {
                    perror("calloc fail");
                    return RET_FAIL;
                }
                
                
                // link mini node to the bucket
                node->next = mini_node_array;
            }
            else
            {
                mini_node_array = realloc(node->next, (node->dyn_array_size + 1)*sizeof(mini_node_t));
                if(mini_node_array == NULL)
                {
                    perror("realloc fail\n");
                    return RET_FAIL;   
                }

            }

            node->dyn_array_size++;   
        }
        

        // did not find it then store key-value to the last index of array
        last_index = node->dyn_array_used;

        mini_node_t *last_node = mini_node_array + last_index;

        last_node->key = key;
        last_node->value = value;
        last_node->hash = hash_val;

        // add kv successful, counter increased
        node->dyn_array_used++;
    }


    return RET_OK;
}



void *chash_get(hash_t *hashmap, char *key)
{

    if(hashmap == NULL || key == NULL)
    {
        printf("parameter error\n");
        return NULL;
    }


    node_t *node = NULL;
    unsigned int hash_val = JSHash(key);
    unsigned int index = hash_val % (hashmap->capacity);
    node = &(hashmap->buckets[index]);
    
    
    if(chash_strcmp(key, node->key) == 0)
    {
        return node->value;
    }
    
    //find in mini node array
    int i = 0;
    for(i = 0; i < node->dyn_array_used; i++)
    {
        if(strcmp(key, node->next[i].key) == 0)
        {
            //printf("find key in mini node array\n");
            return node->next[i].value;
        }
        
    }
 
    return NULL;
}



// the key and value must on heap or will free crash
int chash_delete(hash_t *hashmap, char *key)
{

    if(hashmap == NULL || key == NULL)
    {
        printf("parameter error\n");
        return RET_FAIL;
    }


    node_t *node = NULL;
    int last_index = 0;
    unsigned int hash_val = JSHash(key);
    unsigned int index = hash_val % (hashmap->capacity);
    node = &(hashmap->buckets[index]);
    
        
    if(chash_strcmp(key, node->key) == 0)
    {
        // FREE key value memory on heap
        free(node->key);
        node->key = NULL;

        if(node->value != NULL)
        {
            free(node->value);
            node->value = NULL;
        }
        
        node->hash = 0;
        return RET_OK;
    }

    
    
    int i = 0;
    for(i = 0; i < node->dyn_array_used; i++)
    {
        if(strcmp(key, node->next[i].key) == 0)
        {
            printf("find key in mini node array\n");

            free(node->next[i].key);
            node->next[i].key = NULL;
            free(node->next[i].value);
            node->next[i].value = NULL;

            // node i is not the last one element
            if(i < node->dyn_array_used - 1)
            {
                last_index = node->dyn_array_used - 1;
                memcpy(&node->next[i], &node->next[last_index], sizeof(mini_node_t));
                memset(&node->next[last_index], 0, sizeof(mini_node_t));
            }
            

            return RET_OK;
        }
        
    }    
    
    

    return RET_FAIL;
}



// free internal key and value within mini node array 
void chash_free_mini_array_kv(mini_node_t *head, int size)
{
    if(head == NULL)
        return;

    int i = 0;
    mini_node_t *p_tmp = NULL;
    
    for(i = 0; i < size; i++)
    {
        p_tmp = head + i;
        if(p_tmp->key != NULL)
        {
            free(p_tmp->key);
            p_tmp->key = NULL;
        }

        if(p_tmp->value != NULL)
        {
            free(p_tmp->value);
            p_tmp->value = NULL;
        }
    }
}

void chash_destory(hash_t *hashmap)
{

    if(hashmap == NULL)
        return;

    int i;
    mini_node_t *p = NULL;
    
    for(i = 0; i < hashmap->capacity; i++)
    {
        p = hashmap->buckets[i].next;
        if(p != NULL)
        {
            chash_free_mini_array_kv(p, hashmap->buckets[i].dyn_array_size);
            free(p);
        }
    }

    free(hashmap->buckets);
    hashmap->buckets = NULL;
    hashmap->capacity = 0;
    
    free(hashmap);
}


int main()
{
    printf("hello hash\n");

    hash_t *test_hash = chash_new(100);

    char *key1 = calloc(1, 20);
    strncpy(key1, "abc", 3);

    char *key2 = calloc(1, 20);
    strncpy(key2, "hash", 4);

    char *key3 = calloc(1, 20);
    strncpy(key3, "thu", 3);
    
    int *value1 = malloc(sizeof(int));
    *value1 = 10;
    int *value2 = malloc(sizeof(int));
    *value2 = 20;
    int *value3 = malloc(sizeof(int));
    *value3 = 30;

    
    chash_add(test_hash, key1, value1);
    chash_add(test_hash, key2, value2);
    chash_add(test_hash, key3, value3);

    int *temp = NULL;
    temp = chash_get(test_hash, "thu");
    if(temp)
        printf("get the hash %d\n", *temp);


    int ret = chash_delete(test_hash, "thu");
    if(ret == 0)
        printf("del key successful\n");

    ret = chash_delete(test_hash, "fookey");
    if(ret == 0)
        printf("del key successful\n");


    temp = chash_get(test_hash, "thu");
    if(temp)
        printf("get the hash %d\n", *temp);
    else
        printf("get the hash null\n");
    
    
    chash_destory(test_hash);
}
