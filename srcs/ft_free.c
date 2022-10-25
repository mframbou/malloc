#include <libft.h>
#include "./ft_malloc.h"

typedef struct s_find_block_data
{
	t_block	*block;
	t_page *page;
} t_find_block_data;

t_find_block_data find_block_data(void *ptr, t_page *pages)
{
	for (t_page *page = pages; page != NULL; page = page->next)
	{
		for (t_block *block = page->first; block != NULL; block = block->next)
		{
			if (block->addr == ptr)
				return (t_find_block_data){ block, page };

			if (block == page->last)
				break;
		}
	}

	return (t_find_block_data){ NULL, NULL };
}

void ft_free(void *ptr)
{
	if (ptr == NULL)
		return;

	t_find_block_data data = find_block_data(ptr, g_data.tiny_pages);
	if (data.block == NULL)
		data = find_block_data(ptr, g_data.small_pages);
	if (data.block == NULL)
		data = find_block_data(ptr, g_data.large_pages);

	if (data.block == NULL)
		return;


	t_block *block = data.block;
	t_page *page = data.page;

	if (block->prev)
		block->prev->next = block->next;

	if (block->next)
		block->next->prev = block->prev;

	if (block == page->first && block == page->last)
	{
		page->first = NULL;
		page->last = NULL;

		// Only delete page if it's not the first one of small / tiny malloc
		// so that if user do malloc / free / malloc / free the page is not recreated every time
		if (page->prev == NULL && page->next == NULL && page == g_data.tiny_pages)
			return;

		if (page->prev == NULL && page->next == NULL && page == g_data.small_pages)
			return;

		if (page->prev)
			page->prev->next = page->next;

		if (page->next)
			page->next->prev = page->prev;

		if (page == g_data.tiny_pages)
			g_data.tiny_pages = page->next;
		else if (page == g_data.small_pages)
			g_data.small_pages = page->next;
		else if (page == g_data.large_pages)
			g_data.large_pages = page->next;

		if (munmap(page->addr, page->real_size) == -1)
			printf("munmap error\n");
	}
	else if (block == page->first)
	{
		page->first = block->next;
	}
	else if (block == page->last)
	{
		page->last = block->prev;
	}
}