#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "chash.h"


#define RET_OK 0
#define RET_FAIL -1



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
    if(hashmap == NULL || key == NULL || value == NULL)
    {
        printf("parameter error at %s\n", __FUNCTION__);
        return RET_FAIL;
    }
        

    node_t *node = NULL;
    mini_node_t *mini_node_array = NULL;
    int last_index = 0; // last index of mini node array
    unsigned int hash_val = JSHash(key);
    unsigned int index = hash_val % (hashmap->capacity);
    node = &(hashmap->buckets[index]);

    
    if(node->key == NULL)
    { 
        node->key = key;
        node->value = value;
        node->hash = hash_val;  // ASSIGN HASH VALUE
        node->next = NULL;  // mini node array is empty
    }
    else
    {
        printf("bucket[%d] have key string\n", index);

        if(node->hash == hash_val && strcmp(node->key, key) == 0)
        {
            printf("update value\n");
            if(node->value != NULL)
            {
                free(node->value);
            }    
            
            node->value = value;
            return RET_OK;
        }
            
        //ADD to the mini node allocated array

        int i = 0;
        for(i = 0; i < node->dyn_array_used; i++)
        {

            if(node->next[i].hash == hash_val && strcmp(node->next[i].key, key) == 0)
            {
                printf("update value\n");
                free(node->next[i].value);
                node->next[i].value = value;
                return RET_OK;
            }
        }

        // no memory remain, have to add node
        if(i == node->dyn_array_used && node->dyn_array_used == node->dyn_array_size)
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
        mini_node_array[last_index].key = key;
        mini_node_array[last_index].value = value;
        mini_node_array[last_index].hash = hash_val;

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
    
    if(node->key == NULL)
    {
        //printf("does not get the value\n");
        return NULL;
    }
    else
    {
        //printf("search the key\n");

        // there is no mini node
        if(node->dyn_array_used == 0)
        {
            if(strcmp(key, node->key) == 0)
            {
                return node->value;
            }
        }
        else
        {
            int i = 0;
            for(i = 0; i < node->dyn_array_used; i++)
            {
                if(strcmp(key, node->next[i].key) == 0)
                {
                    printf("find key in mini node array\n");
                    if(node->next[i].value == NULL)
                    {
                        printf("the found value pointer is null\n");
                    }

                    return node->next[i].value;
                }
                
            }

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
    
    if(node->key == NULL)
    {
        printf("the bucket have no key\n");
        return RET_FAIL;
    }
    else
    {
        
        if(strcmp(key, node->key) == 0)
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
        

    }

    return RET_FAIL;
}



int main()
{
    printf("hello hash\n");



    hash_t *test_hash = chash_new(100);

    int value1 = 10;
    int value2 = 20;
    int value3 = 30;
    chash_add(test_hash, "abc", &value1);
    chash_add(test_hash, "hash", &value2);
    chash_add(test_hash, "thu", &value3);

    int *temp = NULL;
    temp = chash_get(test_hash, "thu");
    if(temp)
        printf("get the hash %d\n", *temp);


    int ret = chash_delete(test_hash, "thu");
    if(ret == 0)
        printf("del key successful\n");
}
