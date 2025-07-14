/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ray_cast.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ecasalin <ecasalin@42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/14 09:13:31 by ecasalin          #+#    #+#             */
/*   Updated: 2025/07/14 09:16:05 by ecasalin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

typedef struct s_point
{
	float	x;
	float	y;
}				t_point;

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

t_point	find_hit_pos(t_cast_ray_vars *vars, t_point player, t_side_hit side_hit)
{
	t_point	hit_pos;
	double	wall_hit_dist;

	if (side_hit == vertical)
		wall_hit_dist = vars->dist_to_side_x - vars->delta_x_dist;
	else
		wall_hit_dist = vars->dist_to_side_y - vars->delta_y_dist;
	hit_pos.x = player.x + vars->ray_x_dir * wall_hit_dist;
	hit_pos.y = player.y + vars->ray_y_dir * wall_hit_dist;
	return (hit_pos);
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
		vars->dist_to_side_x = (vars->cell_x + 1.0 - world->player.x) * vars->delta_x_dist;
	if (vars->ray_y_dir < 0)
		vars->dist_to_side_y = (world->player.y - vars->cell_y) * vars->delta_y_dist;
	else
		vars->dist_to_side_y = (vars->cell_y + 1.0 - world->player.y) * vars->delta_y_dist;
}

t_point	find_next_wall(t_cast_ray_vars *vars, t_world *world)
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

t_point	cast_ray(t_world *world, double angle, int cell_size)
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
	vars.delta_x_dist = fabs(1 / vars.ray_x_dir);
	vars.delta_y_dist = fabs(1 / vars.ray_y_dir);
	/* On détermine si la prochaine case traversée par le rayon est à droite/gauche/haut/bas (step),
	ainsi que la distance que sépare le joueur du bord x/y le plus proche en suivant la direction du rayon */
	set_next_cell_pos(&vars);
	set_dist_from_next_side(&vars, world);
	return (find_next_wall(&vars, world));
}
