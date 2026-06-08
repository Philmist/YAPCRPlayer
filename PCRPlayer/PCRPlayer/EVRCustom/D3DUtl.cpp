#include "../stdafx.h"

//#pragma comment(lib, "D3dx9.lib")

namespace d3dutl {

HINSTANCE GetD3X9Dll()
{
#if D3DX_SDK_VERSION < 43
#error DirectX SDK June 2010 (v43) or newer is required
#endif
	static HINSTANCE s_hD3DX9Dll = NULL;

	if (s_hD3DX9Dll == NULL)
	{
		// load latest compatible version of the DLL that is available
		s_hD3DX9Dll = LoadLibrary(L"d3dx9_43.dll");
	}
	return s_hD3DX9Dll;
}

D3DLoader::D3DXCreateTexturePtr D3DLoader::D3DXCreateTexture_ = NULL;
D3DLoader::D3DXCompileShaderPtr D3DLoader::D3DXCompileShader_ = NULL;
D3DLoader::D3DXDisassembleShaderPtr D3DLoader::D3DXDisassembleShader_ = NULL;

D3DLoader::D3DLoader()
{
	HINSTANCE hDll = GetD3X9Dll();

	if (hDll)
	{
		D3DXCreateTexture_ = (D3DXCreateTexturePtr)GetProcAddress(hDll, "D3DXCreateTexture");
		D3DXCompileShader_ = (D3DXCompileShaderPtr)GetProcAddress(hDll, "D3DXCompileShader");
		D3DXDisassembleShader_ = (D3DXDisassembleShaderPtr)GetProcAddress(hDll, "D3DXDisassembleShader");
	}
}

D3DLoader::~D3DLoader()
{

}


template<int texcoords>
struct MYD3DVERTEX {
	float x, y, z, rhw;
	struct {
		float u, v;
	} t[texcoords];
};

template<>
struct MYD3DVERTEX<0> {
	float x, y, z, rhw;
	DWORD Diffuse;
};

template<int texcoords>
static void AdjustQuad(MYD3DVERTEX<texcoords>* v, double dx, double dy)
{
	for (int i = 0; i < 4; i++) {
		v[i].x -= 0.5f;
		v[i].y -= 0.5f;

		for (int j = 0; j < max(texcoords - 1, 1); j++) {
			v[i].t[j].u -= (float)(0.5f*dx);
			v[i].t[j].v -= (float)(0.5f*dy);
		}

		if (texcoords > 1) {
			v[i].t[texcoords - 1].u -= 0.5f;
			v[i].t[texcoords - 1].v -= 0.5f;
		}
	}
}

template<int texcoords>
static HRESULT TextureBlt(IDirect3DDevice9* pD3DDev, MYD3DVERTEX<texcoords> v[4], D3DTEXTUREFILTERTYPE filter)
{
	if (!pD3DDev) {
		return E_POINTER;
	}

	DWORD FVF = 0;

	switch (texcoords) {
		case 1: FVF = D3DFVF_TEX1; break;
		case 2: FVF = D3DFVF_TEX2; break;
		case 3: FVF = D3DFVF_TEX3; break;
		case 4: FVF = D3DFVF_TEX4; break;
		case 5: FVF = D3DFVF_TEX5; break;
		case 6: FVF = D3DFVF_TEX6; break;
		case 7: FVF = D3DFVF_TEX7; break;
		case 8: FVF = D3DFVF_TEX8; break;
		default:
			return E_FAIL;
	}

	HRESULT hr;

	hr = pD3DDev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	hr = pD3DDev->SetRenderState(D3DRS_LIGHTING, FALSE);
	hr = pD3DDev->SetRenderState(D3DRS_ZENABLE, FALSE);
	hr = pD3DDev->SetRenderState(D3DRS_STENCILENABLE, FALSE);
	hr = pD3DDev->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	hr = pD3DDev->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
	hr = pD3DDev->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
	hr = pD3DDev->SetRenderState(D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_ALPHA|D3DCOLORWRITEENABLE_BLUE|D3DCOLORWRITEENABLE_GREEN|D3DCOLORWRITEENABLE_RED);

	for (int i = 0; i < texcoords; i++) {
		hr = pD3DDev->SetSamplerState(i, D3DSAMP_MAGFILTER, filter);
		hr = pD3DDev->SetSamplerState(i, D3DSAMP_MINFILTER, D3DTEXF_NONE);
		hr = pD3DDev->SetSamplerState(i, D3DSAMP_MIPFILTER, D3DTEXF_NONE);

		hr = pD3DDev->SetSamplerState(i, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
		hr = pD3DDev->SetSamplerState(i, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
	}

	//

	hr = pD3DDev->SetFVF(D3DFVF_XYZRHW | FVF);
	// hr = pD3DDev->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, v, sizeof(v[0]));

	MYD3DVERTEX<texcoords> tmp = v[2];
	v[2] = v[3];
	v[3] = tmp;
	hr = pD3DDev->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, v, sizeof(v[0]));

	for (int i = 0; i < texcoords; i++) {
		pD3DDev->SetTexture(i, NULL);
	}

	return S_OK;
}

static void Transform(CRect r, Vector v[4])
{
	XForm xform = XForm(Ray(Vector(0, 0, 0), Vector(0, 0, 0)), Vector(1, 1, 1), false);

	v[0] = Vector((float)r.left,  (float)r.top, 0);
	v[1] = Vector((float)r.right, (float)r.top, 0);
	v[2] = Vector((float)r.left,  (float)r.bottom, 0);
	v[3] = Vector((float)r.right, (float)r.bottom, 0);

	Vector center((float)r.CenterPoint().x, (float)r.CenterPoint().y, 0);
	int l = (int)(Vector((float)r.Size().cx, (float)r.Size().cy, 0).Length()*1.5f)+1;

	for (size_t i = 0; i < 4; i++) {
		v[i] = xform << (v[i] - center);
		v[i].z = v[i].z / l + 0.5f;
		v[i].x /= v[i].z*2;
		v[i].y /= v[i].z*2;
		v[i] += center;
	}
}

static HRESULT CompileShader(
	IDirect3DDevice9* pD3DDev,
	LPCSTR pSrcData,
	LPCSTR pFunctionName,
	LPCSTR pProfile,
	DWORD Flags,
	const D3DXMACRO* pDefines,
	IDirect3DPixelShader9** ppPixelShader,
	CString* errmsg = NULL,
	CString* disasm = NULL)
{
	HRESULT hr;

	CComPtr<ID3DXBuffer> pShader, pDisAsm, pErrorMsgs;
	hr = D3DLoader::D3DXCompileShader(pSrcData, (UINT)strlen(pSrcData), pDefines, NULL, pFunctionName, pProfile, Flags, &pShader, &pErrorMsgs, NULL);

	if (FAILED(hr)) {
		if (errmsg) {
			CStringA msg = "Unexpected compiler error";

			if (pErrorMsgs) {
				int len = pErrorMsgs->GetBufferSize();
				memcpy(msg.GetBufferSetLength(len), pErrorMsgs->GetBufferPointer(), len);
			}

			*errmsg = msg;
		}

		return hr;
	}

	if (ppPixelShader) {
		if (!pD3DDev) {
			return E_FAIL;
		}
		hr = pD3DDev->CreatePixelShader((DWORD*)pShader->GetBufferPointer(), ppPixelShader);
		if (FAILED(hr)) {
			return hr;
		}
	}

	if (disasm) {
		hr = D3DLoader::D3DXDisassembleShader((DWORD*)pShader->GetBufferPointer(), FALSE, NULL, &pDisAsm);
		if (SUCCEEDED(hr) && pDisAsm) {
			*disasm = CStringA((const char*)pDisAsm->GetBufferPointer());
		}
	}

	return S_OK;
}

static HRESULT CreateTextureFromSurface(IDirect3DDevice9Ex* device, IDirect3DTexture9** texture, IDirect3DSurface9* surface)
{
	D3DSURFACE_DESC desc;
	if (D3D_OK != surface->GetDesc(&desc))
	{
		return E_FAIL;
	}

	HRESULT hr = S_OK;
	hr = D3DLoader::D3DXCreateTexture(device, desc.Width, desc.Height, 1, D3DUSAGE_RENDERTARGET,
		//D3DFMT_A32B32G32R32F,
		desc.Format,
		D3DPOOL_DEFAULT, texture);

	if (texture)
	{
		IDirect3DSurface9* texSurface = NULL;
		hr = (*texture)->GetSurfaceLevel(0, &texSurface);

		if (texSurface)
		{
			hr = device->StretchRect(surface, NULL, texSurface, NULL, D3DTEXF_NONE);
			SAFE_RELEASE(texSurface);
		}
	}
	return S_OK;
}

HRESULT GetSurfaceRect(IDirect3DSurface9* surface, RECT& rect, const RECT* pDefaultRect = NULL)
{
	if (pDefaultRect)
	{
		rect = *pDefaultRect;
		return S_OK;
	}

	D3DSURFACE_DESC desc;
	if (D3D_OK != surface->GetDesc(&desc))
	{
		return E_FAIL;
	}

	rect.left = 0;
	rect.top = 0;
	rect.right = desc.Width;
	rect.bottom = desc.Height;
	return S_OK;
}

/*
HRESULT CopyTextureToSurface(IDirect3DDevice9Ex* device, IDirect3DTexture9* texture, IDirect3DSurface9* surface)
{
	HRESULT hr = S_OK;
	if (texture)
	{
		IDirect3DSurface9* texSurface = NULL;
		hr = texture->GetSurfaceLevel(0, &texSurface);

		if (texSurface)
		{
			hr = device->StretchRect(texSurface, NULL, surface, NULL, D3DTEXF_NONE);
			SAFE_RELEASE(texSurface);
		}
	}
	return S_OK;
}
*/



////////////////////////////////////////////////////////////////////////////////////////


D3DResizer::D3DResizer(IDirect3DDevice9Ex* device)
	: device_(device)
	, shaderProfile_(NULL)
{
	if (!device) { return; }

	ZeroMemory(&caps_, sizeof(caps_));
	device->GetDeviceCaps(&caps_);

	// Define the shader profile.
	if (caps_.PixelShaderVersion >= D3DPS_VERSION(3, 0)) {
		shaderProfile_ = "ps_3_0";
	} else if (caps_.PixelShaderVersion >= D3DPS_VERSION(2,0)) {
		// http://en.wikipedia.org/wiki/High-level_shader_language

		if (caps_.PS20Caps.NumTemps >= 22
			&& (caps_.PS20Caps.Caps & (D3DPS20CAPS_ARBITRARYSWIZZLE | D3DPS20CAPS_GRADIENTINSTRUCTIONS |
			D3DPS20CAPS_PREDICATION | D3DPS20CAPS_NODEPENDENTREADLIMIT | D3DPS20CAPS_NOTEXINSTRUCTIONLIMIT))) {
			shaderProfile_ = "ps_2_a";
		} else if (caps_.PS20Caps.NumTemps >= 32
			&& (caps_.PS20Caps.Caps & D3DPS20CAPS_NOTEXINSTRUCTIONLIMIT)) {
			shaderProfile_ = "ps_2_b";
		} else {
			shaderProfile_ = "ps_2_0";
		}
	} else {
		shaderProfile_ = NULL;
	}
}



HRESULT D3DResizer::InitShaderResizer(int iShader)
{
	if (iShader < 0 || iShader >= shader_count) {
		return E_INVALIDARG;
	}

	if (pixelShaders_[iShader]) {
		return S_OK;
	}

	if (!shaderProfile_) {
		return E_FAIL;
	}

	bool twopass = false;
	std::string src;
	D3DXMACRO ShaderMacros[3] = {
		{ "Ml", caps_.PixelShaderVersion >= D3DPS_VERSION(3, 0) ? "1" : "0" },
		{ NULL, NULL },
		{ NULL, NULL }
	};

	switch (iShader) {
	case shader_nearest:
		src = shader_resizer_nearest;
		break;
	case shader_bilinear:
		src = shader_resizer_bilinear;
		break;
	case shader_smootherstep:
		src = shader_resizer_smootherstep;
		break;
	case shader_bicubic06:
		src = shader_resizer_bicubic;
		ShaderMacros[1].Name = "A";
		ShaderMacros[1].Definition = "-0.6";
		break;
	case shader_bicubic08:
		src = shader_resizer_bicubic;
		ShaderMacros[1].Name = "A";
		ShaderMacros[1].Definition = "-1.0"; // ORG(-0.8)
		break;
	case shader_bicubic10:
		src = shader_resizer_bicubic;
		ShaderMacros[1].Name = "A";
		ShaderMacros[1].Definition = "-1.4"; // ORG(-1.0)
		break;
	case shader_bspline4:
		src = shader_resizer_bspline4;
		break;
	case shader_mitchell4:
		src = shader_resizer_mitchell4;
		break;
	case shader_catmull4:
		src = shader_resizer_catmull4;
		break;

	case shader_bicubic06_y:
		iShader--;
	case shader_bicubic06_x:
		src = shader_resizer_bicubic_2pass;
		ShaderMacros[1].Name = "A";
		ShaderMacros[1].Definition = "-0.6";
		twopass = true;
		break;
	case shader_bicubic08_y:
		iShader--;
	case shader_bicubic08_x:
		src = shader_resizer_bicubic_2pass;
		ShaderMacros[1].Name = "A";
		ShaderMacros[1].Definition = "-1.0"; // ORG(-0.8)
		twopass = true;
		break;
	case shader_bicubic10_y:
		iShader--;
	case shader_bicubic10_x:
		src = shader_resizer_bicubic_2pass;
		ShaderMacros[1].Name = "A";
		ShaderMacros[1].Definition = "-1.4"; // ORG(-1.0)
		twopass = true;
		break;
	case shader_bspline4_y:
		iShader--;
	case shader_bspline4_x:
		src = shader_resizer_bspline4_2pass;
		twopass = true;
		break;
	case shader_mitchell4_y:
		iShader--;
	case shader_mitchell4_x:
		src = shader_resizer_mitchell4_2pass;
		twopass = true;
		break;
	case shader_catmull4_y:
		iShader--;
	case shader_catmull4_x:
		src = shader_resizer_catmull4_2pass;
		twopass = true;
		break;

	case shader_lanczos2_y:
		iShader--;
	case shader_lanczos2_x:
		src = shader_resizer_lanczos2_2pass;
		twopass = true;
		break;
	case shader_lanczos3_y:
		iShader--;
	case shader_lanczos3_x:
		src = shader_resizer_lanczos3_2pass;
		twopass = true;
		break;
	}

	if (src.empty()) {
		return E_FAIL;
	}

	HRESULT hr = S_OK;
	CString ErrorMessage;

	if (twopass) {
		hr = CompileShader(device_, src.c_str(), "main_x", shaderProfile_, 0, ShaderMacros, &pixelShaders_[iShader], &ErrorMessage);
		if (hr == S_OK) {
			hr = CompileShader(device_, src.c_str(), "main_y", shaderProfile_, 0, ShaderMacros, &pixelShaders_[iShader + 1], &ErrorMessage);
		}
	} else {
		hr = CompileShader(device_, src.c_str(), "main", shaderProfile_, 0, ShaderMacros, &pixelShaders_[iShader], &ErrorMessage);
	}

	if (hr == S_OK && !pixelShaders_[shader_downscaling_x]) {
		src = shader_resizer_downscaling_2pass;
		hr = CompileShader(device_, src.c_str(), "main_x", shaderProfile_, 0, ShaderMacros, &pixelShaders_[shader_downscaling_x], &ErrorMessage);
		if (hr == S_OK && !pixelShaders_[shader_downscaling_y]) {
			src = shader_resizer_downscaling_2pass;
			hr = CompileShader(device_, src.c_str(), "main_y", shaderProfile_, 0, ShaderMacros, &pixelShaders_[shader_downscaling_y], &ErrorMessage);
		}
	}

	if (FAILED(hr)) {
		DebugText(L"InitShaderResizer() : shader compilation failed\n%s\n", ErrorMessage.GetString());
		ASSERT(0);
		return hr;
	}

	return S_OK;
}


HRESULT D3DResizer::TextureResize(IDirect3DTexture9* pTexture, const CRect &srcRect, const CRect &dstRect, D3DTEXTUREFILTERTYPE filter)
{
	HRESULT hr;

	D3DSURFACE_DESC desc;
	if (!pTexture || FAILED(pTexture->GetLevelDesc(0, &desc))) {
		return E_FAIL;
	}

	Vector dst[4];
	Transform(dstRect, dst);

	float w = (float)desc.Width;
	float h = (float)desc.Height;

	float dx2 = 1.0f/w;
	float dy2 = 1.0f/h;

	MYD3DVERTEX<1> v[] = {
		{dst[0].x, dst[0].y, dst[0].z, 1.0f/dst[0].z, {srcRect.left * dx2,  srcRect.top * dy2} },
		{dst[1].x, dst[1].y, dst[1].z, 1.0f/dst[1].z, {srcRect.right * dx2, srcRect.top * dy2} },
		{dst[2].x, dst[2].y, dst[2].z, 1.0f/dst[2].z, {srcRect.left * dx2,  srcRect.bottom * dy2} },
		{dst[3].x, dst[3].y, dst[3].z, 1.0f/dst[3].z, {srcRect.right * dx2, srcRect.bottom * dy2} },
	};
	AdjustQuad(v, 0, 0);

	hr = device_->SetTexture(0, pTexture);
	hr = device_->SetPixelShader(NULL);
	hr = TextureBlt(device_, v, filter);

	return hr;
}


HRESULT D3DResizer::TextureResizeShader(IDirect3DTexture9* pTexture, const CRect &srcRect, const CRect &dstRect, D3DTEXTUREFILTERTYPE filter, int iShader, bool bDownscale)
{
	HRESULT hr;

	D3DSURFACE_DESC desc;
	if (!pTexture || FAILED(pTexture->GetLevelDesc(0, &desc))) {
		return E_FAIL;
	}

	Vector dst[4];
	Transform(dstRect, dst);

	const float w = sqrt(pow(dst[1].x - dst[0].x, 2) + pow(dst[1].y - dst[0].y, 2) + pow(dst[1].z - dst[0].z, 2));
	const float h = sqrt(pow(dst[2].x - dst[0].x, 2) + pow(dst[2].y - dst[0].y, 2) + pow(dst[2].z - dst[0].z, 2));
	const float rx = srcRect.Width() / w;
	const float ry = srcRect.Height() / h;

	if (bDownscale)
	{
		if (pixelShaders_[shader_downscaling_x] &&
			pixelShaders_[shader_downscaling_y] &&
			rx > 2.0f && ry > 2.0f)
		{
			return TextureResizeShader2pass(pTexture, srcRect, dstRect, filter, iShader, bDownscale);
		}
	}

	// make const to give compiler a chance of optimising, also float faster than double and converted to float to sent to PS anyway
	const float dx = 1.0f/(float)desc.Width;
	const float dy = 1.0f/(float)desc.Height;
	const float tx0 = (float)srcRect.left - 0.5f;
	const float tx1 = (float)srcRect.right - 0.5f;
	const float ty0 = (float)srcRect.top - 0.5f;
	const float ty1 = (float)srcRect.bottom - 0.5f;

	MYD3DVERTEX<1> v[] = {
		{dst[0].x - 0.5f, dst[0].y -0.5f, dst[0].z, 1.0f/dst[0].z, { tx0, ty0 } },
		{dst[1].x - 0.5f, dst[1].y -0.5f, dst[1].z, 1.0f/dst[1].z, { tx1, ty0 } },
		{dst[2].x - 0.5f, dst[2].y -0.5f, dst[2].z, 1.0f/dst[2].z, { tx0, ty1 } },
		{dst[3].x - 0.5f, dst[3].y -0.5f, dst[3].z, 1.0f/dst[3].z, { tx1, ty1 } },
	};

	hr = device_->SetTexture(0, pTexture);

	{
		float fConstData[][4] = { { dx, dy, 0, 0 }, { dx*0.5f, dy*0.5f, 0, 0 }, { dx, 0, 0, 0 }, { 0, dy, 0, 0 } };
		hr = device_->SetPixelShaderConstantF(0, (float*)fConstData, _countof(fConstData));
		hr = device_->SetPixelShader(pixelShaders_[iShader]);
	}

	// D3DTEXF_TEST
	hr = TextureBlt(device_, v, filter);
	return hr;
}


// The 2 pass sampler is incorrect in that it only does bilinear resampling in the y direction.
HRESULT D3DResizer::TextureResizeShader2pass(IDirect3DTexture9* pTexture, const CRect &srcRect, const CRect &dstRect, D3DTEXTUREFILTERTYPE filter, int iShader1, bool bDownscale)
{
	HRESULT hr;

	// src情報
	D3DSURFACE_DESC desc;
	if (!pTexture || FAILED(pTexture->GetLevelDesc(0, &desc))) {
		return E_FAIL;
	}

	// dst情報
	Vector dst[4];
	Transform(dstRect, dst);

	// 縮小係数
	const float w2 = sqrt(pow(dst[1].x - dst[0].x, 2) + pow(dst[1].y - dst[0].y, 2) + pow(dst[1].z - dst[0].z, 2));
	const float h2 = sqrt(pow(dst[2].x - dst[0].x, 2) + pow(dst[2].y - dst[0].y, 2) + pow(dst[2].z - dst[0].z, 2));
	const float rx = srcRect.Width() / w2;
	const float ry = srcRect.Height() / h2;

	// リサイズテクスチャはsrcRect以上の範囲でdstRectに合わせる
	UINT texWidth = srcRect.Width();
	UINT texHeight = srcRect.Height();

	if (dstRect.Width() > srcRect.Width()) texWidth = dstRect.Width();
	if (dstRect.Height() > srcRect.Height()) texHeight = dstRect.Height();


	D3DSURFACE_DESC resizeDesc;

	bool downscale = false;

	if (bDownscale)
	{
		downscale =
			pixelShaders_[shader_downscaling_x] &&
			pixelShaders_[shader_downscaling_y] &&
			rx >= 2.0f && ry >= 2.0f;
	}

	//DebugText(L"tw=%d, th=%d; rx=%f, ry=%f;\n", texWidth, texHeight, rx, ry);



	if (resizeTexture_) // リサイズ用テクスチャがある
	{
		if (FAILED(resizeTexture_->GetLevelDesc(0, &resizeDesc)))
		{
			return TextureResize(pTexture, srcRect, dstRect, D3DTEXF_LINEAR);
		}

		if (texWidth != resizeDesc.Width || texHeight != resizeDesc.Height)
		{
			resizeTexture_.Release();
		}
	}

	if (!resizeTexture_)
	{
		hr = device_->CreateTexture(
					texWidth, texHeight, 1, D3DUSAGE_RENDERTARGET,
					desc.Format,
					//D3DFMT_A32B32G32R32F,
					//m_SurfaceType == D3DFMT_A32B32G32R32F ? D3DFMT_A32B32G32R32F : D3DFMT_A16B16G16R16F, // use only float textures here
					D3DPOOL_DEFAULT, &resizeTexture_, NULL);
		if (FAILED(hr))
		{
			resizeTexture_ = NULL;
			return TextureResize(pTexture, srcRect, dstRect, D3DTEXF_LINEAR);
		}

		if (FAILED(resizeTexture_->GetLevelDesc(0, &resizeDesc)))
		{
			return TextureResize(pTexture, srcRect, dstRect, D3DTEXF_LINEAR);
		}
	}

	const float dx0 = 1.0f/(float)desc.Width;
	const float dy0 = 1.0f/(float)desc.Height;

	float w1 = min(w2, (float)texWidth);
	float h1 = (float)min(srcRect.Height(), (int)texHeight);

	const float dx1 = 1.0f/(float)resizeDesc.Width;
	const float dy1 = 1.0f/(float)resizeDesc.Height;

	const float tx0 = (float)srcRect.left - 0.5f;
	const float tx1 = (float)srcRect.right - 0.5f;
	const float ty0 = (float)srcRect.top - 0.5f;
	const float ty1 = (float)srcRect.bottom - 0.5f;

	MYD3DVERTEX<1> vx[] = {
		{ -0.5f, -0.5f, 0.5f, 2.0f, { tx0, ty0 } },
		{    w1, -0.5f, 0.5f, 2.0f, { tx1, ty0 } },
		{ -0.5f,    h1, 0.5f, 2.0f, { tx0, ty1 } },
		{    w1,    h1, 0.5f, 2.0f, { tx1, ty1 } },
	};

	MYD3DVERTEX<1> vy[] = {
		{dst[0].x - 0.5f, dst[0].y - 0.5f, dst[0].z, 1.0f/dst[0].z, { -0.5f, -0.5f } },
		{dst[1].x - 0.5f, dst[1].y - 0.5f, dst[1].z, 1.0f/dst[1].z, {    w1, -0.5f } },
		{dst[2].x - 0.5f, dst[2].y - 0.5f, dst[2].z, 1.0f/dst[2].z, { -0.5f,    h1 } },
		{dst[3].x - 0.5f, dst[3].y - 0.5f, dst[3].z, 1.0f/dst[3].z, {    w1,    h1 } },
	};


	// remember current RenderTarget
	CComPtr<IDirect3DSurface9> pRenderTarget;
	hr = device_->GetRenderTarget(0, &pRenderTarget);
	// set temp RenderTarget
	CComPtr<IDirect3DSurface9> pTempRenderTarget;
	hr = resizeTexture_->GetSurfaceLevel(0, &pTempRenderTarget);
	hr = device_->SetRenderTarget(0, pTempRenderTarget);

	// resize width
	hr = device_->SetTexture(0, pTexture);
	
	if (downscale)
	{
		float fConstData[][4] = {{dx0, dy0, 0, 0}, {rx, 0, 0, 0}, {ry, 0, 0, 0}};
		hr = device_->SetPixelShaderConstantF(0, (float*)fConstData, _countof(fConstData));
		hr = device_->SetPixelShader(pixelShaders_[shader_downscaling_x]);
	}
	else
	{
		float fConstData[][4] = {{dx0, dy0, 0, 0}};
		hr = device_->SetPixelShaderConstantF(0, (float*)fConstData, _countof(fConstData));
		hr = device_->SetPixelShader(pixelShaders_[iShader1]);
	}
	
	// D3DTEXF_TEST
	hr = TextureBlt(device_, vx, filter);


	// restore current RenderTarget
	hr = device_->SetRenderTarget(0, pRenderTarget);
	
	// resize height
	hr = device_->SetTexture(0, resizeTexture_);
	
	if (downscale)
	{
		float fConstData[][4] = {{dx1, dy1, 0, 0}, {rx, 0, 0, 0}, {ry, 0, 0, 0}};
		hr = device_->SetPixelShaderConstantF(0, (float*)fConstData, _countof(fConstData));
		hr = device_->SetPixelShader(pixelShaders_[shader_downscaling_y]);
	}
	else
	{
		float fConstData[][4] = {{dx1, dy1, 0, 0}};
		hr = device_->SetPixelShaderConstantF(0, (float*)fConstData, _countof(fConstData));
		hr = device_->SetPixelShader(pixelShaders_[iShader1 + 1]);
	}
	
	// D3DTEXF_TEST
	hr = TextureBlt(device_, vy, filter);

	return hr;
}


HRESULT D3DResizer::ResizeRect(IDirect3DSurface9* src, const RECT* pSrcRect, IDirect3DSurface9* dst, const RECT* pDstRect, int resizer)
{
	if (!device_ || !src || !dst) { return E_FAIL; }
	
	RECT srcRect, dstRect;
	if (FAILED(GetSurfaceRect(src, srcRect, pSrcRect))) { return E_FAIL; }
	if (FAILED(GetSurfaceRect(dst, dstRect, pDstRect))) { return E_FAIL; }


	D3DTEXTUREFILTERTYPE filter = D3DTEXF_POINT;
	int shader = shader_lanczos3_x;

	bool noshader = false;
	bool onepass = false;
	bool downsacale = true;
	
	switch (resizer)
	{
	case RESIZER_NEAREST:	filter = D3DTEXF_POINT; noshader = true; break;
	case RESIZER_BILINEAR:	filter = D3DTEXF_LINEAR; noshader = true; break;

	case RESIZER_SHADER_NEAREST:		shader = shader_nearest; onepass = true; downsacale = false; break;
	case RESIZER_SHADER_BILINEAR:		shader = shader_bilinear; onepass = true; break;
	case RESIZER_SHADER_SMOOTHERSTEP:	shader = shader_smootherstep; onepass = true; break;

	case RESIZER_SHADER_BICUBIC06:		shader = shader_bicubic06; onepass = true; break;
	case RESIZER_SHADER_BICUBIC08:		shader = shader_bicubic08; onepass = true; break;
	case RESIZER_SHADER_BICUBIC10:		shader = shader_bicubic10; onepass = true; break;
	case RESIZER_SHADER_BSPLINE4:		shader = shader_bspline4; onepass = true; break;
	case RESIZER_SHADER_MITCHELL4:		shader = shader_mitchell4; onepass = true; break;
	case RESIZER_SHADER_CATMULL4:		shader = shader_catmull4; onepass = true; break;

	case RESIZER_SHADER_2PASS_BICUBIC06:	shader = shader_bicubic06_x; break;
	case RESIZER_SHADER_2PASS_BICUBIC08:	shader = shader_bicubic08_x; break;
	case RESIZER_SHADER_2PASS_BICUBIC10:	shader = shader_bicubic10_x; break;
	case RESIZER_SHADER_2PASS_BSPLINE4:		shader = shader_bspline4_x; break;
	case RESIZER_SHADER_2PASS_MITCHELL4:	shader = shader_mitchell4_x; break;
	case RESIZER_SHADER_2PASS_CATMULL4:		shader = shader_catmull4_x; break;

	case RESIZER_SHADER_2PASS_LANCZOS2:		shader = shader_lanczos2_x; break;
	case RESIZER_SHADER_2PASS_LANCZOS3:		shader = shader_lanczos3_x; break;

	case RESIZER_UNKNOWN:
	case RESIZER_DEFAULT:
	default:
		return device_->StretchRect(src, &srcRect, dst, &dstRect, D3DTEXF_LINEAR);
	}

	HRESULT hr = S_OK;


	CComPtr<IDirect3DTexture9> texture;
	CreateTextureFromSurface(device_, &texture, src);

	if (texture)
	{
		CComPtr<IDirect3DSurface9> org;
		hr = device_->GetRenderTarget(0, &org);
		hr = device_->SetRenderTarget(0, dst);
		device_->BeginScene();
		if (noshader)
		{
			hr = TextureResize(texture, srcRect, dstRect, filter);
		}
		else
		{
			hr = InitShaderResizer(shader);
			if (SUCCEEDED(hr))
			{
				if (onepass)
				{
					hr = TextureResizeShader(texture, srcRect, dstRect, filter, shader, downsacale);
				}
				else
				{
					hr = TextureResizeShader2pass(texture, srcRect, dstRect, filter, shader, downsacale);
				}
			}
		}
		device_->EndScene();
		device_->SetRenderTarget(0, org);

		return hr;
	}

	return device_->StretchRect(src, &srcRect, dst, &dstRect, D3DTEXF_LINEAR);
}

} // namespace d3dutl





/*
HRESULT D3DResizer::TextureResizeShader(IDirect3DTexture9* pTexture, const CRect &dstRect, int iShader)
{
	HRESULT hr;

	D3DSURFACE_DESC desc;
	if (!pTexture || FAILED(pTexture->GetLevelDesc(0, &desc))) {
		return E_FAIL;
	}

	CRect srcRect(0, 0, desc.Width, desc.Height);

	Vector dst[4];
	Transform(dstRect, dst);


	const float w = sqrt(pow(dst[1].x - dst[0].x, 2) + pow(dst[1].y - dst[0].y, 2) + pow(dst[1].z - dst[0].z, 2));
	const float h = sqrt(pow(dst[2].x - dst[0].x, 2) + pow(dst[2].y - dst[0].y, 2) + pow(dst[2].z - dst[0].z, 2));
	const float rx = srcRect.Width() / w;
	const float ry = srcRect.Height() / h;

	// make const to give compiler a chance of optimising, also float faster than double and converted to float to sent to PS anyway
	const float dx = 1.0f/(float)desc.Width;
	const float dy = 1.0f/(float)desc.Height;
	const float tx0 = (float)srcRect.left - 0.5f;
	const float tx1 = (float)srcRect.right - 0.5f;
	const float ty0 = (float)srcRect.top - 0.5f;
	const float ty1 = (float)srcRect.bottom - 0.5f;

	MYD3DVERTEX<1> v[] = {
		{dst[0].x - 0.5f, dst[0].y -0.5f, dst[0].z, 1.0f/dst[0].z, { tx0, ty0 } },
		{dst[1].x - 0.5f, dst[1].y -0.5f, dst[1].z, 1.0f/dst[1].z, { tx1, ty0 } },
		{dst[2].x - 0.5f, dst[2].y -0.5f, dst[2].z, 1.0f/dst[2].z, { tx0, ty1 } },
		{dst[3].x - 0.5f, dst[3].y -0.5f, dst[3].z, 1.0f/dst[3].z, { tx1, ty1 } },
	};

	hr = device_->SetTexture(0, pTexture);

	DebugText(L"dx=%f dy=%f, rx=%f ry=%f\n", dx, dy, rx, ry);
	
	// shader_downscaling 縮小命令
	// 1920x1080の映像を縮小しようとするとフリーズする
	// おそらくシェーダでループを使っているので
	// 補完回数が増えると命令数限界に達してしまうことが原因
	if (pixelShaders_[shader_downscaling] && rx > 2.0f && ry > 2.0f) {
		//m_nDX9Resizer = RESIZER_SHADER_AVERAGE;
		float fConstData[][4] = {{dx, dy, 0, 0}, {rx, 0, 0, 0}, {ry, 0, 0, 0}};

		hr = device_->SetPixelShaderConstantF(0, (float*)fConstData, _countof(fConstData));
		hr = device_->SetPixelShader(pixelShaders_[shader_downscaling]);
	}
	else {
		float fConstData[][4] = { { dx, dy, 0, 0 }, { dx*0.5f, dy*0.5f, 0, 0 }, { dx, 0, 0, 0 }, { 0, dy, 0, 0 } };
		hr = device_->SetPixelShaderConstantF(0, (float*)fConstData, _countof(fConstData));
		hr = device_->SetPixelShader(pixelShaders_[iShader]);
	}
	hr = TextureBlt(device_, v, D3DTEXF_LINEAR);
	return hr;
}
*/

