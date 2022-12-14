#ifndef FT_MALLOC_H
#define FT_MALLOC_H

#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include <pthread.h>

#define TINY_FT_MALLOC_MAX_SIZE 1024
#define SMALL_FT_MALLOC_MAX_SIZE 4096

typedef struct s_block
{
	#ifdef FT_MALLOC_FRONT_GUARD
	#if FT_MALLOC_FRONT_GUARD > 8 && FT_MALLOC_FRONT_GUARD < 4096
	char front_guard[FT_MALLOC_FRONT_GUARD];
	#endif
	#endif

	void *addr;
	size_t real_size;
	struct s_block *next;
	struct s_block *prev; // keep prev in case of free so that we can easily remove it without traversing the whole chained list

	#ifdef FT_MALLOC_BACK_GUARD
	#if FT_MALLOC_BACK_GUARD > 8 && FT_MALLOC_BACK_GUARD < 4096
	char back_guard[FT_MALLOC_BACK_GUARD];
	#endif
	#endif
} t_block;

typedef struct s_page
{
	#ifdef FT_MALLOC_FRONT_GUARD
	#if FT_MALLOC_FRONT_GUARD > 8 && FT_MALLOC_FRONT_GUARD < 4096
	char front_guard[FT_MALLOC_FRONT_GUARD];
	#endif
	#endif

	size_t real_size;
	struct s_page *next;
	struct s_page *prev;
	t_block *first;
	t_block *last;

	#ifdef FT_MALLOC_BACK_GUARD
	#if FT_MALLOC_BACK_GUARD > 8 && FT_MALLOC_BACK_GUARD < 4096
	char back_guard[FT_MALLOC_BACK_GUARD];
	#endif
	#endif
} t_page;

typedef struct s_malloc_data
{
	t_page *tiny_pages;
	t_page *small_pages;
	t_page *large_pages;
} t_malloc_data;

extern t_malloc_data g_data;
extern pthread_mutex_t g_mutex;

typedef struct s_find_block_data
{
	t_block	*block;
	t_page *page;
} t_find_block_data;

t_find_block_data find_block_data(void *ptr, t_page *pages);
void internal_free(t_find_block_data data);
t_block *internal_malloc(size_t size);
t_block *allocate_block(size_t size, t_page **pages);
int is_front_guard_intact(t_block *block);
int is_back_guard_intact(t_block *block);
void verify_blocks_integrity();

void *malloc(size_t size);
void *realloc(void *ptr, size_t size);
void free(void *ptr);
void *calloc(size_t count, size_t size);
void show_alloc_mem();
void show_alloc_mem_ex();
size_t get_mmap_pages_count();

#endif