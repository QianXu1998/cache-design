/* Contains cache struct definitions & function prototypes
 *
 * To modify structure of cache, change values below
 */


#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#define I_OFFSET_BITS  6
#define I_INDEX_BITS   14
#define I_TAG_BITS     12
#define I_ADDRESS_SIZE 32

#define D_OFFSET_BITS  6
#define D_INDEX_BITS   14
#define D_TAG_BITS     12
#define D_ADDRESS_SIZE 32

#define ICS (1 << I_INDEX_BITS) // Instruction cache set count
#define DCS (1 << D_INDEX_BITS) // Data cache set count
#define ICA 2                   // Instruction cache associativity
#define DCA 4                   // Data cache associativity

// Bits masking macros
#define I_INDEX_MASK (0x000FFFC0)
#define I_TAG_MASK   (0xFFF00000)
#define i_index(x) ((x & I_INDEX_MASK) >> I_OFFSET_BITS)
#define i_tag(x)   ((x & I_TAG_MASK) >> (I_OFFSET_BITS + I_INDEX_BITS))
#define d_address(t, i) (((t) << (D_INDEX_BITS + D_OFFSET_BITS)) + ((i) << D_OFFSET_BITS))
#define i_address(t, i) ((t) << ((I_INDEX_BITS + I_OFFSET_BITS)) + ((i) << I_OFFSET_BITS))

#define D_INDEX_MASK (0x000FFFC0)
#define D_TAG_MASK   (0xFFF00000)
#define d_index(x) ((x & D_INDEX_MASK) >> D_OFFSET_BITS)
#define d_tag(x)   ((x & D_TAG_MASK) >> (D_OFFSET_BITS + D_INDEX_BITS))


enum MODE_STATE {
	SILENT  = 0,
	VERBOSE = 1,
	DEBUG   = 2};

// Data structure declarations

enum MESI_BITS {
	MODIFIED  = 0,
	EXCLUSIVE = 1,
	SHARED    = 2,
	INVALID   = 3};

// representation of a single cache line
struct cache_line
{
 	enum MESI_BITS MESI;
	uint8_t LRU;
	uint32_t tag;
};

struct cache_set
{
	struct cache_line* line;
};

// representation of a single cache
struct cache
{
	long hits;
	long misses;
	long reads;
	long writes;
	int offset_bits;
	int index_bits;
	int tag_bits;
	int associativity;
	int set_count;
	struct cache_set* set;
};

// global cache performance statistics
long hits, misses, reads, writes;

// General cache function prototypes
int allocate_cache(struct cache*, int way_cnt, int set_cnt, int tag_bits, int index_bits, int offset_bits);
int parse_input(int argc, char* argv[]);
void invalidate_cache(struct cache* cache);
int cache_check(struct cache* cache, int index, int tag);
int update_LRU(struct cache* cache, int index, int tag, int way);
int find_LRU(struct cache* cache, int index);
int find_victim(struct cache* cache, int index);
int display_cache(struct cache* cache);
int tag_mask(struct cache* cache, int address);
int index_mask(struct cache* cache, int address);

