#include "minilibx/mlx.h"

#define RES_X 500
#define RES_Y 350

typedef struct s_image
{
	void	*img;
	char	*img_addr;
	int		bppixel;
	int		line_len;
	int		endian;
}			t_image;

typedef struct s_wall
{
	int	wall_size;
	int	wall_color;

void	xy_pixel_put(t_image *image, int x, int y, int color)
{
	char	*pixel_pos;

	pixel_pos = image->img_addr + (y * image->line_len + x * (image->bppixel / 8));
	*(int *)pixel_pos = color;
}

void	put_square

void	draw_map(t_image *image)
{
	int	x;
	int	y;

	x = 0;
	y = 0;
	while (

}

int	main(void)
{
	void	*display;
	void	*win;
	t_image	img;

	display = mlx_init();
	win = mlx_new_window(display, RES_X, RES_Y, "cub3D");
	img.img = mlx_new_image(display, RES_X, RES_Y);
	img.img_addr = mlx_get_data_addr(img.img, &img.bppixel, &img.line_len, &img.endian);

}
