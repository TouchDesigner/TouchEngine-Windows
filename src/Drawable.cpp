/* Shared Use License: This file is owned by Derivative Inc. (Derivative) and
 * can only be used, and/or modified for use, in conjunction with
 * Derivative's TouchDesigner software, and only if you are a licensee who has
 * accepted Derivative's TouchDesigner license or assignment agreement (which
 * also govern the use of this file).  You may share a modified version of this
 * file with another authorized licensee of Derivative's TouchDesigner software.
 * Otherwise, no redistribution or sharing of this file, with or without
 * modification, is permitted.
 *
 * TouchEngine
 *
 * Copyright © 2021 Derivative. All rights reserved.
 *
 */

#include "stdafx.h"
#include "Drawable.h"


Drawable::Drawable()
    : x(0.0), y(0.0), width(0.0), height(0.0)
{
}

Drawable::Drawable(float ix, float iy, float iwidth, float iheight)
    : x(ix), y(iy), width(iwidth), height(iheight)
{
}


Drawable::~Drawable()
{
}
