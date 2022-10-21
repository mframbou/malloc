#include "./srcs/ft_malloc.h"
#include "./libft/libft.h"

#define POUET_DEBUG 0

typedef struct s_vec3
{
	double	x;
	double	y;
	double	z;
}	t_vec3;

typedef struct s_test
{
	double	bitch;
	double	lasagna[50];
	t_vec3	random0;
	t_vec3	random1;
	t_vec3	random2;
	t_vec3	random3;
	t_vec3	random4;
}	t_tg;

int main()
{
	printf("%zu\n", sizeof(t_tg) * 30);
	for (int j = 0; j < 2000; j++)
	{
		t_tg	*oui;

		oui = ft_malloc(sizeof(t_tg) * 30);
		ft_bzero(oui, sizeof(t_tg) * 30);

		for (int i = 0; i < 30; i++)
		{
			t_tg	current = oui[i];
			current.bitch = 1;
			current.random0 = (t_vec3){0, 1, 2};
			current.random1 = (t_vec3){0, 1, 2};
			current.random2 = (t_vec3){0, 1, 2};
			current.random3 = (t_vec3){0, 1, 2};
			current.random4 = (t_vec3){0, 1, 2};
		}
	}

	size_t meta_mmap = get_mmap_meta_count();
	size_t data_mmap = get_mmap_data_count();

	printf("%zu metadata mmap and %zu data mmap\nTotal : %zu mmap\n", meta_mmap, data_mmap, meta_mmap + data_mmap);
	// show_alloc_mem();
	// show_struct_mem_space();
	// show_alloc_pages();
	return (0);
}