#include "stdafx.h"
#include "Renderer.h"


Renderer::Renderer()
    : myBackgroundColor{0.0f, 0.0f, 0.0f}
{
}


Renderer::~Renderer()
{
}

void Renderer::setBackgroundColor(float r, float g, float b)
{
    myBackgroundColor[0] = r;
    myBackgroundColor[1] = g;
    myBackgroundColor[2] = b;
}
