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

int PsMatrix::current_color = 0;
int PsMatrix::default_w = 600, PsMatrix::default_h = 600, PsMatrix::minimum_dim = 200;

/*
** PsMatrix uses the default constructor of PsShape, so without a container.
*/
PsMatrix::PsMatrix()
{
    this->color = (current_color++) % MATRIX_COUNT_COLOR;
    this->div_x = 0;
    this->div_y = 0;
    this->div_is_active = false;
}

/*
** When a matrix is destroyed, all the images it contained are deleted.
*/
PsMatrix::~PsMatrix()
{
    ImageList::iterator i;

    for (i = this->images.begin(); i != this->images.end(); ++i)
        delete* i;
}

/*
** Changes the color of the matrix (for "highlight" mode).
*/
void PsMatrix::DoChangeColor()
{
    this->color = (current_color++) % MATRIX_COUNT_COLOR;
}

/*
** Resets all transformations applied to the matrix, except the size (a matrix does not have an "initial size" like an image, so it is completely arbitrary). Currently, it simply makes it square by averaging its height and width, but this is no more justified than any other operation.
*/
void PsMatrix::DoResetAll()
{
    float mean = (this->h + this->w) / 2;

    this->h = mean;
    this->i = 0;
    this->j = 0;
    this->r = 0;
    this->w = mean;
}

/*
** Retrieves the color of the matrix (for "highlight" mode).
*/
void PsMatrix::GetColor(float& r, float& g, float& b) const
{
    static float rgb[][3] =
    {
        {0.75f, 0.75f, 1.00f},
        {1.00f, 0.75f, 0.75f},
        {0.75f, 1.00f, 0.75f},
        {1.00f, 1.00f, 0.50f},
        {1.00f, 0.50f, 1.00f},
        {0.50f, 1.00f, 1.00f},
    };

    r = rgb[this->color][0];
    g = rgb[this->color][1];
    b = rgb[this->color][2];
}

/*
** Retrieves the position of a matrix. It might be useful to record these coordinates relatively rather than absolutely, to easily test when a matrix goes out of the document?
*/
void PsMatrix::GetPosition(float& x, float& y) const
{
    x = this->x;
    y = this->y;
}

/*
** Changes the rotation angle of a matrix, with possible constraint to PI / 8. If the "reflect" mode is active, this rotation is reflected on all images contained in the matrix.
*/
void PsMatrix::SetAngle(float r, bool constrain, bool reflect)
{
    ImageList::iterator image;
    float angle;

    if (constrain)
        r = (int)(r / (PS_MATH_PI / 8)) * (PS_MATH_PI / 8);

    if (reflect)
    {
        angle = r - this->r;

        for (image = this->images.begin(); image != this->images.end(); ++image)
            (*image)->SetAngle((*image)->GetAngle() + angle, false, reflect);
    }

    this->r = r;
}

/*
** Changes the position of the matrix, and places it at x, y.
** FIXME: Prevent the matrix from going out of the document. The problem is that at this level of the code, we cannot access the size of the document; it is always possible to get around this by going through PsController, but it would seem much cleaner to have relative coordinates for the position (see function "GetPosition"), and thus simply test if x < -SHAPE_SIZE, x > SHAPE_SIZE, y < -SHAPE_SIZE, y > SHAPE_SIZE.
*/
void PsMatrix::SetPosition(float x, float y, bool)
{
    PsShape::FinalizePosition(x, y);
}

/*
** Changes the size of a matrix, with possible constraint on the ratios (the parameters old_w and old_h are used to know the ratio of the dimensions of the matrix before this transformation). See the function "PsShape::FinalizeSize" regarding the parameters inv_x and inv_y.
*/
void PsMatrix::SetSize(float w, float h, float inv_x, float inv_y, float old_w, float old_h, bool constrain, bool reflect)
{
    ImageList::iterator image;
    float ratio_h;
    float ratio_w;

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

        for (image = this->images.begin(); image != this->images.end(); ++image)
        {
            float angle;
            float cos_a;
            float sin_a;

            angle = this->r - (*image)->r;
            cos_a = abs(cos(angle));
            sin_a = abs(sin(angle));

            (*image)->SetSize((*image)->w * (ratio_w * cos_a + ratio_h * (1 - cos_a)), (*image)->h * (ratio_w * sin_a + ratio_h * (1 - sin_a)), constrain, reflect);
        }
    }

    PsShape::FinalizeSize(w, h, inv_x, inv_y);
}

/*
** Changes the torsion of the matrix, with possible constraint on the step.
*/
void PsMatrix::SetTorsion(float i, float j, bool constrain)
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
** Retrieves the torsion of the matrix.
*/
void PsMatrix::GetTorsion(float& a, float& b) const
{
    a = this->i;
    b = this->j;
}

/*
** Indicates if the matrix is subject to torsions.
*/
bool PsMatrix::HasTorsion()
{
    return this->i != 0 || this->j != 0;
}
