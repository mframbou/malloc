#include "./ft_malloc.h"
#include <libft.h>

/**
 * @brief Prints the allocated memory addresses and sizes in the given page list
 *
 * @param pages the pages in which to search for allocated blocks
 * @return size_t the sum of all memory blocks in the given pages
 */
size_t print_alloc_mem_in_pages(t_page *pages)
{
	size_t total_bytes = 0;
	size_t total_allocs = 0;

	for (t_page *page = pages; page != NULL; page = page->next)
	{
		for (t_block *block = page->first; block != NULL; block = block->next)
		{
			total_bytes += block->real_size - sizeof(t_block);
			total_allocs++;

			printf("%p - %p : %zu bytes\n", block->addr, (char *)block->addr + block->real_size - sizeof(t_block), block->real_size - sizeof(t_block));
			if (block == page->last)
				break;
		}
	}

	printf("Total allocations : %zu (%zu bytes)\n", total_allocs, total_bytes);

	return total_bytes;
}

void show_alloc_mem()
{
	size_t total_bytes = 0;

	pthread_mutex_lock(&g_mutex);

	void *tiny_addr = g_data.tiny_pages;
	void *small_addr = g_data.small_pages;
	void *large_addr = g_data.large_pages;

	printf("TINY : %p\n", tiny_addr);
	total_bytes += print_alloc_mem_in_pages(g_data.tiny_pages);

	printf("SMALL : %p\n", small_addr);
	total_bytes += print_alloc_mem_in_pages(g_data.small_pages);

	printf("LARGE : %p\n", large_addr);
	total_bytes += print_alloc_mem_in_pages(g_data.large_pages);

	printf("Total : %zu bytes\n", total_bytes);

	pthread_mutex_unlock(&g_mutex);
}