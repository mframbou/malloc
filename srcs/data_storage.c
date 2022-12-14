#include <libft.h>
#include "./ft_malloc.h"

size_t g_mmap_pages_count = 0;

size_t get_mmap_pages_count()
{
	return g_mmap_pages_count;
}

void initialize_front_guard(char *addr)
{
	#ifdef FT_MALLOC_FRONT_GUARD
	#if FT_MALLOC_FRONT_GUARD > 8 && FT_MALLOC_FRONT_GUARD < 4096
	ft_bzero(addr, FT_MALLOC_FRONT_GUARD);
	ft_memcpy(addr, "IchIchNH", 8);
	#endif
	#endif
}

void initialize_back_guard(char *addr)
{
	#ifdef FT_MALLOC_BACK_GUARD
	#if FT_MALLOC_BACK_GUARD > 8 && FT_MALLOC_BACK_GUARD < 4096
	ft_bzero(addr, FT_MALLOC_BACK_GUARD);
	ft_memcpy(addr, "PaStAbOx", 8);
	#endif
	#endif
}

int is_front_guard_intact(t_block *block)
{
	#ifdef FT_MALLOC_FRONT_GUARD
	#if FT_MALLOC_FRONT_GUARD > 8 && FT_MALLOC_FRONT_GUARD < 4096
	if (ft_memcmp(block->front_guard, "IchIchNH", 8) != 0)
		return 0;
	#endif
	#endif
	return 1;
}

int is_back_guard_intact(t_block *block)
{
	#ifdef FT_MALLOC_BACK_GUARD
	#if FT_MALLOC_BACK_GUARD > 8 && FT_MALLOC_BACK_GUARD < 4096
	if (ft_memcmp(block->back_guard, "PaStAbOx", 8) != 0)
		return 0;
	#endif
	#endif
	return 1;
}

t_page *initialize_page(void *page_addr, size_t real_page_size)
{
	t_page *page = (t_page *)page_addr;
	ft_bzero(page, sizeof(t_page));
	page->real_size = real_page_size;

	#ifdef FT_MALLOC_FRONT_GUARD
	#if FT_MALLOC_FRONT_GUARD > 8 && FT_MALLOC_FRONT_GUARD < 4096
	initialize_front_guard(page->front_guard);
	#endif
	#endif

	#ifdef FT_MALLOC_BACK_GUARD
	#if FT_MALLOC_BACK_GUARD > 8 && FT_MALLOC_BACK_GUARD < 4096
	initialize_back_guard(page->back_guard);
	#endif
	#endif

	return page;
}

t_block *initialize_block(void *block_addr, size_t real_block_size)
{
	t_block *block = (t_block *)block_addr;
	ft_bzero(block, sizeof(t_block));
	block->real_size = real_block_size;
	block->addr = (char *)block + sizeof(t_block);

	#ifdef FT_MALLOC_FRONT_GUARD
	#if FT_MALLOC_FRONT_GUARD > 8 && FT_MALLOC_FRONT_GUARD < 4096
	initialize_front_guard(block->front_guard);
	#endif
	#endif

	#ifdef FT_MALLOC_BACK_GUARD
	#if FT_MALLOC_BACK_GUARD > 8 && FT_MALLOC_BACK_GUARD < 4096
	initialize_back_guard(block->back_guard);
	#endif
	#endif

	return block;
}

/**
 * @brief Creates a new page with the specified size, sets it's metadata and returns it.
 *
 * @param size the size of the page to create (will be rounded up to the nearest multiple of getpagesize())
 * @param pages the page list in which to store the new page
 * @return t_page*
 */
t_page *add_new_page(size_t size, t_page **pages)
{
	void *addr;

	size_t page_size = getpagesize();
	size_t alloc_size = size / page_size * page_size + (size % page_size == 0 ? 0 : page_size);

	if ((addr = mmap(NULL, alloc_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, 0, 0)) == MAP_FAILED) // we dont care about last 2 args(fd and offset)
		return NULL;

	g_mmap_pages_count += alloc_size / page_size;

	t_page *page = initialize_page(addr, alloc_size);

	if (!*pages)
		*pages = page;
	else
	{
		t_page *tmp = *pages;
		while (tmp->next)
			tmp = tmp->next;
		tmp->next = page;
		page->prev = tmp;
	}

	return page;
}

/**
 * @brief Find free space in the given pages and create a block if found(until page == NULL)
 *
 * @param size size of free space needed
 * @param pages the pages in which to search (linked list)
 * @return t_block * the newly created block
 * @return NULL if no free space found
 */
t_block *create_block_if_space(size_t size, t_page *pages)
{
	// real-block size is memory aligned on 8 bytes (see https://en.wikipedia.org/wiki/Data_structure_alignment)
	size_t real_block_size = size + sizeof(t_block);

	for (t_page *page = pages; page != NULL; page = page->next)
	{
		if (!page->first)
		{
			// page is always aligned so no need to check
			if (page->real_size - sizeof(t_page) < real_block_size)
				continue;

			// create block at the start of page (first sizeof(t_page) bytes are reserved for page metadata)
			t_block *block = initialize_block((char *)page + sizeof(t_page), real_block_size);
			page->first = block;
			page->last = block;

			for (t_page *tmp = page->prev; tmp != NULL; tmp = tmp->prev)
			{
				if (tmp->last)
				{
					tmp->last->next = block;
					block->prev = tmp->last;
					break;
				}
			}

			return block;
		}

		char *page_start_usable_addr = (char *)page + sizeof(t_page);

		size_t free_before_first_block = (char *)page->first - page_start_usable_addr;
		if (free_before_first_block >= real_block_size)
		{
			// sizeof(something) is always aligned so no need to check
			t_block *block = initialize_block(page_start_usable_addr, real_block_size);
			block->next = page->first;
			if (page->first)
			{
				block->prev = page->first->prev;
				if (page->first->prev)
					page->first->prev->next = block;
				page->first->prev = block;
			}
			page->first = block;

			return block;
		}

		for (t_block *block = page->first; block->next != NULL && block != page->last; block = block->next)
		{
			// alignment_offset is the number of  bytes we should add to the address to make it aligned

			size_t alignment_offset = block->real_size % 8 == 0 ? 0 : 8 - block->real_size % 8;
			size_t free_between_blocks = (char *)block->next - ((char *)block + block->real_size + alignment_offset);

			if (free_between_blocks >= real_block_size)
			{
				t_block *new_block = initialize_block((char *)block + block->real_size + alignment_offset, real_block_size);
				new_block->next = block->next;
				new_block->prev = block;
				block->next->prev = new_block;
				block->next = new_block;
				return new_block;
			}
		}

		size_t alignment_offset = page->last->real_size % 8 == 0 ? 0 : 8 - page->last->real_size % 8;
		size_t free_after_last_block = ((char *)page + page->real_size) - ((char *)page->last + page->last->real_size + alignment_offset);
		if (free_after_last_block >= real_block_size)
		{
			t_block *block = initialize_block((char *)page->last + page->last->real_size + alignment_offset, real_block_size);
			block->prev = page->last;
			block->next = page->last->next;
			if (page->last->next)
				page->last->next->prev = block;
			page->last->next = block;
			page->last = block;
			return block;
		}
	}

	return NULL;
}

/**
 * @brief Either creates a new page or finds a page with enough free space and creates a block in it.
 *
 * @param size size of the block to create
 * @param pages the pages in which to search for free space and add the block
 * @return t_block *the addres of the newly created block
 */
t_block *allocate_block(size_t size, t_page **pages)
{
	t_block *new_block;

	new_block = create_block_if_space(size, *pages);

	if (new_block)
		return new_block;

	int page_size = getpagesize();
	t_page *new_page = NULL;

	// no free space found, allocate new page
	size_t new_page_size;

	if (size <= TINY_FT_MALLOC_MAX_SIZE)
		new_page_size = TINY_FT_MALLOC_MAX_SIZE * 100 + sizeof(t_page);
	else if (size <= SMALL_FT_MALLOC_MAX_SIZE)
		new_page_size = SMALL_FT_MALLOC_MAX_SIZE * 100 + sizeof(t_page);
	else
		new_page_size = size + sizeof(t_page);

	// count t_page in needed size, otherwise if eg we malloc 4096*2, there will be no space for page and segfault
	new_page = add_new_page(new_page_size, pages);

	if (!new_page)
	{
		ft_putstr_fd("cannot add new page (mmap of ", STDOUT_FILENO);
		ft_putnbr_fd(new_page_size, STDOUT_FILENO);
		ft_putstr_fd(" failed)\n", STDOUT_FILENO);
		return NULL;
	}

	new_block = initialize_block((char *)new_page + sizeof(t_page), size + sizeof(t_block));
	new_page->first = new_block;
	new_page->last = new_block;

	// Set previous block->next to block (check all pages until we find one page not empty)
	for (t_page *tmp = new_page->prev; tmp != NULL; tmp = tmp->prev)
	{
		if (tmp->last)
		{
			tmp->last->next = new_block;
			new_block->prev = tmp->last;
			break;
		}
	}

	return new_block;
}