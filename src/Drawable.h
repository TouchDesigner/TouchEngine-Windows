#pragma once
class Drawable
{
public:
    Drawable();
    Drawable(float x, float y, float width, float height);
    ~Drawable();
    float x;
    float y;
    float width;
    float height;
};

