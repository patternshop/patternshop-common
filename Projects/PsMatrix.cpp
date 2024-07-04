/**
 * This file is part of Patternshop Project.
 *
 * Patternshop is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Patternshop is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Patternshop.  If not, see <http://www.gnu.org/licenses/>
*/
#include "PsMatrix.h"
#include "PsMaths.h"

int	PsMatrix::current_color = 0;
int	PsMatrix::default_w = 600, PsMatrix::default_h = 600, PsMatrix::minimum_dim = 200;

/*
** PsMatrix utilise le constructeur par défaut d'PsShape, donc sans conteneur.
*/
PsMatrix::PsMatrix()
{
	color = (current_color++) % MATRIX_COUNT_COLOR;
	div_x = 0;
	div_y = 0;
	div_is_active = false;
}

/*
** À la destruction d'une matrice, on efface toutes les images qu'elle contenait.
*/
PsMatrix::~PsMatrix()
{
	ImageList::iterator	i;

	for (i = images.begin(); i != images.end(); ++i)
		delete* i;
}

/*
** Change la couleur de la matrice (pour le mode "highlight").
*/
void	PsMatrix::DoChangeColor()
{
	this->color = (current_color++) % MATRIX_COUNT_COLOR;
}

/*
** Annule toutes les transformations apportées à la matrice, sauf la taille (une matrice n'a pas de "taille
** initiale" comme une image, c'est donc totalement arbitraire). Actuellement, ça se contente de la rendre
** carrée en faisant la moyenne de sa hauteur et de sa largeur, mais ce n'est pas plus justifié que n'importe
** quelle operation.
*/
void		PsMatrix::DoResetAll()
{
	float	mean = (this->h + this->w) / 2;

	this->h = mean;
	this->i = 0;
	this->j = 0;
	this->r = 0;
	this->w = mean;
}

/*
** Charge les données d'une matrice (ceci comprend toutes les images qu'elle contient) depuis un fichier.
*/
ErrID		PsMatrix::FileLoad(FILE* file)
{
	PsImage* image;
	ErrID	err;
	size_t	n;

	if (fread(&color, sizeof(color), 1, file) != 1)
		return ERROR_FILE_READ;

	if (fread(&n, sizeof(n), 1, file) != 1)
		return ERROR_FILE_READ;

	while (n--)
	{
		images.push_back((image = new PsImage(this)));

		if ((err = image->FileLoad(file)) != ERROR_NONE)
			return err;
	}

	if (fread(&div_is_active, sizeof(div_is_active), 1, file) != 1)
		return ERROR_FILE_READ;

	if (fread(&div_x, sizeof(div_x), 1, file) != 1)
		return ERROR_FILE_READ;

	if (fread(&div_y, sizeof(div_y), 1, file) != 1)
		return ERROR_FILE_READ;

	return PsShape::FileLoad(file);
}

/*
** Sauvegarde toutes les données d'une matrice dans un fichier.
*/
ErrID									PsMatrix::FileSave(FILE* file) const
{
	ImageList::const_iterator	i;
	ErrID								err;
	size_t								n;

	if (fwrite(&color, sizeof(color), 1, file) != 1)
		return ERROR_FILE_WRITE;

	n = images.size();

	if (fwrite(&n, sizeof(n), 1, file) != 1)
		return ERROR_FILE_WRITE;

	for (i = images.begin(); i != images.end(); ++i)
		if ((err = (*i)->FileSave(file)) != ERROR_NONE)
			return err;

	if (fwrite(&div_is_active, sizeof(div_is_active), 1, file) != 1)
		return ERROR_FILE_WRITE;

	if (fwrite(&div_x, sizeof(div_x), 1, file) != 1)
		return ERROR_FILE_WRITE;

	if (fwrite(&div_y, sizeof(div_y), 1, file) != 1)
		return ERROR_FILE_WRITE;

	return PsShape::FileSave(file);
}

/*
** Récuere la couleur de la matrice (pour le mode "highlight").
*/
void				PsMatrix::GetColor(float& r, float& g, float& b) const
{
	static float	rgb[][3] =
	{
		{0.75f, 0.75f, 1.00f},
		{1.00f, 0.75f, 0.75f},
		{0.75f, 1.00f, 0.75f},
		{1.00f, 1.00f, 0.50f},
		{1.00f, 0.50f, 1.00f},
		{0.50f, 1.00f, 1.00f},
	};

	r = rgb[color][0];
	g = rgb[color][1];
	b = rgb[color][2];
}

/*
** Récupere la position d'une matrice. Il serait peut-être utile d'enregistrer ces coordonnées
** en relatif plutot qu'en absolu, pour tester quand une matrice sort du document facilement ?
*/
void	PsMatrix::GetPosition(float& x, float& y) const
{
	x = this->x;
	y = this->y;
}

/*
** Change l'angle de rotation d'une matrice, avec éventuelle contrainte à PI / 8. Si le mode
** "reflect" est actif, cette rotation est répercutée sur toutes les images contenues dans
** la matrice.
*/
void  PsMatrix::SetAngle(float r, bool constrain, bool reflect)
{
	ImageList::iterator	image;
	float				angle;

	if (constrain)
		r = (int)(r / (PS_MATH_PI / 8)) * (PS_MATH_PI / 8);

	if (reflect)
	{
		angle = r - this->r;

		for (image = images.begin(); image != images.end(); ++image)
			(*image)->SetAngle((*image)->GetAngle() + angle, false, reflect);
	}

	this->r = r;
}

/*
** Change la position de la matrice, et la place à x, y.
** FIXME : Empêcher la matrice de sortir du document. Le problème est qu'à ce niveau du code,
** on ne peut pas acceder à la taille du document; il est toujours possible de s'en sortir en
** passant par PsController, mais cela me semblerait bien plus propre d'avoir des coordonnées
** relatives pour la position (voir fonction "GetPosition"), et ainsi de tester simplement si
** x < -SHAPE_SIZE, x > SHAPE_SIZE, y < -SHAPE_SIZE, y > SHAPE_SIZE.
*/
void	PsMatrix::SetPosition(float x, float y, bool)
{
	PsShape::FinalizePosition(x, y);
}

/*
** Change la taille d'une matrice, avec éventuelle contrainte sur les ratios (les paramètres
** old_w et old_h servent justement à connaitre le ratio des dimentions de la matrice avant
** cette transformation). Voir la fonction "PsShape::FinalizeSize" à propos des paramètres inv_x et inv_y.
*/
void					PsMatrix::SetSize(float w, float h, float inv_x, float inv_y, float old_w, float old_h, bool constrain, bool reflect)
{
	ImageList::iterator	image;
	float				ratio_h;
	float				ratio_w;

	if (h < minimum_dim)
		h = (float)minimum_dim;
	if (w < minimum_dim)
		w = (float)minimum_dim;

	if ((constrain || this->constraint) && old_w && old_h)
	{
		ratio_h = h / old_h;
		ratio_w = w / old_w;

		h = old_h * (ratio_h + ratio_w) / 2;
		w = old_w * (ratio_h + ratio_w) / 2;
	}

	if (reflect)
	{
		ratio_h = h / this->h;
		ratio_w = w / this->w;

		for (image = images.begin(); image != images.end(); ++image)
		{
			float	angle;
			float	cos_a;
			float	sin_a;

			angle = r - (*image)->r;
			cos_a = abs(cos(angle));
			sin_a = abs(sin(angle));

			(*image)->SetSize((*image)->w * (ratio_w * cos_a + ratio_h * (1 - cos_a)), (*image)->h * (ratio_w * sin_a + ratio_h * (1 - sin_a)), constrain, reflect);
		}
	}

	PsShape::FinalizeSize(w, h, inv_x, inv_y);
}

/*
** Change la torsion de la matrice, avec éventuelle contrainte sur le pas.
*/
void	PsMatrix::SetTorsion(float i, float j, bool constrain)
{
	if (constrain)
	{
		i = (int)(i / MATRIX_ROUND_TORSIO) * MATRIX_ROUND_TORSIO;
		j = (int)(j / MATRIX_ROUND_TORSIO) * MATRIX_ROUND_TORSIO;
	}

	this->i = i;
	this->j = j;
}

/*
** Récuper la torsion de la matrice.
*/
void	PsMatrix::GetTorsion(float& a, float& b) const
{
	a = i;
	b = j;
}

/*
** Indique si la matrice est soumise à des torsions
*/
bool PsMatrix::HasTorsion()
{
	return i != 0 || j != 0;
}
