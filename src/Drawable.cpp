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
