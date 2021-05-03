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
#include "VertexShader.h"


VertexShader::VertexShader()
	: myVertexShader(nullptr), myInputLayout(nullptr)
{
}

VertexShader::VertexShader(ID3D11VertexShader * shader, ID3D11InputLayout * layout)
	:myVertexShader(shader), myInputLayout(layout)
{
	myVertexShader->AddRef();
	myInputLayout->AddRef();
}


VertexShader::VertexShader(VertexShader && o)
	: myVertexShader(o.myVertexShader), myInputLayout(o.myInputLayout)
{
	o.myVertexShader = nullptr;
	o.myInputLayout = nullptr;
}

VertexShader&
VertexShader::operator=(VertexShader && o)
{
	if (myVertexShader)
	{
		myVertexShader->Release();
	}
	if (myInputLayout)
	{
		myInputLayout->Release();
	}
	myVertexShader = o.myVertexShader;
	myInputLayout = o.myInputLayout;
	o.myVertexShader = nullptr;
	o.myInputLayout = nullptr;
	return *this;
}

bool
VertexShader::isValid() const
{
	if (myVertexShader && myInputLayout)
	{
		return true;
	}
	return false;
}

void
VertexShader::setInputLayout(ID3D11DeviceContext * context)
{
	context->IASetInputLayout(myInputLayout);
}

void
VertexShader::setShader(ID3D11DeviceContext * context)
{
	context->VSSetShader(myVertexShader, nullptr, 0);
}

VertexShader::~VertexShader()
{
	if (myVertexShader)
	{
		myVertexShader->Release();
	}
	if (myInputLayout)
	{
		myInputLayout->Release();
	}
}
