#pragma once
class Renderer
{
public:
    Renderer();
    virtual ~Renderer();
    virtual bool setup() = 0;
    virtual bool render() = 0;
    void setBackgroundColor(float r, float g, float b);
protected:
    float myBackgroundColor[3];
};

