/*
 * main.c
 *
 * ECE 585 Final Project
 * Authors:
 * Manisha Balakrishna Garade
 * Amruta Kalambkar
 * Rashmi Rajendra Kulkarni
 * Daniel Christiansen
 *
 * Version 1.0
 */

 /*
  * To modify structure of cache, modify attached cache.h file
  */


#include <string.h>
#include "cache.h"

// Main cache operation functions
int read_data(int address);
int read_instruction(int address);
int write_data(int address);
void clear_cache();
void print_contents();
void invalidate_line(int address);
void return_data(int address);

// global variables
enum MODE_STATE mode; // Modes: silent, verbose, debug
struct cache data_cache, instruction_cache;

// index into cache line like so:
// data_cache.set[i].line[j].LRU, etc

int main(int argc, char* argv[])
{
	unsigned int operation;
	unsigned int address;
	FILE *fp;
	char ch;
	int s_ret;

	// Allocates caches
	if(allocate_cache(&data_cache, DCA, DCS, D_TAG_BITS, D_INDEX_BITS, D_OFFSET_BITS))
		return -1;
	if(allocate_cache(&instruction_cache, ICA, ICS, I_TAG_BITS, I_INDEX_BITS, I_OFFSET_BITS))
		return -1;

    // Clear caches & reset statistic so that all lines are invalid
    clear_cache();

	// set mode and filename globals
	if(parse_input(argc, argv))
		return -1;

	fp = fopen(argv[2],"r");
	if(NULL == fp)
	{
		fprintf(stderr, "Unable to open file: %s\n", argv[2]);
		return -1;
	}

	while(fscanf(fp,"%d",&operation)!=EOF)
    {
		if(operation==0)
		{
			fscanf(fp,"%X\n",&address);
			if(mode == DEBUG)
				printf("operation %d, Address %X\n", operation, address);
			read_data(address);
		}
		else if(operation==1)
        {
			fscanf(fp,"%X\n",&address);
			if(mode == DEBUG)
				printf("operation %d, Address %X\n", operation, address);
			write_data(address);
		}
		else if (operation == 2)
        {
			fscanf(fp,"%X\n",&address);
			if(mode == DEBUG)
				printf("operation %d, Address %X\n", operation, address);
			read_instruction(address);
		}
		else if(operation== 3)
        {
			fscanf(fp,"%X\n",&address);
			if(mode == DEBUG)
				printf("operation %d, Address %X\n", operation, address);
			invalidate_line(address);
		}
		else if (operation == 4)
        {
			fscanf(fp,"%X\n",&address);
			if(mode == DEBUG)
				printf("operation %d, Address %X\n", operation, address);
			return_data(address);
		}
		else if(operation == 8)
		{   printf("operation=%d\n",operation);
			clear_cache();
			do {
				s_ret = fscanf(fp, "%c", &ch);
			} while (((int)ch != 10) && ((int)ch !=13) && (s_ret != EOF));
        }
		else if(operation == 9)
		{   printf("operation=%d\n",operation);
			print_contents();
            do {
				s_ret = fscanf(fp, "%c", &ch);
			} while (((int)ch != 10) && ((int)ch !=13) && (s_ret != EOF));
		}
		else
        {
			fprintf(stderr, "Invalid operation\n");
			do {
				s_ret = fscanf(fp, "%c", &ch);
            } while (((int)ch != 10) && ((int)ch !=13) && (s_ret != EOF));
		}
	}

	return 0;
}

// Parses command line input
int parse_input(int argc, char* argv[])
{
	if(argc < 2 || argc > 3)
	{
	    fprintf(stderr, "Incorrect number of arguments.\n");
		return -1;
    }

    if(0 == strcmp(argv[1], "-s"))
        mode = SILENT;
    else if(0 == strcmp(argv[1], "-v"))
        mode = VERBOSE;
	else if(0 == strcmp(argv[1], "-d"))
		mode = DEBUG;
    else if(0 == strcmp(argv[1], "--help"))
    {
        printf("\nProgram use:");
        printf("\n\tcachesim -[flag] [filename]");
        printf("\n\t-v\tverbose\t- print read/write operations");
        printf("\n\t-s\tsilent\t- only output information on command\n");
		return -1;
    }
    else
    {
        fprintf(stderr, "Invalid argument '%s'.\n", argv[1]);
        return -1;
    }

	return 0;
}

// Reads data cache
int read_data(int address)
{
	// mask tag & index
	// call cache_check
	// if hit, call update_LRU
	// if miss
	//   send read from L2 signal
	//   find line to evict, replace line, update LRU
	//   new line should be in shared state

	int new_index, new_tag, i;
	// mask tag & index
	new_index = d_index(address);
	new_tag = d_tag(address);

	// call cache_check
	i = cache_check(&data_cache, new_index, new_tag);
	if (i != -1)
	{
		// if hit, update LRU & statistics
		update_LRU(&data_cache, new_index, new_tag, i);
		data_cache.hits++;
		hits++;
		if (mode==DEBUG)
        {
            printf("Data cache hit count=%d\n",data_cache.hits);
            printf("MESI=%d\n",data_cache.set[new_index].line[i].MESI);
	}
	}
	else
	{
		data_cache.misses++;
		misses++;
		// if miss, find line to evict, replace line, update LRU
		i = find_victim(&data_cache, new_index);
		// Write back only when line is modified
		if((data_cache.set[new_index].line[i].MESI == MODIFIED)
			&& ((mode == VERBOSE) || (mode == DEBUG)))
			printf("Write to L2 %x\n",
				d_address(data_cache.set[new_index].line[i].tag,
				new_index));
		data_cache.set[new_index].line[i].tag = new_tag;
		data_cache.set[new_index].line[i].MESI = SHARED;
		update_LRU(&data_cache, new_index, new_tag, i);
		if((mode == VERBOSE) || (mode == DEBUG))
         	printf("Read from L2 %x\n", address);
         	if (mode== DEBUG)
              {
                printf("Data cache miss count=%d\n",data_cache.misses);
                printf("MESI = %d \n",data_cache.set[new_index].line[i].MESI);
              }
	}
	data_cache.reads++;
	reads++;
	if(mode==DEBUG)
        printf("Data cache read count=%d\n",data_cache.reads);
}


int write_data(int address)
{
	// mask index & tag bits
	// call cache_check function
	// if hit:
	//   if in shared state, write through, update MESI & LRU
	//   if not, update LRU
	// if miss:
	//   read for ownership from L2
	//   put in modified state

	int new_index, new_tag, i;

	// mask tag & index
	new_index = d_index(address);
	new_tag = d_tag(address);

	// call cache_check
	i = cache_check(&data_cache, new_index, new_tag);
	if (i != -1) // if hit
	{
		data_cache.hits++;
		hits++;
		if(mode==DEBUG)
		{
            printf("Data cache hit count = %d\n",data_cache.hits);
		}
		//if in shared state, write through
		if (data_cache.set[new_index].line[i].MESI == SHARED)
		{
			if((mode == VERBOSE) || (mode == DEBUG))
				printf("Write to L2 %x\n", address);// write through
			update_LRU(&data_cache, new_index, new_tag, i);
 			 //update MESI & LRU
			data_cache.set[new_index].line[i].MESI = EXCLUSIVE;
			if (mode==DEBUG)
                printf("MESI=%d\n",data_cache.set[new_index].line[i].MESI);
		}
		else // if in E or M, write back
		{
			//update LRU
			if (data_cache.set[new_index].line[i].MESI == EXCLUSIVE)
				data_cache.set[new_index].line[i].MESI = MODIFIED;
			update_LRU(&data_cache, new_index, new_tag, i);
			if (mode==DEBUG)
                printf("MESI=%d\n",data_cache.set[new_index].line[i].MESI);
		}
	}
	else  // if miss:
	{
		data_cache.misses++;
		misses++;
		i = find_victim(&data_cache, new_index);
		if(mode==DEBUG)
            printf("Data cache miss count=%d\n",data_cache.misses);
		//update_LRU(&data_cache, new_index, new_tag, i);
		// Write back only when line is modified
		if((data_cache.set[new_index].line[i].MESI == MODIFIED)
			&& ((mode == VERBOSE) || (mode == DEBUG)))
			printf("Write to L2 %x\n",
				d_address(data_cache.set[new_index].line[i].tag,
				new_index));
		data_cache.set[new_index].line[i].tag = new_tag;
		data_cache.set[new_index].line[i].MESI = SHARED;
		if(mode==DEBUG)
            printf("MESI=%d\n",data_cache.set[new_index].line[i].MESI);
		update_LRU(&data_cache, new_index, new_tag, i);
		if((mode == VERBOSE) || (mode == DEBUG))
        {
		 	printf("Read for ownership from L2 %x\n", address);
			printf("Write to L2 %x\n", address);
		}
		data_cache.set[new_index].line[i].MESI = EXCLUSIVE;
        if(mode==DEBUG)
        printf("MESI=%d\n",data_cache.set[new_index].line[i].MESI);
	}
	data_cache.writes++;
	writes++;
	if(mode==DEBUG)
        printf("Data cache write count= %d\n",data_cache.writes);
}

int read_instruction(int address)
{
	// mask tag & index
	// call cache_check
	// if hit, call update_LRU
	// if miss
	//   send read from L2 signal
	//   find line to evict, replace line, update LRU
	//   new line should be in shared state

	int new_index, new_tag, i;
	// mask tag & index
	new_index = i_index(address);
	new_tag = i_tag(address);

	 // call cache_check
	i = cache_check(&instruction_cache, new_index, new_tag);
	if (i != -1)
	{
		// if hit, update LRU
		update_LRU(&instruction_cache, new_index, new_tag, i);

		instruction_cache.hits++;
		hits++;
		if (mode==DEBUG)
        {printf("Instruction cache hit count=%d\n",instruction_cache.hits);
        printf("MESI=%d",instruction_cache.set[new_index].line[i].MESI);
	}
	}
	else
	{
		// if miss, find line to evict, replace line, update LRU
		i = find_victim(&instruction_cache, new_index);
		// Write back only when line is modified
		if((instruction_cache.set[new_index].line[i].MESI == MODIFIED)
			&& ((mode == VERBOSE) || (mode == DEBUG)))
			printf("Write to L2 %x\n",
				i_address(instruction_cache.set[new_index].line[i].tag, new_index));
		instruction_cache.set[new_index].line[i].tag = new_tag;
		instruction_cache.set[new_index].line[i].MESI = SHARED;
		update_LRU(&instruction_cache, new_index, new_tag, i);
		if((mode == VERBOSE) || (mode == DEBUG))
         	printf("Read from L2 %x\n", address);

		instruction_cache.misses++;
		misses++;
		if (mode==DEBUG)
        {
            printf("MESI=%d\n",instruction_cache.set[new_index].line[i].MESI);
            printf("Instruction cache miss count=%d\n",instruction_cache.misses);
        }
	}
	instruction_cache.reads++;
	reads++;
	if (mode==DEBUG)
        printf("Instruction read count=%d\n",instruction_cache.reads);
}

// Invalidates a line
void invalidate_line(int address)
{
	int new_tag, new_index, i;

	// Check if line is in data cache
	new_tag = d_tag(address);
	new_index = d_index(address);
	i = cache_check(&data_cache, new_index, new_tag);
	if(i != -1)
	{
		if((data_cache.set[new_index].line[i].MESI == MODIFIED) ||
			(data_cache.set[new_index].line[i].MESI == EXCLUSIVE))
			fprintf(stderr, "Warning: Invalidating modified line at address %x\n",
			d_address(new_tag, new_index));
		data_cache.set[new_index].line[i].MESI = INVALID;
		if(mode==DEBUG)
            printf("MESI=%d\n",data_cache.set[new_index].line[i].MESI);
		return;
	}

	// Check if line is instruction cache
	new_tag = i_tag(address);
	new_index = i_index(address);
	i = cache_check(&instruction_cache, new_index, new_tag);
	if(i != -1)
	{
		instruction_cache.set[new_index].line[i].MESI = INVALID;
		if((mode == VERBOSE) || (mode == DEBUG))
			printf("Invalidating address %x\n", address);
	}
}

// Sends data to L2 (in response to a snoop)
void return_data(int address)
{
	int new_tag, new_index, i;

	// Mask index & tag from address
	new_tag = d_tag(address);
	new_index = d_index(address);

	i = cache_check(&data_cache, new_index, new_tag);
	if((i != -1) &&
		(data_cache.set[new_index].line[i].MESI == MODIFIED))
	{
		if((mode == VERBOSE) || (mode == DEBUG))
			{
			    printf("Return data to L2 %x\n", address);
		data_cache.set[new_index].line[i].MESI = SHARED;
			}
		if(mode==DEBUG)
            printf("MESI=%d\n",data_cache.set[new_index].line[i].MESI);
	}
}

// Clears all caches and resets statistics
void clear_cache()
{
	hits = 0;
	misses = 0;
	reads = 0;
	writes = 0;


	invalidate_cache(&data_cache);
	invalidate_cache(&instruction_cache);

}

// Prints cache statistics & valid cache content
void print_contents()
{
	// Global statistics
	printf("\nGlobal cache statistics:\n");
	printf("Reads:\t%ld\n", reads);
	printf("Writes:\t%ld\n", writes);
 	printf("Hits:\t%ld\n", hits);
	printf("Misses:\t%ld\n", misses);
	printf("Hit rate:\t%f\n", ((float)hits / ((float)hits + (float)misses)));

	// Data cache statistics & contents
	printf("\nData cache statistics:\n");
	printf("Reads:\t%ld\n", data_cache.reads);
	printf("Writes:\t%ld\n", data_cache.writes);
 	printf("Hits:\t%ld\n", data_cache.hits);
	printf("Misses\t%ld\n", data_cache.misses);
	printf("Hit rate:\t%f\n", ((float)data_cache.hits / ((float)data_cache.hits +(float)data_cache.misses)));
	printf("\nData ");
	display_cache(&data_cache);

	// Instruction cache statistics & contents
	printf("\nInstruction cache statistics:\n");
	printf("Reads:\t%ld\n", instruction_cache.reads);
	printf("Writes:\t%ld\n", instruction_cache.writes);
 	printf("Hits:\t%ld\n", instruction_cache.hits);
	printf("Misses\t%ld\n", instruction_cache.misses);
	printf("Hit rate:\t%f\n", ((float)instruction_cache.hits / ((float)instruction_cache.hits + (float)instruction_cache.misses)));
	printf("\nInstruction ");
	display_cache(&instruction_cache);
}

