#pragma once
#include <stdio.h>
#include <windows.h>
#include <d3d11.h>
#include <dxgi1_2.h>
#include <sal.h>
#include <new>
#include <warning.h>
#include <DirectXMath.h>
#include "PixelShader.h"
#include "VertexShader.h"

typedef _Return_type_success_(return == DUPL_RETURN_SUCCESS) enum
{
    DUPL_RETURN_SUCCESS = 0,
    DUPL_RETURN_ERROR_EXPECTED = 1,
    DUPL_RETURN_ERROR_UNEXPECTED = 2
}DUPL_RETURN;

#define NUMVERTICES 6

typedef struct _VERTEX
{
    DirectX::XMFLOAT3 Pos;
    DirectX::XMFLOAT2 TexCoord;
} VERTEX;
