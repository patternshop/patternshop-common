// PsProject.cpp

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

#include <stdio.h>
#include "PsProjectController.h"
#include "PsController.h"
#include "PsAction.h"
#include "PsMatrix.h"
#include "PsPattern.h"
#include "PsSoftRender.h"
#include "PsWinPropertiesCx.h"

extern PsWinPropertiesCx* dlgPropreties;

/*
** Triggers the display of the project in the active context, using hardware rendering (thus OpenGL).
*/
void PsProjectController::RenderToScreen()
{
    int x;
    int y;

    if (this->center)
    {
        this->renderer.CenterView();
        this->center = false;
    }

    this->renderer.GetSize(x, y);
    this->renderer.SetEngine(PsRender::ENGINE_HARDWARE);
    this->renderer.Render(*this, x, y);

#ifdef _WINDOWS
    int m_glErrorCode = glGetError();
    if (m_glErrorCode != GL_NO_ERROR)
    {
        const GLubyte* estring;
        CString mexstr;
        estring = gluErrorString(m_glErrorCode);
        mexstr.Format("RenderToScreen:\n\tAn OpenGL error occurred: %s\n", estring);
        AfxMessageBox(mexstr, MB_OK | MB_ICONEXCLAMATION);
        TRACE0(mexstr);
    }
#endif /* _WINDOWS */
}

/*
** Triggers rendering to a file
*/
bool PsProjectController::RenderToFile(const char* filename, int x, int y)
{
    PsController::Instance().SetProgress(-1);

    this->renderer.SetZone((float)this->GetWidth(), (float)this->GetHeight());

    InitSoftwareFile(x, y);

    PsController::Instance().SetProgress(5);

    this->renderer.SetEngine(PsRender::ENGINE_SOFTWARE);
    this->renderer.Render(*this, x, y);
    this->renderer.Recalc();

    PsController::Instance().SetProgress(90);

    flushSoftwareFile(filename, this->bHideColor);

    PsController::Instance().SetProgress(-2);

    return true;
}

/*
** Selects an image in the project (and the matrix that contains it).
*/
void PsProjectController::SelectImage(PsImage* image)
{
    this->matrix = image ? image->GetParent() : 0;
    this->image = image;
    this->shape = image;

    PsController::Instance().UpdateWindow();
    PsController::Instance().UpdateDialogProject();

    if (dlgPropreties)
    {
        //dlgPropreties->FocusImageInformation();
        dlgPropreties->UpdateInformation(this);
    }
}

/*
** Selects a matrix in the project.
*/
void PsProjectController::SelectMatrix(PsMatrix* matrix)
{
    this->matrix = matrix;
    this->image = 0;
    this->shape = matrix;

    PsController::Instance().UpdateWindow();
    PsController::Instance().UpdateDialogProject();

    if (dlgPropreties)
    {
        //dlgPropreties->FocusMatrixInformation();
        dlgPropreties->UpdateInformation(this);
    }
}

/*
** Movement of the "magnify" tool.
*/
void PsProjectController::ToolMagnifyDrag(int y, int old_x, int old_y)
{
    float zoom = this->prev_zoom + (y - old_y) * ZOOM_COEF;

    if (y > old_y)
        PsController::Instance().SetCursor(CURSOR_MAGNIFY2);
    else
        PsController::Instance().SetCursor(CURSOR_MAGNIFY3);

    /* static float fx, fy, ox, oy;
    if (prev_zoom == renderer.zoom)
    {
        renderer.Convert(old_x, old_y, fx, fy);
        ox = renderer.scroll_x;
        oy = renderer.scroll_y;
    } */

    this->renderer.SetZoom(zoom);

    PsController::Instance().UpdateWindow();
    PsController::Instance().UpdateDialogOverview();
}

/*
** Initialization of the "magnify" tool.
*/
PsController::Tool PsProjectController::ToolMagnifyStart()
{
    this->prev_zoom = this->renderer.zoom;

    return PsController::TOOL_MAGNIFY_ZOOM;
}

/*
** Movement of the "Modify" tool (which needs to know in which mode it is, to know the operation to perform among movement, rotation, resizing...)
*/
void PsProjectController::ToolModifyMove(int x, int y, int old_x, int old_y, PsController::Tool tool)
{
    float fx;
    float fy;
    float sx;
    float sy;
    float tx;
    float ty;
    bool constrain = PsController::Instance().GetOption(PsController::OPTION_CONSTRAIN);
    bool reflect = PsController::Instance().GetOption(PsController::OPTION_REFLECT);

    this->renderer.Convert(x, y, fx, fy);
    this->renderer.Convert(old_x, old_y, sx, sy);

    if (this->bPatternsIsSelected && !this->pattern->hide)
    {
        PsLayer* layer = this->pattern->aLayers[this->iLayerId];

        switch (tool)
        {
        case PsController::TOOL_MODIFY_ROTATE:
        {
            if (this->LogMustAdd())
                this->LogAdd(new LogPatternRotate(*this));

            PsController::Instance().SetCursor((PsCursor)(CURSOR_ROTATE1 + (int)((layer->ToAngle(fx, fy) + PS_MATH_PI / 2) * 2 / PS_MATH_PI) % 4));
            layer->rRotation.Yaw = this->prev_r + PsRotator::ToDegree(-(layer->ToAngle(fx, fy) - layer->ToAngle(sx, sy)));
            break;
        }

        case PsController::TOOL_MODIFY_MOVE:
        {
            if (this->LogMustAdd())
                this->LogAdd(new LogPatternTranslate(*this));

            PsVector vTranslation(fx - sx, fy - sy, 0.f);

            PsLayer* layer = this->pattern->aLayers[this->iLayerId];

            PsVector vEye = this->renderer.GetEyeLocation();

            float fDistance = (this->prev_origin - vEye).Size();
            float fDistanceZ0 = (this->prev_origin_z0 - vEye).Size();

            vTranslation = vTranslation * (fDistance / fDistanceZ0);

            layer->vTranslation.X = this->prev_x + vTranslation.X;
            layer->vTranslation.Y = this->prev_y + vTranslation.Y;

            break;
        }

        case PsController::TOOL_MODIFY_SIZE:
        {
            // FIXME : LOG !!

            tx = (fx - sx) * cos(-this->shape->GetAngle()) - (fy - sy) * sin(-this->shape->GetAngle());
            ty = (fx - sx) * sin(-this->shape->GetAngle()) + (fy - sy) * cos(-this->shape->GetAngle());

            break;
        }

        }
    }
    else
    {

        if (!this->shape || this->shape->hide)
            return;

        switch (tool)
        {
        case PsController::TOOL_MODIFY_MOVE:
        {
            if (this->LogMustAdd())
                this->LogAdd(new LogMove(*this, this->shape, this->prev_x, this->prev_y));

            this->shape->SetPosition(this->prev_x + fx - sx, this->prev_y + fy - sy, constrain);
            break;
        }

        case PsController::TOOL_MODIFY_ROTATE:
        {
            if (this->LogMustAdd())
                this->LogAdd(new LogRotate(*this, this->shape, this->prev_r, reflect));

            PsController::Instance().SetCursor((PsCursor)(CURSOR_ROTATE1 + (int)((this->shape->ToAngle(fx, fy) + PS_MATH_PI / 2) * 2 / PS_MATH_PI) % 4));

            this->shape->SetAngle(this->prev_r + this->shape->ToAngle(fx, fy) - this->shape->ToAngle(sx, sy), constrain, reflect);
            break;
        }

        case PsController::TOOL_MODIFY_SIZE:
        {
            if (this->LogMustAdd())
                this->LogAdd(new LogResize(*this, this->shape, this->prev_x, this->prev_y, this->prev_w, this->prev_h, reflect));

            tx = (fx - sx) * cos(-this->shape->GetAngle()) - (fy - sy) * sin(-this->shape->GetAngle());
            ty = (fx - sx) * sin(-this->shape->GetAngle()) + (fy - sy) * cos(-this->shape->GetAngle());

            if (this->init_corner == 0)
                this->shape->SetSize(this->prev_w - tx, this->prev_h - ty, -SHAPE_SIZE, -SHAPE_SIZE, this->prev_w, this->prev_h, constrain, reflect);
            else if (this->init_corner == 1)
                this->shape->SetSize(this->prev_w + tx, this->prev_h - ty, SHAPE_SIZE, -SHAPE_SIZE, this->prev_w, this->prev_h, constrain, reflect);
            else if (this->init_corner == 2)
                this->shape->SetSize(this->prev_w - tx, this->prev_h + ty, -SHAPE_SIZE, SHAPE_SIZE, this->prev_w, this->prev_h, constrain, reflect);
            else
                this->shape->SetSize(this->prev_w + tx, this->prev_h + ty, SHAPE_SIZE, SHAPE_SIZE, this->prev_w, this->prev_h, constrain, reflect);
            break;
        }

        case PsController::TOOL_MODIFY_TORSIO:
        {
            if (this->LogMustAdd())
                this->LogAdd(new LogTorsio(*this, this->shape, this->prev_i, this->prev_j));

            tx = fx - sx;
            ty = fy - sy;

            if (this->init_corner == 0)
                this->shape->SetTorsion(this->prev_i - tx * cos(this->shape->GetAngle()) - ty * sin(this->shape->GetAngle()), this->prev_j, constrain);
            else if (this->init_corner == 1)
                this->shape->SetTorsion(this->prev_i, this->prev_j - tx * sin(this->shape->GetAngle()) + ty * cos(this->shape->GetAngle()), constrain);
            else if (this->init_corner == 2)
                this->shape->SetTorsion(this->prev_i + tx * cos(this->shape->GetAngle()) + ty * sin(this->shape->GetAngle()), this->prev_j, constrain);
            else
                this->shape->SetTorsion(this->prev_i, this->prev_j + tx * sin(this->shape->GetAngle()) - ty * cos(this->shape->GetAngle()), constrain);
            break;
        }
        }
    }

    PsController::Instance().UpdateWindow();
    if (dlgPropreties)
        dlgPropreties->UpdateInformation(this);
}
