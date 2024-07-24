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
#include "PsShape.h"
#include "PsMaths.h"

/*
** Constructor for a PsShape contained within another.
*/
PsShape::PsShape(const PsShape& parent) :
    parent(&parent),
    h(200),
    i(0),
    j(0),
    r(0),
    w(200),
    x(0),
    y(0),
    hide(false),
    constraint(false)
{
}

/*
** Constructor for an autonomous PsShape.
*/
PsShape::PsShape() :
    parent(0),
    h(0),
    i(0),
    j(0),
    r(0),
    w(0),
    x(0),
    y(0),
    hide(false),
    constraint(false)
{
}

PsShape::~PsShape()
{
}

/*
** Retrieves the rotation angle of the PsShape.
*/
float    PsShape::GetAngle() const
{
    return this->r;
}

/*
** Retrieves the name of this PsShape (technically unnecessary, but this name will be displayed
** in dialog boxes for the user).
*/
const std::string& PsShape::GetName() const
{
    return this->name;
}

/*
** Changes the position of the PsShape. This method, which should be called "SetPosition", has the prefix
** "Finalize", indicating that another process will be necessary by the class inheriting from PsShape,
** before calling this method, and that its call alone is therefore not sufficient.
*/
void    PsShape::FinalizePosition(float x, float y)
{
    this->x = x;
    this->y = y;
}

/*
** Changes the size of the PsShape, in width (w) and height (h). The point "inv_x, inv_y" is
** the "invariant point": the one that should not move during resizing. When
** the user resizes a PsShape, this point is the handle opposite to the one that is
** currently being dragged. If inv_x and inv_y remain at 0, the PsShape simply changes size
** without compensating for any displacement.
*/
void        PsShape::FinalizeSize(float w, float h, float inv_x, float inv_y)
{
    float    inv_x1;
    float    inv_x2;
    float    inv_y1;
    float    inv_y2;

    this->ToAbsolute(inv_x, inv_y, inv_x1, inv_y1);

    this->h = h;
    this->w = w;

    if (inv_x || inv_y)
    {
        this->ToAbsolute(inv_x, inv_y, inv_x2, inv_y2);

        inv_x2 -= inv_x1;
        inv_y2 -= inv_y1;

        this->GetPosition(inv_x1, inv_y1);
        this->SetPosition(inv_x1 + inv_x2, inv_y1 + inv_y2);
    }
}

/*
** Tests if a point (in absolute terms, relative to the document and not the window, see function
** PsRender::Convert) is in a PsShape or not.
*/
bool        PsShape::InContent(float ax, float ay) const
{
    float    rx, ry;

    this->ToRelative(ax, ay, rx, ry);

    return rx >= -SHAPE_SIZE && rx <= SHAPE_SIZE && ry >= -SHAPE_SIZE && ry <= SHAPE_SIZE;
}

/*
** Tests if a point is on one of the resizing handles of a PsShape, and records
** in "i" the index of the handle (0 = top/left, etc. in a clockwise direction). This function must
** take the zoom into account, so that the size of the handles is not affected by it
** (we must therefore be able to cancel its effects).
*/
bool        PsShape::InResize(float px, float py, float zoom, int& i) const
{
    float    size = SHAPE_SIZE_RESIZE * zoom;
    float    corner[4][2];

    this->ToAbsolute(-SHAPE_SIZE, -SHAPE_SIZE, corner[0][0], corner[0][1]);
    this->ToAbsolute(SHAPE_SIZE, -SHAPE_SIZE, corner[1][0], corner[1][1]);
    this->ToAbsolute(-SHAPE_SIZE, SHAPE_SIZE, corner[2][0], corner[2][1]);
    this->ToAbsolute(SHAPE_SIZE, SHAPE_SIZE, corner[3][0], corner[3][1]);

    for (i = 0; i < 4; ++i)
        if (px >= corner[i][0] - size && px <= corner[i][0] + size && py >= corner[i][1] - size && py <= corner[i][1] + size)
            return true;

    return false;
}

/*
** Tests if a point is at one of the rotation locations; same remarks as for
** resizing.
*/
bool        PsShape::InRotate(float px, float py, float zoom, int& i) const
{
    float    size = SHAPE_SIZE_ROTATE * zoom;
    float    corner[4][2];
    float    angle;
    float    ax;
    float    ay;

    this->ToAbsolute(-SHAPE_SIZE, -SHAPE_SIZE, corner[0][0], corner[0][1]);
    this->ToAbsolute(SHAPE_SIZE, -SHAPE_SIZE, corner[1][0], corner[1][1]);
    this->ToAbsolute(-SHAPE_SIZE, SHAPE_SIZE, corner[2][0], corner[2][1]);
    this->ToAbsolute(SHAPE_SIZE, SHAPE_SIZE, corner[3][0], corner[3][1]);

    for (i = 0; i < 4; ++i)
    {
        angle = this->ToAngle(corner[i][0], corner[i][1]);

        ax = size * cos(angle) - size * sin(angle);
        ay = size * sin(angle) + size * cos(angle);

        if (px >= corner[i][0] + ax - size && px <= corner[i][0] + ax + size && py >= corner[i][1] + ay - size && py <= corner[i][1] + ay + size)
            return true;
    }

    return false;
}

/*
** Tests if a point is on one of the torsion handles, same remarks
** as for the two previous functions.
*/
bool    PsShape::InTorsion(float px, float py, float zoom, int& i) const
{
    float    size = SHAPE_SIZE_TORSIO * zoom;
    float    corner[4][2];

    this->ToAbsolute(0, -SHAPE_SIZE, corner[0][0], corner[0][1]);
    this->ToAbsolute(SHAPE_SIZE, 0, corner[1][0], corner[1][1]);
    this->ToAbsolute(0, SHAPE_SIZE, corner[2][0], corner[2][1]);
    this->ToAbsolute(-SHAPE_SIZE, 0, corner[3][0], corner[3][1]);

    for (i = 0; i < 4; ++i)
        if (px >= corner[i][0] - size && px <= corner[i][0] + size && py >= corner[i][1] - size && py <= corner[i][1] + size)
            return true;

    return false;
}

/*
** Changes the name of the PsShape (see GetName).
*/
void    PsShape::SetName(const std::string& name)
{
    this->name = name;
}

/*
** Transforms the relative coordinates of the PsShape into absolute coordinates. Relative coordinates
** allow much simpler calculations within a PsShape (its top/left corner is always at -SHAPE_SIZE, -SHAPE_SIZE
** regardless of the transformations applied, for example). Absolute coordinates take these transformations into account.
*/
void        PsShape::ToAbsolute(float rx, float ry, float& ax, float& ay) const
{
    float    sx;
    float    sy;
    float    tx;
    float    ty;

    tx = rx * this->w / (SHAPE_SIZE * 2) + this->i * ry / (SHAPE_SIZE * 2);
    ty = ry * this->h / (SHAPE_SIZE * 2) + this->j * rx / (SHAPE_SIZE * 2);

    if (this->parent)
        this->parent->ToAbsolute(this->x, this->y, sx, sy);
    else
    {
        sx = this->x;
        sy = this->y;
    }

    ax = tx * cos(this->r) - ty * sin(this->r) + sx;
    ay = tx * sin(this->r) + ty * cos(this->r) + sy;
}

/*
** Returns the angle between the center of the PsShape and the point "ax, ay", in radians.
*/
float        PsShape::ToAngle(float ax, float ay) const
{
    float    tx;
    float    ty;

    if (this->parent)
        this->parent->ToAbsolute(this->x, this->y, tx, ty);
    else
    {
        tx = this->x;
        ty = this->y;
    }

    if (ax < tx)
        return (float)atan((ay - ty) / (ax - tx)) + PS_MATH_PI;
    else if (ax > tx)
        return (float)atan((ay - ty) / (ax - tx));
    else
        return (ay < ty ? -PS_MATH_PI : PS_MATH_PI) / 2.0f;
}

/*
** Conversion of absolute coordinates (therefore at the document level) to coordinates relative
** to the PsShape (see ToAbsolute).
*/
void        PsShape::ToRelative(float ax, float ay, float& rx, float& ry) const
{
    float    sx;
    float    sy;
    float    tx;
    float    ty;

    if (this->parent)
        this->parent->ToAbsolute(this->x, this->y, sx, sy);
    else
    {
        sx = this->x;
        sy = this->y;
    }

    sx = ax - sx;
    sy = ay - sy;

    tx = (sx * cos(-this->r) - sy * sin(-this->r));
    ty = (sx * sin(-this->r) + sy * cos(-this->r));

    rx = (tx - this->i * ty / this->h) / (this->w - this->j / this->h) * (SHAPE_SIZE * 2);
    ry = (ty - this->j * tx / this->w) / (this->h - this->i / this->w) * (SHAPE_SIZE * 2);
}
