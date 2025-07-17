#include "minilibx/mlx.h"
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <X11/keysym.h>
#include <math.h>

#define RES_X 1200
#define RES_Y 1200

#define FAILURE 1
#define SUCCESS 0

typedef struct s_size
{
	int	x;
	int	y;
}				t_size;

typedef struct s_point
{
	double	x;
	double	y;
}				t_point;

typedef struct s_minimap
{
	void	*img;
	char	*img_addr;
	int		bppixel;
	int		line_len;
	int		endian;
	t_size	size;
	int		cell_size;
}				t_minimap;

typedef struct s_scene
{
	void	*img;
	char	*img_addr;
	int		bppixel;
	int		line_len;
	int		endian;
	t_size	size;
}				t_scene;

typedef struct s_wall
{
	int	wall_size;
	int	wall_color;
}				t_wall;

typedef struct s_x_elements
{
	void		*display;
	void		*win;
	t_minimap	minimap;
	t_scene		scene;
	int			refresh;
}				t_x_elements;

typedef struct s_world
{
	char	**map;
	int		map_x;
	int		map_y;
	int		orientation;
	t_point	player;
	t_point	offest;
	int		ceiling_color;
	int		floor_color;
}				t_world;

int	double_to_pixel(double world_point, int cell_size)
{
	return (world_point * cell_size);
}

void	xy_pixel_put(t_minimap *minimap, int x, int y, int color)
{
	char	*pixel_pos;

	pixel_pos = minimap->img_addr + (y * minimap->line_len + x * (minimap->bppixel / 8));
	*(int *)pixel_pos = color;
}

int	set_cell_size()
{
	int	cell_size;

	if (RES_X > RES_Y)
		cell_size = ((RES_X - RES_Y) / 2 + RES_Y) / 10;
	if (RES_Y > RES_X)
		cell_size = ((RES_Y - RES_X) / 2 + RES_X) / 10;
	if (RES_X == RES_Y)
		cell_size = RES_X / 10;
	return (cell_size);
}

t_size	set_minimap_size(t_world *world, int cell_size)
{
	t_size	minimap_size;

	minimap_size.x = world->map_x * cell_size;
	minimap_size.y = world->map_y * cell_size;
	return (minimap_size);
}

char **alloc_map(void) //TEST
{
	char **map;
	int	i = 0;

	map = calloc(51, 8);
	map[50] = NULL;
	while (i < 50)
	{
		map[i] = malloc(51);
		map[i][50] = '\0';
		map[i][49] = '1';
		map[i][0] = '1';
		i++;
	}
	memset(&map[0][1], '1', 48);
	memset(&map[49][1], '1', 48);
	i = 1;
	while (i < 49)
	{
		memset(&map[i][1], '0', 48);
		i++;
	}
	map[3][6] = '1';
	map[3][4] = '1';
	map[1][3] = '1';
	map[2][2] = '1';
	map[2][4] = '1';
	map[5][3] = '1';
	map[5][5] = '1';
	return (map);
}

void	print_map(char **map) //TEST
{
	int i;

	i = 0;
	while (map[i] != NULL)
	{
		printf("%s\n", map[i]);
		i++;
	}
}

void	init_world(t_world *world) //TEST
{
	world->map = alloc_map();
	world->map_x = 50;
	world->map_y = 50;
	world->player.x = 10.5;
	world->orientation = 0;
	world->player.y = 10.5;
}

int	init_minimap_image(t_world *world, t_x_elements *x_elem)
{
	x_elem->minimap.cell_size = set_cell_size();
	x_elem->minimap.size = set_minimap_size(world, x_elem->minimap.cell_size);
	x_elem->minimap.img = mlx_new_image(x_elem->display, x_elem->minimap.size.x, x_elem->minimap.size.y);
	if (x_elem->minimap.img == NULL)
		return (FAILURE);
	x_elem->minimap.img_addr = mlx_get_data_addr(x_elem->minimap.img, &x_elem->minimap.bppixel, &x_elem->minimap.line_len, &x_elem->minimap.endian);
	return (SUCCESS);
}

int	init_scene_image(t_world *world, t_x_elements *x_elem)
{
	x_elem->scene.size = (t_size){RES_X, RES_Y};
	x_elem->scene.img = mlx_new_image(x_elem->display, x_elem->scene.size.x, x_elem->scene.size.y);
	if (x_elem->scene.img == NULL)
		return (FAILURE);
	x_elem->scene.img_addr = mlx_get_data_addr(x_elem->scene.img, &x_elem->scene.bppixel, &x_elem->scene.line_len, &x_elem->scene.endian);
	return (SUCCESS);
}

int	x_init(t_x_elements *x_elem, t_world *world)
{
	x_elem->refresh = 1;
	x_elem->display = mlx_init();
	if (x_elem->display == NULL)
		return (FAILURE);
	x_elem->win = mlx_new_window(x_elem->display, RES_X, RES_Y, "cub3D");
	if (x_elem->win == NULL)
	{
		mlx_destroy_display(x_elem->display);
		return (FAILURE);
	}
	if (init_minimap_image(world, x_elem) == FAILURE)
	{
		mlx_destroy_window(x_elem->display, x_elem->win);
		mlx_destroy_display(x_elem->display);
		return (FAILURE);
	}
	if (init_scene_image(world, x_elem) == FAILURE)
	{
		mlx_destroy_image(x_elem->display, x_elem->minimap.img);
		mlx_destroy_window(x_elem->display, x_elem->win);
		mlx_destroy_display(x_elem->display);
		return (FAILURE);
	}
	return (SUCCESS);
}

void	draw_grid(t_minimap *minimap) //TEST
{
	int	x;
	int	y;

	y = 0;
	while (y < minimap->size.y)
	{
		x = 0;
		while (x < minimap->size.x)
		{
			{
				if (y % minimap->cell_size == 0 || x % minimap->cell_size == 0)
					xy_pixel_put(minimap, x, y, 0Xffffff);
			}
			x++;
		}
		y++;
	}
}
void	draw_square(t_minimap *minimap, int *x, int *y, int color) //TEST
{
	int	square_x;
	int	square_y;
	int	temp_x;
	int	temp_y;

	temp_y = *y;
	temp_x = *x;
	square_y = 0;
	while (square_y < minimap->cell_size)
	{
		*x = temp_x;
		square_x = 0;
		while (square_x < minimap->cell_size)
		{
			xy_pixel_put(minimap, *x, *y, color);
			*x = *x + 1;
			square_x++;
		}
		*y = *y + 1;
		square_y++;
	}
	*y = temp_y;
}

void	place_player(t_minimap *minimap, t_world *world) //TEST
{
	int	player_pixel_x;
	int	player_pixel_y;

	player_pixel_x = world->player.x * minimap->cell_size;
	player_pixel_y = world->player.y * minimap->cell_size;
	xy_pixel_put(minimap, player_pixel_x, player_pixel_y, 0xff0000);
}

void	erase_player(t_minimap *minimap, t_world *world) //TEST
{
	int	player_pixel_x;
	int	player_pixel_y;

	player_pixel_x = world->player.x * minimap->cell_size;
	player_pixel_y = world->player.y * minimap->cell_size;
	xy_pixel_put(minimap, player_pixel_x, player_pixel_y, 0x000000);
}

void	draw_line(t_point start, t_point end, t_world *world, t_x_elements *x_elem) //TEST
{
	int		start_x;
	int		start_y;
	int		end_x;
	int		end_y;
	int		delta_x;
	int		delta_y;
	int		steps;
	double	x_direction;
	double	y_direction;
	double	x;
	double	y;

	start_x = double_to_pixel(start.x, x_elem->minimap.cell_size);
	start_y = double_to_pixel(start.y, x_elem->minimap.cell_size);
	end_x = double_to_pixel(end.x, x_elem->minimap.cell_size);
	end_y = double_to_pixel(end.y, x_elem->minimap.cell_size);
	// printf("start_x: %d start_y: %d | end_x: %d end_y: %d | offset_x: %f offset_y: %f\n", start_x, start_y, end_x, end_y, world->offest.x, world->offest.y);
	delta_x = end_x - start_x;
	delta_y = end_y - start_y;

	if (abs(delta_x) > abs(delta_y))
		steps = abs(delta_x);
	else
		steps = abs(delta_y);

	x_direction = delta_x / (double)steps;
	y_direction = delta_y / (double)steps;

	x = start_x;
	y = start_y;
	while (steps)
	{
		mlx_pixel_put(x_elem->display,
					x_elem->win,
					(int)x + (int)world->offest.x,
					(int)y + (int)world->offest.y,
					0xffffff);
		x += x_direction;
		y += y_direction;
		steps--;
		}
}

typedef enum e_side_hit
{
	horizontal,
	vertical
}			t_side_hit;

typedef struct s_cast_ray_vars
{
	int		cell_x;
	int		cell_y;
	double	ray_x_dir;
	double	ray_y_dir;
	double	delta_x_dist;
	double	delta_y_dist;
	int		next_x;
	int		next_y;
	double	dist_to_side_x;
	double	dist_to_side_y;
}				t_cast_ray_vars;

typedef struct s_ray_cast
{
	t_side_hit	side_hit;
	t_point		hit_pos;
	double		ray_len;
}				t_ray_cast;

t_ray_cast	find_hit_pos(t_cast_ray_vars *vars, t_point player, t_side_hit side_hit)
{
	t_ray_cast	ray_cast;

	if (side_hit == vertical)
		ray_cast.ray_len = vars->dist_to_side_x - vars->delta_x_dist;
	else
		ray_cast.ray_len = vars->dist_to_side_y - vars->delta_y_dist;
	ray_cast.hit_pos.x = player.x + vars->ray_x_dir * ray_cast.ray_len;
	ray_cast.hit_pos.y = player.y + vars->ray_y_dir * ray_cast.ray_len;
	ray_cast.side_hit = side_hit;
	return (ray_cast);
}

void	set_next_cell_pos(t_cast_ray_vars *vars)
{
	if (vars->ray_x_dir < 0)
		vars->next_x = -1;
	else
		vars->next_x = 1;
	if (vars->ray_y_dir < 0)
		vars->next_y = -1;
	else
		vars->next_y = 1;
}

void	set_dist_from_next_side(t_cast_ray_vars *vars, t_world *world)
{
	if (vars->ray_x_dir < 0)
		vars->dist_to_side_x = (world->player.x - vars->cell_x) * vars->delta_x_dist;
	else
		vars->dist_to_side_x = (vars->cell_x + 1 - world->player.x) * vars->delta_x_dist;
	if (vars->ray_y_dir < 0)
		vars->dist_to_side_y = (world->player.y - vars->cell_y) * vars->delta_y_dist;
	else
		vars->dist_to_side_y = (vars->cell_y + 1 - world->player.y) * vars->delta_y_dist;
}

t_ray_cast	find_next_wall(t_cast_ray_vars *vars, t_world *world)
{
	t_side_hit	side_hit;
	/* En ajoutant delta_dist à dist_to_size on obtient à chaque fois la distance entre notre joueur et la prochaine ligne franchie */
	while (1)
	{
		if (vars->dist_to_side_x <= vars->dist_to_side_y)
		{
			vars->cell_x += vars->next_x;
			side_hit = vertical;
			vars->dist_to_side_x += vars->delta_x_dist;
		}
		else
		{
			vars->cell_y += vars->next_y;
			side_hit = horizontal;
			vars->dist_to_side_y += vars->delta_y_dist;
		}
		if (world->map[vars->cell_y][vars->cell_x] == '1')
			break ;
	}
	return (find_hit_pos(vars, world->player, side_hit));
}

t_ray_cast	cast_ray(t_world *world, double angle, int cell_size)
{
	t_cast_ray_vars	vars;
	/* On calcule la case de départ */
	vars.cell_x = (int)world->player.x;
	vars.cell_y = (int)world->player.y;
	/* On détermine la direction du rayon en se basant sur l'angle */
	vars.ray_x_dir = cos(angle);
	vars.ray_y_dir = -sin(angle);
	/* FAUT PROTEGER SI JAMAIS ray_dir = 0 */
	/* On détermine le delta de la distance parcourrue par le rayon en x et en y */
	if (vars.ray_x_dir == 0)
		vars.delta_x_dist = INFINITY;
	else
		vars.delta_x_dist = fabs(1 / vars.ray_x_dir);
	if (vars.ray_y_dir == 0)
		vars.delta_y_dist = INFINITY;
	else
		vars.delta_y_dist = fabs(1 / vars.ray_y_dir);
	/* On détermine si la prochaine case traversée par le rayon est à droite/gauche/haut/bas (step),
	ainsi que la distance que sépare le joueur du bord x/y le plus proche en suivant la direction du rayon */
	set_next_cell_pos(&vars);
	set_dist_from_next_side(&vars, world);
	return (find_next_wall(&vars, world));
}

void	set_map_offset(t_world *world, t_minimap *minimap)
{
	world->offest.x = (RES_X / 2) - double_to_pixel(world->player.x, minimap->cell_size);
	world->offest.y = (RES_Y / 2) - double_to_pixel(world->player.y, minimap->cell_size);
}

void	draw_map(t_minimap *minimap, t_world *world) //TEST
{
	int	minimap_x;
	int	minimap_y;
	int	map_x;
	int	map_y;
	// static int drawed = 0;

	// if (drawed == 0)
	// {
		map_y = 0;
		minimap_y = 0;
		minimap_x = 0;
		while (map_y < world->map_y)
		{
			map_x = 0;
			while (map_x < world->map_x)
			{
				if (world->map[map_y][map_x] == '1')
					draw_square(minimap, &minimap_x, &minimap_y, 0x008000);
				else
					draw_square(minimap, &minimap_x, &minimap_y, 0x000000);
				map_x++;
			}
			minimap_y += minimap->cell_size - 1;
			map_y++;
		}
		draw_grid(minimap);
		// place_player(minimap, world);
		// drawed = 1;
	// }
}

typedef struct s_hook_args
{
	t_world			*world;
	t_x_elements	*x_elem;
}				t_hook_args;

typedef enum e_wall_sprite
{
	north,
	south,
	east,
	west
}			t_wall_sprite;

t_wall_sprite	chose_wall_sprite(t_world *w, t_ray_cast *ray)
{
	if (w->player.x < ray->hit_pos.x && w->player.y < ray->hit_pos.y
		&& ray->side_hit == vertical)
		return (east);
	else if (w->player.x < ray->hit_pos.x && w->player.y < ray->hit_pos.y
		&& ray->side_hit == horizontal)
		return (north);
	if (w->player.x > ray->hit_pos.x && w->player.y < ray->hit_pos.y
		&& ray->side_hit == vertical)
		return (west);
	else if (w->player.x > ray->hit_pos.x && w->player.y < ray->hit_pos.y
		&& ray->side_hit == horizontal)
		return (north);
	if (w->player.x < ray->hit_pos.x && w->player.y > ray->hit_pos.y
		&& ray->side_hit == vertical)
		return (east);
	else if (w->player.x < ray->hit_pos.x && w->player.y > ray->hit_pos.y
		&& ray->side_hit == horizontal)
		return (south);
	if (w->player.x > ray->hit_pos.x && w->player.y > ray->hit_pos.y
		&& ray->side_hit == vertical)
		return (west);
	else
		return (south);
}


typedef enum e_camera_direction
{
	up,
	right,
	down,
	left
}			t_camera_direction;

t_camera_direction	set_camera_direction(int orientation)
{
	if (orientation < RES_X)
		return (up);
	if (orientation < RES_X * 2)
		return(right);
	if (orientation < RES_X * 3)
		return (down);
	else
		return (left);

}

t_point	set_left_camera_limit(int orientation, t_camera_direction direction)
{
	t_point camera_limit;

	if (direction == up)
	{
		camera_limit.x = 0;
		camera_limit.y = orientation;
	}
	else if (direction == right)
	{
		camera_limit.x = orientation - RES_X;
		camera_limit.y = RES_X;
	}
	else if (direction == down)
	{
		camera_limit.x = RES_X;
		camera_limit.y = RES_X - (orientation - (RES_X * 2));
	}
	else
	{
		camera_limit.x = RES_X - (orientation - (RES_X * 3));
		camera_limit.y = 0;
	}
	return (camera_limit);
}

double	set_ray_angle(t_point camera_limit)
{
	return (atan2(camera_limit.y - (RES_X / 2), camera_limit.x - (RES_X / 2)));
}

void	capture_scene(t_world *world, t_x_elements *x_elements)
{
	int					i;
	t_camera_direction	direction;
	t_point				camera_limit;
	double				ray_angle;
	int					temp_orientation;

	temp_orientation = world->orientation;
	i = 0;
	while (i <= RES_X)
	{
		if (temp_orientation == RES_X * 4)
		temp_orientation = 0;
		direction = set_camera_direction(temp_orientation);
		while (i <= RES_X)
		{
			camera_limit = set_left_camera_limit(temp_orientation, direction);
			ray_angle = set_ray_angle(camera_limit);
			printf("x: %f y: %f | direction: %u | angle: %f\n", camera_limit.x, camera_limit.y, direction, ray_angle);
			draw_line(world->player, cast_ray(world, ray_angle, x_elements->minimap.cell_size).hit_pos, world, x_elements);
			i++;
			temp_orientation++;
			if ((temp_orientation) % RES_X == 0)
				break ;
		}
	}
}

int	update_minimap(t_hook_args *args) //TEST
{
	// static int x = 0;
	// static int y = 0;

	// set_map_offset(args->world, args->x_elem->minimap);
	// place_player(args->x_elem->minimap, args->world);
	// erase_player(args->x_elem->minimap, args->world);
	// place_player(args->x_elem->minimap, args->world);
	// draw_map(args->x_elem->minimap, args->world);
	if (args->x_elem->refresh)
	{
		set_map_offset(args->world, &args->x_elem->minimap);
		mlx_clear_window(args->x_elem->display, args->x_elem->win);
		mlx_put_image_to_window(args->x_elem->display,
			args->x_elem->win,
			args->x_elem->minimap.img,
			args->world->offest.x,
			args->world->offest.y);
		mlx_pixel_put(args->x_elem->display, args->x_elem->win, RES_X / 2 + 1, RES_Y / 2, 0xff0000);
		mlx_pixel_put(args->x_elem->display, args->x_elem->win, RES_X / 2 - 1, RES_Y / 2, 0xff0000);
		mlx_pixel_put(args->x_elem->display, args->x_elem->win, RES_X / 2, RES_Y / 2 + 1, 0xff0000);
		mlx_pixel_put(args->x_elem->display, args->x_elem->win, RES_X / 2, RES_Y / 2 - 1, 0xff0000);
		mlx_pixel_put(args->x_elem->display, args->x_elem->win, RES_X / 2, RES_Y / 2, 0xff0000);
		capture_scene(args->world, args->x_elem);
		args->x_elem->refresh = 0;
	}

	// printf("Wall hit at: %f, %f\n", hit.x, hit.y);
	// t_point hit = cast_ray(args->world, (2 / 3.14159265358979323846), args->x_elem->minimap->cell_size);
	// draw_line(args->world->player, hit, args->world, args->x_elem);
	// hit = cast_ray(args->world, (4 / 3.14159265358979323846), args->x_elem->minimap->cell_size);
	// draw_line(args->world->player, hit, args->world, args->x_elem);
	// draw_line(args->world->player, (t_point){1, 1}, args->world, args->x_elem);
	// draw_line(args->world->player, (t_point){2, 1}, args->world, args->x_elem);
	// draw_line(args->world->player, (t_point){3, 1}, args->world, args->x_elem);
	// usleep(10000);
	// x++;
	// y++;
	return (SUCCESS);
}

int	key_hook(int keycode, t_hook_args *args) //TEST
{
	if (keycode == XK_Right)
	{
		// erase_player(args->x_elem->minimap, args->world);
		// args->world->offest_x -= 0.1;
		args->world->player.x += 0.3;
	}
	if (keycode == XK_Left)
	{
		// erase_player(args->x_elem->minimap, args->world);
		// args->world->offest_x += 0.1;
		args->world->player.x -= 0.3;
	}
	if (keycode == XK_Up)
	{
		// erase_player(args->x_elem->minimap, args->world);
		// args->world->offest_y += 0.1;
		args->world->player.y -= 0.3;
	}
	if (keycode == XK_Down)
	{
		// erase_player(args->x_elem->minimap, args->world);
		// args->world->offest_y -= 0.1;
		args->world->player.y += 0.3;
	}
	if (keycode == XK_a)
	{
		if (args->world->orientation == 0)
			args->world->orientation = (RES_X * 4) - 1;
		else
			args->world->orientation -= (RES_X / 10);
	}
	if (keycode == XK_d)
	{
		if (args->world->orientation == (RES_X * 4) - 1)
			args->world->orientation = 0;
		else
			args->world->orientation += (RES_X / 10);
	}
	printf("orientation: %d\n", args->world->orientation);
	args->x_elem->refresh = 1;
	return (1);
}

int	main(void) //TEST
{
	t_x_elements	x_elem;
	t_world			world;
	t_hook_args		hook_args;

	x_elem = (t_x_elements){NULL};
	world = (t_world){NULL};
	init_world(&world);
	x_init(&x_elem, &world);

	set_map_offset(&world, &x_elem.minimap);

	hook_args.world = &world;
	hook_args.x_elem = &x_elem;
	draw_map(&x_elem.minimap, &world);
	mlx_key_hook(x_elem.win, key_hook, &hook_args);
	// capture_scene(&world, &x_elem);
	mlx_loop_hook(x_elem.display, update_minimap, &hook_args);
	mlx_loop(x_elem.display);
	// print_map(world.map);
}
