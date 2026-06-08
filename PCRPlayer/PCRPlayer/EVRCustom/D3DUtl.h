#pragma once

#include <d3dx9tex.h>
#include "DX9Shaders.h"

namespace d3dutl {

typedef enum {
	RESIZER_UNKNOWN = -1,
	RESIZER_DEFAULT = 0,
	RESIZER_NEAREST,
	RESIZER_BILINEAR,
	RESIZER_SHADER_NEAREST,
	RESIZER_SHADER_BILINEAR,
	RESIZER_SHADER_SMOOTHERSTEP,
	RESIZER_SHADER_BICUBIC06,
	RESIZER_SHADER_BICUBIC08,
	RESIZER_SHADER_BICUBIC10,
	RESIZER_SHADER_BSPLINE4,
	RESIZER_SHADER_MITCHELL4,
	RESIZER_SHADER_CATMULL4,
	RESIZER_SHADER_2PASS_BICUBIC06,
	RESIZER_SHADER_2PASS_BICUBIC08,
	RESIZER_SHADER_2PASS_BICUBIC10,
	RESIZER_SHADER_2PASS_BSPLINE4,
	RESIZER_SHADER_2PASS_MITCHELL4,
	RESIZER_SHADER_2PASS_CATMULL4,
	RESIZER_SHADER_2PASS_LANCZOS2,
	RESIZER_SHADER_2PASS_LANCZOS3,
} RESIZER_TYPE;

typedef enum {
	shader_nearest,
	shader_bilinear,
	shader_smootherstep,
	shader_bicubic06,
	shader_bicubic08,
	shader_bicubic10,
	shader_bspline4,
	shader_mitchell4,
	shader_catmull4,
	shader_bicubic06_x,
	shader_bicubic06_y,
	shader_bicubic08_x,
	shader_bicubic08_y,
	shader_bicubic10_x,
	shader_bicubic10_y,
	shader_bspline4_x,
	shader_bspline4_y,
	shader_mitchell4_x,
	shader_mitchell4_y,
	shader_catmull4_x,
	shader_catmull4_y,
	shader_lanczos2_x,
	shader_lanczos2_y,
	shader_lanczos3_x,
	shader_lanczos3_y,
	shader_downscaling_x,
	shader_downscaling_y,
	shader_downscaling,
	shader_count
} shader_type;



HINSTANCE GetD3X9Dll();

class D3DLoader {

	typedef HRESULT (WINAPI * D3DXCreateTexturePtr) (
			LPDIRECT3DDEVICE9         pDevice,
			UINT                      Width,
			UINT                      Height,
			UINT                      MipLevels,
			DWORD                     Usage,
			D3DFORMAT                 Format,
			D3DPOOL                   Pool,
			LPDIRECT3DTEXTURE9*       ppTexture);

	typedef HRESULT (WINAPI * D3DXCompileShaderPtr) (
		LPCSTR		pSrcData,
		UINT		SrcDataLen,
		CONST D3DXMACRO* pDefines,
		LPD3DXINCLUDE	pInclude,
		LPCSTR		pFunctionName,
		LPCSTR		pProfile,
		DWORD		Flags,
		LPD3DXBUFFER*	ppShader,
		LPD3DXBUFFER*	ppErrorMsgs,
		LPD3DXCONSTANTTABLE* ppConstantTable);

	typedef HRESULT (WINAPI * D3DXDisassembleShaderPtr) (
		CONST DWORD*	pShader,
		bool		EnableColorCode,
		LPCSTR		pComments,
		LPD3DXBUFFER*	ppDisassembly);

	static D3DXCreateTexturePtr D3DXCreateTexture_;
	static D3DXCompileShaderPtr D3DXCompileShader_;
	static D3DXDisassembleShaderPtr D3DXDisassembleShader_;



	D3DLoader();
	~D3DLoader();
public:
	static void Init()
	{
		static D3DLoader init;
	}

	static HRESULT WINAPI D3DXCreateTexture(
			LPDIRECT3DDEVICE9         pDevice,
			UINT                      Width,
			UINT                      Height,
			UINT                      MipLevels,
			DWORD                     Usage,
			D3DFORMAT                 Format,
			D3DPOOL                   Pool,
			LPDIRECT3DTEXTURE9*       ppTexture)
	{
		if (D3DXCreateTexture_)
		{
			return D3DXCreateTexture_(pDevice, Width, Height, MipLevels, Usage, Format, Pool, ppTexture);
		}
		return E_NOTIMPL;
	}

	static HRESULT WINAPI D3DXCompileShader(
			LPCSTR		pSrcData,
			UINT		SrcDataLen,
			CONST D3DXMACRO* pDefines,
			LPD3DXINCLUDE	pInclude,
			LPCSTR		pFunctionName,
			LPCSTR		pProfile,
			DWORD		Flags,
			LPD3DXBUFFER*	ppShader,
			LPD3DXBUFFER*	ppErrorMsgs,
			LPD3DXCONSTANTTABLE* ppConstantTable)
	{
		if (D3DXCompileShader_)
		{
			return D3DXCompileShader_(pSrcData, SrcDataLen, pDefines, pInclude, pFunctionName, pProfile, Flags, ppShader, ppErrorMsgs, ppConstantTable);
		}
		return E_NOTIMPL;
	}


	static HRESULT WINAPI D3DXDisassembleShader(
			CONST DWORD*	pShader,
			bool		EnableColorCode,
			LPCSTR		pComments,
			LPD3DXBUFFER*	ppDisassembly)
	{
		if (D3DXDisassembleShader_)
		{
			return D3DXDisassembleShader_(pShader, EnableColorCode, pComments, ppDisassembly);
		}
		return E_NOTIMPL;
	}
};




class D3DResizer {
	IDirect3DDevice9Ex* device_;
	D3DCAPS9 caps_;
	LPCSTR shaderProfile_;
	CComPtr<IDirect3DPixelShader9> pixelShaders_[shader_count];
	CComPtr<IDirect3DTexture9> resizeTexture_;

private:
	HRESULT InitShaderResizer(int iShader);

	HRESULT TextureResize(IDirect3DTexture9* pTexture, const CRect &srcRect, const CRect &dstRect, D3DTEXTUREFILTERTYPE filter);
	HRESULT TextureResizeShader(IDirect3DTexture9* pTexture, const CRect &srcRect, const CRect &dstRect, D3DTEXTUREFILTERTYPE filter, int iShader, bool bDownscale);
	HRESULT TextureResizeShader2pass(IDirect3DTexture9* pTexture, const CRect &srcRect, const CRect &dstRect, D3DTEXTUREFILTERTYPE filter, int iShader1, bool bDownscale);

public:
	D3DResizer(IDirect3DDevice9Ex* device);

	HRESULT ResizeRect(IDirect3DSurface9* src, const RECT* pSrcRect, IDirect3DSurface9* dst, const RECT* pDstRect, int resizer);
};

} // namespace d3dutl