#include "cachelab.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <getopt.h>

static int timer = 0;
static int hit = 0;
static int miss = 0;
static int evic = 0;
static int verbose = 0;
static FILE *trace_file;

typedef struct line
{
    int vaild;
    int LRU_count;
    long tag;
} myline;

typedef struct set
{
    struct line *lines;
    int set_index;
} myset;

typedef struct cache
{
    int s_bits;
    int E;
    struct set *sets;
    int b_bits;
} mycache;

void initCache(mycache *cache);
int accessCache(mycache *cache, long address);
void modifyTypeNumber(int type);
void freeCache(mycache *cache);
void tackle_file(mycache *cache, char *vayloc, int verbose);

int main(int argc, char **argv)
{
    int o;
    char *vayloc;
    mycache *cache = malloc(sizeof(mycache));
    const char *opt = "s:E:b:t:vh";
    while ((o = getopt(argc, argv, opt)) != -1)
    {
        switch (o)
        {
        case 's':
            cache->s_bits = atoi(optarg);
            break;
        case 'E':
            cache->E = atoi(optarg);
            break;
        case 'b':
            cache->b_bits = atoi(optarg);
            break;
        case 'h':
            printf("This is a helper!! But i don't know how to write, hahaha ");
            break;
        case 't':
            vayloc = optarg;
            if (vayloc == NULL)
            {
                fprintf(stderr, "Error: Missing trace file (-t).\n");
                exit(1);
            }
            break;
        case 'v':
            verbose = 1;
            break;
        default:
            printf("Undefined arg for commandline");
            exit(1);
        }
    }
    trace_file = fopen(vayloc, "r");
    initCache(cache);
    tackle_file(cache, vayloc, verbose);
    freeCache(cache);
    fclose(trace_file);
    printSummary(hit, miss, evic);
    return 0;
}

void initCache(mycache *cache)
{
    // Initialize the cache to 0
    int set_num = 1 << cache->s_bits;
    myset *set_ptr = (myset *)malloc(sizeof(myset) * set_num);
    cache->sets = set_ptr;
    int size_line = sizeof(myline) * cache->E;

    for (int i = 0; i < set_num; i++)
    {
        set_ptr[i].set_index = i;
        myline *line_ptr = (myline *)malloc(size_line);
        set_ptr[i].lines = line_ptr;
        for (int j = 0; j < cache->E; j++)
        {
            line_ptr[j].vaild = 0;
            line_ptr[j].LRU_count = 0;
            line_ptr[j].tag = 0;
        }
    }
    return;
}

int cal_set_index(mycache *cache, long address)
{
    return (address >> cache->b_bits) & ((1 << cache->s_bits) - 1);
}

int cal_tag(mycache *cache, long address)
{
    return address >> (cache->s_bits + cache->b_bits);
}

int cal_set_num(mycache *cache, long address)
{
    return 1 << cache->s_bits;
}

int accessCache(mycache *cache, long address)
{
    // Check the block whether in the cache or not
    timer += 1;
    int set_index = cal_set_index(cache, address);
    int tag = cal_tag(cache, address);
    for (int i = 0; i < cache->E; i++)
    {
        if ((cache->sets[set_index].lines[i].vaild == 1) && (cache->sets[set_index].lines[i].tag == tag))
        {
            cache->sets[set_index].lines[i].LRU_count = timer;
            return 0;
        }
    }

    for (int i = 0; i < cache->E; i++)
    {
        if (cache->sets[set_index].lines[i].vaild == 0)
        {
            cache->sets[set_index].lines[i].vaild = 1;
            cache->sets[set_index].lines[i].tag = tag;
            cache->sets[set_index].lines[i].LRU_count = timer;
            return 1;
        }
    }

    int min = timer;
    int evic_tag = 0;
    for (int i = 0; i < cache->E; i++)
    {
        if (min > cache->sets[set_index].lines[i].LRU_count)
        {
            min = cache->sets[set_index].lines[i].LRU_count;
            evic_tag = i;
        }
    }
    cache->sets[set_index].lines[evic_tag].vaild = 1;
    cache->sets[set_index].lines[evic_tag].tag = tag;
    cache->sets[set_index].lines[evic_tag].LRU_count = timer;
    return 2;
}

void modifyTypeNumber(int type)
{
    // change the state of cache
    if (type == 0)
    {
        hit += 1;
    }
    else if (type == 1)
    {
        miss += 1;
    }
    else
    {
        miss += 1;
        evic += 1;
    }
    return;
}

void freeCache(mycache *cache)
{
    // free the whole cache
    if (!cache)
    {
        return;
    }
    int set_num = 1 << cache->s_bits;
    for (int i = 0; i < set_num; i++)
    {
        if (cache->sets[i].lines)
        {
            free(cache->sets[i].lines);
            cache->sets[i].lines = NULL;
        }
    }
    free(cache->sets);
    free(cache);
    return;
}

void tackle_file(mycache *cache, char *vayloc, int verbose)
{
    if (verbose == 0)
    {
        char line[1024];
        while (fgets(line, sizeof(line), trace_file) != NULL)
        {
            char op;
            unsigned long long address;
            int size;
            if (sscanf(line, " %c %llx,%d", &op, &address, &size) != 3)
            {
                continue;
            }
            printf("%c %llx, %d\n", op, address, size);
            if (op == 'I')
            {
                continue;
            }

            if (op == 'M')
            {
                int result1 = accessCache(cache, address);
                modifyTypeNumber(result1);
                int result2 = accessCache(cache, address);
                modifyTypeNumber(result2);
            }
            else
            {
                int result = accessCache(cache, address);
                modifyTypeNumber(result);
            }
        }
    }
    else
    {
        char line[1024];
        while (fgets(line, sizeof(line), trace_file) != NULL)
        {
            char op;
            unsigned long long address;
            int size;
            if (sscanf(line, " %c %llx,%d", &op, &address, &size) != 3)
            {
                continue;
            }
            if (op == 'I')
            {
                continue;
            }

            if (op == 'M')
            {
                int result1 = accessCache(cache, address);
                modifyTypeNumber(result1);
                char *string1;
                if (result1 == 0)
                {
                    string1 = "Hit";
                }
                else if (result1 == 1)
                {
                    string1 = "Miss";
                }
                else
                {
                    string1 = "miss evication";
                }
                int result2 = accessCache(cache, address);
                modifyTypeNumber(result2);
                printf("%c %llx, %d %s Hit\n", op, address, size, string1);
            }
            else
            {
                int result = accessCache(cache, address);
                modifyTypeNumber(result);
                char *string1;
                if (result == 0)
                {
                    string1 = "Hit";
                }
                else if (result == 1)
                {
                    string1 = "Miss";
                }
                else
                {
                    string1 = "miss evication";
                }
                printf("%c %llx, %d %s\n", op, address, size, string1);
            }
        }
    }
}
