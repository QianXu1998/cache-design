// cache.c
// Contains functions for general cache operation
// Functions in this file shouldn't know the difference
// between data and instruction caches.

#include "cache.h"
enum MODE_STATE mode;
// Index masking function
int index_mask(struct cache* cache, int address)
{
	if(!cache)
		return -1;

	return ((address << cache->tag_bits) >> (cache->tag_bits + cache->offset_bits));
}

// Tag masking function
int tag_mask(struct cache* cache, int address)
{
	if(!cache)
		return -1;

	return (address >> (cache->index_bits + cache->offset_bits));
}

// Allocates cache structure
// Pass in pointer to cache struct, associativity, and set count
// Returns 0 on success, -1 on failure
int allocate_cache(struct cache* new_cache , int way_cnt,
	int set_cnt, int tag_bits, int index_bits, int offset_bits)
{
	int i;
	// The cache should store its own specs
	new_cache->associativity = way_cnt;
	new_cache->set_count = set_cnt;
	new_cache->offset_bits = offset_bits;
	new_cache->index_bits = index_bits;
	new_cache->tag_bits = tag_bits;
	// Allocate array of set pointer
	new_cache->set = calloc(set_cnt, sizeof(struct cache_set));
	if(NULL == new_cache->set)
	{
		fprintf(stderr, "Unable to allocate set array.\n");
		return -1;
	}
	// Allocate array of lines for each set
	for(i = 0; i < set_cnt; i++)
	{
		new_cache->set[i].line =
			calloc(way_cnt, sizeof(struct cache_line));
		if(NULL == new_cache->set[i].line)
		{
			fprintf(stderr, "Unable to allocate line array.\n");
			return -1;
		}
	}

	return 0;
}

// This function checks the cache for an address
// Pass tag and index to check
// Returns an index, or -1 on miss
int cache_check(struct cache* cache, int index, int tag)
{
	if(NULL == cache)
	{
		fprintf(stderr, "Null pointer passed to cache_check.\n");
		return -1;
	}

	int i;

	for(i = 0; i < cache->associativity; i++)
	{
		if((cache->set[index].line[i].tag == tag)
			&& (cache->set[index].line[i].MESI != INVALID))
			return i;
	}

	return -1;
}

// This function updates the LRU bits for a single set
// Pass most recently used line
// Doesn't check that the tags are valid,
// just makes the line indicated by 'way' the MRU.
int update_LRU(struct cache* cache, int index, int tag, int way)
{
	if(!cache)
	{
		fprintf(stderr, "Null pointer passed to update_LRU.\n");
		return -1;
	}

	int i, old_count;
	struct cache_set* curr_set = &cache->set[index];

	old_count = curr_set->line[way].LRU;
	curr_set->line[way].LRU = 0;
	for(i = 0; i < cache->associativity; i++)
	{
		if((way != i) && (curr_set->line[i].LRU <= old_count))
			{
			    curr_set->line[i].LRU++;


			}
			 if(mode==DEBUG)
                printf("Updated LRU=%d \n",curr_set->line[i].LRU);


	}

	return 0;
}

// This function 'clears' the cache:
// Sets all MESI bits for a cache to invalid
void invalidate_cache(struct cache* target_cache)
{
	if(NULL == target_cache)
	{
		fprintf(stderr, "Null pointer passed to clear_cache.\n");
		return;
	}

	int i, j;

	if(NULL == target_cache->set)
	{
		fprintf(stderr, "Null set pointer in clear_cache.\n");
		return;
	}

	target_cache->hits = 0;
	target_cache->misses = 0;
	target_cache->reads = 0;
	target_cache->writes = 0;

	for(i = 0; i < target_cache->set_count; i++)
	{
		if(NULL == (target_cache->set[i].line))
		{
			fprintf(stderr, "Null line pointer in clear_cache.\n");
			return;
		}
		for(j = 0; j < target_cache->associativity; j++)
			target_cache->set[i].line[j].MESI = INVALID;
	}
}


// This function finds the LRU line in a set
// Pass pointer to target cache and index
// Returns index of LRU line, or -1 on failure
int find_LRU(struct cache* cache, int index)
{
	if(!cache)
	{
		fprintf(stderr, "Null pointer passed to find_LRU.\n");
		return -1;
	}

	int i, max_index = 0;
	struct cache_set* curr_set = &(cache->set[index]);

	for(i = 0; i < cache->associativity; i++)
	{
		if(curr_set->line[i].LRU > curr_set->line[max_index].LRU)
		{
			max_index = i;
		}
	}

	return max_index;
}

// Finds line to replace
// Returns way of line in set/index
// You'll need to check for valid/invalid yourself
int find_victim(struct cache* cache, int index)
{
	struct cache_set* curr_set = &(cache->set[index]);
	int i;

	// Look for invalid/empty line
	for(i = 0; i < cache->associativity; i++)
	{
		if(curr_set->line[i].MESI == INVALID)
			return i;
	}

	// Else, return LRU line
	return find_LRU(cache, index);
}


// Displays tag, LRU, and MESI information
//  of valid lines in a cache
int display_cache(struct cache* cache)
{
	int i, j;
	int ways = cache->associativity;
	int sets = cache->set_count;
	bool valid_set;

	printf("cache contents:\n");
	printf("index: LRU/MESI/tag\n");

	for(i = 0; i < sets; i++)
	{
		valid_set = false;
		for(j = 0; j < ways; j++)
		{
			if(cache->set[i].line[j].MESI != INVALID)
				valid_set = true;
				break;
		}
		if(valid_set)
		{
			printf("\n%04x:\t", i);
			for(j = 0; j < ways; j++)
			{
				if(cache->set[i].line[j].MESI != INVALID)
				{
					printf("%d ", cache->set[i].line[j].LRU);
					switch (cache->set[i].line[j].MESI) {
						case MODIFIED: printf("M "); break;
						case EXCLUSIVE: printf("E "); break;
						case SHARED: printf("S "); break;
					}
					printf("%03x   ", cache->set[i].line[j].tag);
				}
			}
		}
	}
	printf("\n");
}

