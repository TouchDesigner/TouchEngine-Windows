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

#pragma once

class VertexShader
{
public:
    VertexShader();
    VertexShader(ID3D11VertexShader *shader, ID3D11InputLayout *layout);
    VertexShader(const VertexShader &o) = delete;
    VertexShader &operator=(const VertexShader &o) = delete;
    VertexShader(VertexShader &&o);
    ~VertexShader();
    VertexShader &operator=(VertexShader &&o);
    bool isValid() const;
    void setInputLayout(ID3D11DeviceContext *context);
    void setShader(ID3D11DeviceContext *context);
private:
    ID3D11VertexShader *myVertexShader;
    ID3D11InputLayout *myInputLayout;
};

