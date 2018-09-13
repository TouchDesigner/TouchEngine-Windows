#include "stdafx.h"
#include "Drawable.h"


Drawable::Drawable()
{
}

Drawable::Drawable(float ix, float iy, float iwidth, float iheight)
    : x(ix), y(iy), width(iwidth), height(iheight)
{
}


Drawable::~Drawable()
{
}
