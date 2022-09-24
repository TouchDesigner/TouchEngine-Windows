/* Shared Use License: This file is owned by Derivative Inc. (Derivative)
* and can only be used, and/or modified for use, in conjunction with
* Derivative's TouchDesigner software, and only if you are a licensee who has
* accepted Derivative's TouchDesigner license or assignment agreement
* (which also govern the use of this file). You may share or redistribute
* a modified version of this file provided the following conditions are met:
*
* 1. The shared file or redistribution must retain the information set out
* above and this list of conditions.
* 2. Derivative's name (Derivative Inc.) or its trademarks may not be used
* to endorse or promote products derived from this file without specific
* prior written permission from Derivative.
*/

#include "stdafx.h"
#include "DX11VertexShader.h"


DX11VertexShader::DX11VertexShader()
{
}

DX11VertexShader::DX11VertexShader(ID3D11VertexShader * shader, ID3D11InputLayout * layout)
	:myVertexShader(shader), myInputLayout(layout)
{
}

bool
DX11VertexShader::isValid() const
{
	if (myVertexShader && myInputLayout)
	{
		return true;
	}
	return false;
}

void
DX11VertexShader::setInputLayout(ID3D11DeviceContext * context)
{
	context->IASetInputLayout(myInputLayout.Get());
}

void
DX11VertexShader::setShader(ID3D11DeviceContext * context)
{
	context->VSSetShader(myVertexShader.Get(), nullptr, 0);
}
