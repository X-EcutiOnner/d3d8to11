/**
 * Copyright (C) 2015 Patrick Mours. All rights reserved.
 * License: https://github.com/crosire/d3d8to9#license
 */

#include "pch.h"

#include <iomanip>

#include "d3d8to11.hpp"
#include "not_implemented.h"

using namespace d3d8to11;

// TODO: let d3d do the memory management work if possible

void Direct3DTexture8::create_native(ID3D11Texture2D* view_of)
{
	const auto& device  = device8->device;

	block_compressed = is_block_compressed(to_dxgi(format_));

	if (view_of != nullptr)
	{
		view_of->GetDesc(&desc);
		texture = view_of;

		is_render_target = !!(desc.BindFlags & D3D11_BIND_RENDER_TARGET);
		is_depth_stencil = !!(desc.BindFlags & D3D11_BIND_DEPTH_STENCIL);
	}
	else
	{
		uint32_t usage      = D3D11_USAGE_DEFAULT;
		uint32_t bind_flags = D3D11_BIND_SHADER_RESOURCE;

		is_render_target = !!(usage_ & D3DUSAGE_RENDERTARGET);
		is_depth_stencil = !!(usage_ & D3DUSAGE_DEPTHSTENCIL);

		if (!levels_)
		{
			++levels_;

			if (!is_render_target && !is_depth_stencil)
			{
				auto width  = width_;
				auto height = height_;

				while (width != 1 && height != 1)
				{
					++levels_;
					width  = std::max(1u, width / 2);
					height = std::max(1u, height / 2);
				}
			}
		}

		auto format = to_dxgi(format_);

		if (!IsWindows8OrGreater())
		{
			switch (format)
			{
				case DXGI_FORMAT_B5G6R5_UNORM:
				case DXGI_FORMAT_B5G5R5A1_UNORM:
				case DXGI_FORMAT_B4G4R4A4_UNORM:
				case DXGI_FORMAT_B8G8R8A8_UNORM:
					format = DXGI_FORMAT_R8G8B8A8_UNORM;
					break;

				default:
					break;
			}
		}

		/*if (Usage & D3DUSAGE_DYNAMIC)
		{
			usage = D3D11_USAGE_DYNAMIC;
		}*/

		if (is_render_target)
		{
			bind_flags |= D3D11_BIND_RENDER_TARGET;
		}

		if (is_depth_stencil)
		{
			bind_flags |= D3D11_BIND_DEPTH_STENCIL;
			format = to_typeless(format);
		}

		desc.ArraySize  = 1;
		desc.BindFlags  = bind_flags;
		desc.Usage      = static_cast<D3D11_USAGE>(usage);
		desc.Format     = format;
		desc.Width      = width_;
		desc.Height     = height_;
		desc.MipLevels  = levels_;
		desc.SampleDesc = { 1, 0 };

		auto hr = device->CreateTexture2D(&desc, nullptr, &texture);

		if (FAILED(hr))
		{
			std::stringstream message;

			message << "CreateTexture2D failed with error " << std::hex << static_cast<uint32_t>(hr) << std::dec
				<< ": format: " << static_cast<uint32_t>(desc.Format)
				<< " width: " << desc.Width << " height: " << desc.Height << " levels: " << desc.MipLevels;

			throw std::runtime_error(message.str());
		}

		auto srv_format = format;

		if (is_depth_stencil)
		{
			// create a shader resource view with a readable pixel format
			srv_format = typeless_to_float(format);

			// if float didn't work, it's probably int we want
			if (srv_format == DXGI_FORMAT_UNKNOWN)
			{
				srv_format = typeless_to_unorm(format);
			}
		}

		D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc {};

		srv_desc.Format              = srv_format;
		srv_desc.ViewDimension       = D3D11_SRV_DIMENSION_TEXTURE2D;
		srv_desc.Texture2D.MipLevels = levels_;

		if (FAILED(device->CreateShaderResourceView(texture.Get(), &srv_desc, &srv)))
		{
			throw std::runtime_error("CreateShaderResourceView failed");
		}
	}

	surfaces.clear();
	surfaces.resize(levels_);
	surfaces.shrink_to_fit();

	size_t level = 0;
	size_t total_size = 0;

	for (auto& it : surfaces)
	{
		it = new Direct3DSurface8(device8, this, level++);
		it->create_native();
		total_size += it->desc8.Size;
	}

	texture_buffer.resize(total_size);
	texture_buffer.shrink_to_fit();
}

// IDirect3DTexture8
Direct3DTexture8::Direct3DTexture8(Direct3DDevice8* device_, UINT Width, UINT Height, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool)
	: device8(device_)
{
	this->width_  = Width;
	this->height_ = Height;
	this->levels_ = Levels;
	this->usage_  = Usage;
	this->format_ = Format;
	this->pool_   = Pool;
}

HRESULT STDMETHODCALLTYPE Direct3DTexture8::QueryInterface(REFIID riid, void** ppvObj)
{
	if (ppvObj == nullptr)
	{
		return E_POINTER;
	}

	if (riid == __uuidof(this) ||
	    riid == __uuidof(IUnknown) ||
	    riid == __uuidof(Direct3DResource8) ||
	    riid == __uuidof(Direct3DBaseTexture8))
	{
		AddRef();

		*ppvObj = this;

		return S_OK;
	}

	return E_NOINTERFACE;
}

ULONG STDMETHODCALLTYPE Direct3DTexture8::AddRef()
{
	return Direct3DBaseTexture8::AddRef();
}

ULONG STDMETHODCALLTYPE Direct3DTexture8::Release()
{
	const auto result = Direct3DBaseTexture8::Release();

	if (!result)
	{
		delete this;
	}

	return result;
}

HRESULT STDMETHODCALLTYPE Direct3DTexture8::GetDevice(Direct3DDevice8** ppDevice)
{
	if (ppDevice == nullptr)
	{
		return D3DERR_INVALIDCALL;
	}

	device8->AddRef();
	*ppDevice = device8;
	return D3D_OK;
}

HRESULT STDMETHODCALLTYPE Direct3DTexture8::SetPrivateData(REFGUID refguid, const void* pData, DWORD SizeOfData, DWORD Flags)
{
	NOT_IMPLEMENTED_RETURN;
}

HRESULT STDMETHODCALLTYPE Direct3DTexture8::GetPrivateData(REFGUID refguid, void* pData, DWORD* pSizeOfData)
{
	NOT_IMPLEMENTED_RETURN;
}

HRESULT STDMETHODCALLTYPE Direct3DTexture8::FreePrivateData(REFGUID refguid)
{
	NOT_IMPLEMENTED_RETURN;
}

DWORD STDMETHODCALLTYPE Direct3DTexture8::SetPriority(DWORD PriorityNew)
{
	NOT_IMPLEMENTED_RETURN;
}

DWORD STDMETHODCALLTYPE Direct3DTexture8::GetPriority()
{
	NOT_IMPLEMENTED;
	return -1;
}

void STDMETHODCALLTYPE Direct3DTexture8::PreLoad()
{
	NOT_IMPLEMENTED;
}

D3DRESOURCETYPE STDMETHODCALLTYPE Direct3DTexture8::GetType()
{
	return D3DRTYPE_TEXTURE;
}

DWORD STDMETHODCALLTYPE Direct3DTexture8::SetLOD(DWORD LODNew)
{
	NOT_IMPLEMENTED_RETURN;
}

DWORD STDMETHODCALLTYPE Direct3DTexture8::GetLOD()
{
	NOT_IMPLEMENTED;
	return 0;
}

DWORD STDMETHODCALLTYPE Direct3DTexture8::GetLevelCount()
{
	return desc.MipLevels;
}

HRESULT STDMETHODCALLTYPE Direct3DTexture8::GetLevelDesc(UINT Level, D3DSURFACE_DESC8* pDesc)
{
	if (pDesc == nullptr || Level > GetLevelCount())
	{
		return D3DERR_INVALIDCALL;
	}

	const D3DSURFACE_DESC8& surface_desc8 = surfaces[Level]->desc8;

	const auto width  = surface_desc8.Width;
	const auto height = surface_desc8.Height;

	pDesc->Format          = format_;
	pDesc->Type            = GetType();
	pDesc->Usage           = usage_;
	pDesc->Pool            = pool_;
	pDesc->Size            = calc_texture_size(width, height, 1, format_);
	pDesc->MultiSampleType = D3DMULTISAMPLE_NONE;
	pDesc->Width           = width;
	pDesc->Height          = height;

	return D3D_OK;
}

HRESULT STDMETHODCALLTYPE Direct3DTexture8::GetSurfaceLevel(UINT Level, Direct3DSurface8** ppSurfaceLevel)
{
	if (!ppSurfaceLevel)
	{
		return D3DERR_INVALIDCALL;
	}

	*ppSurfaceLevel = nullptr;

	if (Level >= desc.MipLevels)
	{
		return D3DERR_INVALIDCALL;
	}

	*ppSurfaceLevel = surfaces[Level].Get();
	(*ppSurfaceLevel)->AddRef();

	return D3D_OK;
}

HRESULT STDMETHODCALLTYPE Direct3DTexture8::LockRect(UINT Level, D3DLOCKED_RECT* pLockedRect, const RECT* pRect, DWORD Flags)
{
	if (pRect)
	{
		return D3DERR_INVALIDCALL;
	}

	if (Level >= desc.MipLevels)
	{
		return D3DERR_INVALIDCALL;
	}

	if (locked_rects.find(Level) != locked_rects.end())
	{
		return D3DERR_INVALIDCALL;
	}

	const D3DSURFACE_DESC8& surface_desc8 = surfaces[Level]->desc8;
	const auto width = surface_desc8.Width;

	size_t level_offset = 0;
	size_t level_size = 0;
	get_level_offset(Level, level_offset, level_size);

	D3DLOCKED_RECT rect;
	rect.Pitch = calc_texture_size(width, 1, 1, format_);
	rect.pBits = &texture_buffer[level_offset];

	locked_rects[Level] = rect;
	*pLockedRect = rect;
	return D3D_OK;
}

HRESULT STDMETHODCALLTYPE Direct3DTexture8::UnlockRect(UINT Level)
{
	const auto it = locked_rects.find(Level);

	if (it == locked_rects.end())
	{
		return D3DERR_INVALIDCALL;
	}

	// HACK: fixes Phantasy Star Online: Blue Burst
	// TODO: instead of this, make sure render target data is accessible by the CPU, even if that means we need a staging texture
	if (!is_render_target && !is_depth_stencil)
	{
		const auto& context = device8->context;

		// TODO: make this behavior configurable [safe mipmaps]
		if (!Level)
		{
			if (should_convert())
			{
				for (UINT i = 0; i < levels_; ++i)
				{
					convert(i);
				}
			}
			else
			{
				for (UINT i = 0; i < levels_; ++i)
				{
					const auto desc8 = surfaces[i]->desc8;
					const auto width = desc8.Width;

					size_t level_offset = 0;
					size_t level_size = 0;
					get_level_offset(i, level_offset, level_size);

					D3DLOCKED_RECT rect;
					rect.Pitch = calc_texture_size(width, 1, 1, format_);
					rect.pBits = &texture_buffer[level_offset];

					context->UpdateSubresource(texture.Get(), i, nullptr, rect.pBits, rect.Pitch, 0);
				}
			}
		}
		else if (!convert(Level))
		{
			const auto& rect = it->second;
			context->UpdateSubresource(texture.Get(), Level, nullptr, rect.pBits, rect.Pitch, 0);
		}
	}
	
	locked_rects.erase(it);
	return D3D_OK;
}

HRESULT STDMETHODCALLTYPE Direct3DTexture8::AddDirtyRect(const RECT* pDirtyRect)
{
	NOT_IMPLEMENTED_RETURN;
}

void Direct3DTexture8::get_level_offset(UINT level, size_t& offset, size_t& size) const
{
	size = surfaces[level]->desc8.Size;
	offset = 0;

	for (UINT i = 0; i < level; ++i)
	{
		offset += surfaces[i]->desc8.Size;
	}
}

bool Direct3DTexture8::convert(UINT level)
{
	if (IsWindows8OrGreater())
	{
		return false;
	}

	D3DSURFACE_DESC8 level_desc {};
	GetLevelDesc(level, &level_desc);

	size_t level_offset = 0;
	size_t level_size = 0;
	get_level_offset(level, level_offset, level_size);

	uint8_t* buffer = &texture_buffer[level_offset];
	
	const DXGI_FORMAT format = to_dxgi(format_);

	std::vector<uint32_t> rgba;

	switch (format)
	{
		case DXGI_FORMAT_B5G6R5_UNORM:
		{
			rgba.resize(level_size / 2);

			auto b16            = reinterpret_cast<uint16_t*>(buffer);
			uint32_t* b32       = rgba.data();
			const size_t length = rgba.size();

			for (size_t i = 0; i < length; ++i)
			{
				auto p16  = b16[i];
				auto& p32 = b32[i];

				auto b = static_cast<uint8_t>(static_cast<float>(p16 & 0b011111) / 32.0f * 255.0f);
				auto g = static_cast<uint8_t>(static_cast<float>((p16 >> 5) & 0b111111) / 64.0f * 255.0f);
				auto r = static_cast<uint8_t>(static_cast<float>((p16 >> 11) & 0b011111) / 32.0f * 255.0f);

				p32 = 255 << 24 | b << 16 | g << 8 | r;
			}
			break;
		}

		case DXGI_FORMAT_B5G5R5A1_UNORM:
		{
			rgba.resize(level_size / 2);

			auto b16      = reinterpret_cast<uint16_t*>(buffer);
			uint32_t* b32 = rgba.data();
			auto length   = rgba.size();

			for (size_t i = 0; i < length; ++i)
			{
				auto p16  = b16[i];
				auto& p32 = b32[i];

				auto b = static_cast<uint8_t>(static_cast<float>(p16 & 0b011111) / 32.0f * 255.0f);
				auto g = static_cast<uint8_t>(static_cast<float>((p16 >> 5) & 0b011111) / 32.0f * 255.0f);
				auto r = static_cast<uint8_t>(static_cast<float>((p16 >> 10) & 0b011111) / 32.0f * 255.0f);
				auto a = static_cast<uint8_t>(p16 & (1 << 15) ? 255 : 0);

				p32 = a << 24 | b << 16 | g << 8 | r;
			}
			break;
		}

		case DXGI_FORMAT_B4G4R4A4_UNORM:
		{
			rgba.resize(level_size / 2);

			auto b16      = reinterpret_cast<uint16_t*>(buffer);
			uint32_t* b32 = rgba.data();
			auto length   = rgba.size();

			for (size_t i = 0; i < length; ++i)
			{
				auto p16  = b16[i];
				auto& p32 = b32[i];

				auto b = static_cast<uint8_t>(static_cast<float>(p16 & 0xF) / 15.0f * 255.0f);
				auto g = static_cast<uint8_t>(static_cast<float>((p16 >> 4) & 0xF) / 15.0f * 255.0f);
				auto r = static_cast<uint8_t>(static_cast<float>((p16 >> 8) & 0xF) / 15.0f * 255.0f);
				auto a = static_cast<uint8_t>(static_cast<float>((p16 >> 12) & 0xF) / 15.0f * 255.0f);

				p32 = a << 24 | b << 16 | g << 8 | r;
			}
			break;
		}

		case DXGI_FORMAT_B8G8R8A8_UNORM:
		{
			rgba.resize(level_size / 4);

			auto bgr      = reinterpret_cast<uint32_t*>(buffer);
			uint32_t* b32 = rgba.data();
			auto length   = rgba.size();

			for (size_t i = 0; i < length; ++i)
			{
				auto a  = bgr[i];
				auto& b = b32[i];

				b = (a & 0xFF000000) | ((a >> 16) & 0xFF) | ((a >> 8) & 0xFF) << 8 | (a & 0xFF) << 16;
			}

			break;
		}

		default:
			return false;
	}

	device8->context->UpdateSubresource(texture.Get(), level, nullptr, rgba.data(), 4 * level_desc.Width, 0);
	return true;
}

bool Direct3DTexture8::should_convert() const
{
	if (IsWindows8OrGreater())
	{
		return false;
	}

	const DXGI_FORMAT format = to_dxgi(format_);

	switch (format)
	{
		case DXGI_FORMAT_B5G6R5_UNORM:
		case DXGI_FORMAT_B5G5R5A1_UNORM:
		case DXGI_FORMAT_B4G4R4A4_UNORM:
		case DXGI_FORMAT_B8G8R8A8_UNORM:
			return true;

		default:
			return false;
	}
}

// TODO: IDirect3DCubeTexture8

#if 0

Direct3DCubeTexture8::Direct3DCubeTexture8(Direct3DDevice8* device, IDirect3DCubeTexture9* ProxyInterface)
	: ProxyInterface(ProxyInterface),
	Device(device)
{
	Device->address_table->SaveAddress(this, ProxyInterface);
}

Direct3DCubeTexture8::~Direct3DCubeTexture8()
{
}

HRESULT STDMETHODCALLTYPE Direct3DCubeTexture8::QueryInterface(REFIID riid, void** ppvObj)
{
	if (ppvObj == nullptr)
	{
		return E_POINTER;
	}

	if (riid == __uuidof(this) ||
		riid == __uuidof(IUnknown) ||
		riid == __uuidof(Direct3DResource8) ||
		riid == __uuidof(Direct3DBaseTexture8))
	{
		AddRef();

		*ppvObj = this;

		return S_OK;
	}

	return ProxyInterface->QueryInterface(riid, ppvObj);
}

ULONG STDMETHODCALLTYPE Direct3DCubeTexture8::AddRef()
{
	return ProxyInterface->AddRef();
}

ULONG STDMETHODCALLTYPE Direct3DCubeTexture8::Release()
{
	return ProxyInterface->Release();
}

HRESULT STDMETHODCALLTYPE Direct3DCubeTexture8::GetDevice(Direct3DDevice8** ppDevice)
{
	if (ppDevice == nullptr)
	{
		return D3DERR_INVALIDCALL;
	}

	Device->AddRef();

	*ppDevice = Device;

	return D3D_OK;
}

HRESULT STDMETHODCALLTYPE Direct3DCubeTexture8::SetPrivateData(REFGUID refguid, const void* pData, DWORD SizeOfData, DWORD Flags)
{
	return ProxyInterface->SetPrivateData(refguid, pData, SizeOfData, Flags);
}

HRESULT STDMETHODCALLTYPE Direct3DCubeTexture8::GetPrivateData(REFGUID refguid, void* pData, DWORD* pSizeOfData)
{
	return ProxyInterface->GetPrivateData(refguid, pData, pSizeOfData);
}

HRESULT STDMETHODCALLTYPE Direct3DCubeTexture8::FreePrivateData(REFGUID refguid)
{
	return ProxyInterface->FreePrivateData(refguid);
}

DWORD STDMETHODCALLTYPE Direct3DCubeTexture8::SetPriority(DWORD PriorityNew)
{
	return ProxyInterface->SetPriority(PriorityNew);
}

DWORD STDMETHODCALLTYPE Direct3DCubeTexture8::GetPriority()
{
	return ProxyInterface->GetPriority();
}

void STDMETHODCALLTYPE Direct3DCubeTexture8::PreLoad()
{
	ProxyInterface->PreLoad();
}

D3DRESOURCETYPE STDMETHODCALLTYPE Direct3DCubeTexture8::GetType()
{
	return D3DRTYPE_CUBETEXTURE;
}

DWORD STDMETHODCALLTYPE Direct3DCubeTexture8::SetLOD(DWORD LODNew)
{
	return ProxyInterface->SetLOD(LODNew);
}

DWORD STDMETHODCALLTYPE Direct3DCubeTexture8::GetLOD()
{
	return ProxyInterface->GetLOD();
}

DWORD STDMETHODCALLTYPE Direct3DCubeTexture8::GetLevelCount()
{
	return ProxyInterface->GetLevelCount();
}

HRESULT STDMETHODCALLTYPE Direct3DCubeTexture8::GetLevelDesc(UINT Level, D3DSURFACE_DESC8* pDesc)
{
	if (pDesc == nullptr)
	{
		return D3DERR_INVALIDCALL;
	}

	D3DSURFACE_DESC SurfaceDesc;

	const HRESULT hr = ProxyInterface->GetLevelDesc(Level, &SurfaceDesc);

	if (FAILED(hr))
	{
		return hr;
	}

	ConvertSurfaceDesc(SurfaceDesc, *pDesc);

	return D3D_OK;
}

HRESULT STDMETHODCALLTYPE Direct3DCubeTexture8::GetCubeMapSurface(D3DCUBEMAP_FACES FaceType, UINT Level, Direct3DSurface8** ppCubeMapSurface)
{
	if (ppCubeMapSurface == nullptr)
	{
		return D3DERR_INVALIDCALL;
	}

	*ppCubeMapSurface = nullptr;

	IDirect3DSurface9* SurfaceInterface = nullptr;

	const HRESULT hr = ProxyInterface->GetCubeMapSurface(FaceType, Level, &SurfaceInterface);

	if (FAILED(hr))
	{
		return hr;
	}

	*ppCubeMapSurface = Device->address_table->FindAddress<Direct3DSurface8>(SurfaceInterface);

	return D3D_OK;
}

HRESULT STDMETHODCALLTYPE Direct3DCubeTexture8::LockRect(D3DCUBEMAP_FACES FaceType, UINT Level, D3DLOCKED_RECT* pLockedRect, const RECT* pRect, DWORD Flags)
{
	return ProxyInterface->LockRect(FaceType, Level, pLockedRect, pRect, Flags);
}

HRESULT STDMETHODCALLTYPE Direct3DCubeTexture8::UnlockRect(D3DCUBEMAP_FACES FaceType, UINT Level)
{
	return ProxyInterface->UnlockRect(FaceType, Level);
}

HRESULT STDMETHODCALLTYPE Direct3DCubeTexture8::AddDirtyRect(D3DCUBEMAP_FACES FaceType, const RECT* pDirtyRect)
{
	return ProxyInterface->AddDirtyRect(FaceType, pDirtyRect);
}

#endif

// TODO: IDirect3DVolumeTexture8

#if 0

Direct3DVolumeTexture8::Direct3DVolumeTexture8(Direct3DDevice8 *device, IDirect3DVolumeTexture9 *ProxyInterface) :
	ProxyInterface(ProxyInterface),
	Device(device)
{
	Device->address_table->SaveAddress(this, ProxyInterface);
}

Direct3DVolumeTexture8::~Direct3DVolumeTexture8()
{
}

HRESULT STDMETHODCALLTYPE Direct3DVolumeTexture8::QueryInterface(REFIID riid, void **ppvObj)
{
	if (ppvObj == nullptr)
	{
		return E_POINTER;
	}

	if (riid == __uuidof(this) ||
		riid == __uuidof(IUnknown) ||
		riid == __uuidof(Direct3DResource8) ||
		riid == __uuidof(Direct3DBaseTexture8))
	{
		AddRef();

		*ppvObj = this;

		return S_OK;
	}

	return ProxyInterface->QueryInterface(riid, ppvObj);
}

ULONG STDMETHODCALLTYPE Direct3DVolumeTexture8::AddRef()
{
	return ProxyInterface->AddRef();
}

ULONG STDMETHODCALLTYPE Direct3DVolumeTexture8::Release()
{
	return ProxyInterface->Release();
}

HRESULT STDMETHODCALLTYPE Direct3DVolumeTexture8::GetDevice(Direct3DDevice8 **ppDevice)
{
	if (ppDevice == nullptr)
	{
		return D3DERR_INVALIDCALL;
	}

	Device->AddRef();

	*ppDevice = Device;

	return D3D_OK;
}

HRESULT STDMETHODCALLTYPE Direct3DVolumeTexture8::SetPrivateData(REFGUID refguid, const void *pData, DWORD SizeOfData, DWORD Flags)
{
	return ProxyInterface->SetPrivateData(refguid, pData, SizeOfData, Flags);
}

HRESULT STDMETHODCALLTYPE Direct3DVolumeTexture8::GetPrivateData(REFGUID refguid, void *pData, DWORD *pSizeOfData)
{
	return ProxyInterface->GetPrivateData(refguid, pData, pSizeOfData);
}

HRESULT STDMETHODCALLTYPE Direct3DVolumeTexture8::FreePrivateData(REFGUID refguid)
{
	return ProxyInterface->FreePrivateData(refguid);
}

DWORD STDMETHODCALLTYPE Direct3DVolumeTexture8::SetPriority(DWORD PriorityNew)
{
	return ProxyInterface->SetPriority(PriorityNew);
}

DWORD STDMETHODCALLTYPE Direct3DVolumeTexture8::GetPriority()
{
	return ProxyInterface->GetPriority();
}

void STDMETHODCALLTYPE Direct3DVolumeTexture8::PreLoad()
{
	ProxyInterface->PreLoad();
}

D3DRESOURCETYPE STDMETHODCALLTYPE Direct3DVolumeTexture8::GetType()
{
	return D3DRTYPE_VOLUMETEXTURE;
}

DWORD STDMETHODCALLTYPE Direct3DVolumeTexture8::SetLOD(DWORD LODNew)
{
	return ProxyInterface->SetLOD(LODNew);
}

DWORD STDMETHODCALLTYPE Direct3DVolumeTexture8::GetLOD()
{
	return ProxyInterface->GetLOD();
}

DWORD STDMETHODCALLTYPE Direct3DVolumeTexture8::GetLevelCount()
{
	return ProxyInterface->GetLevelCount();
}

HRESULT STDMETHODCALLTYPE Direct3DVolumeTexture8::GetLevelDesc(UINT Level, D3DVOLUME_DESC8 *pDesc)
{
	if (pDesc == nullptr)
	{
		return D3DERR_INVALIDCALL;
	}

	D3DVOLUME_DESC VolumeDesc;

	const HRESULT hr = ProxyInterface->GetLevelDesc(Level, &VolumeDesc);

	if (FAILED(hr))
	{
		return hr;
	}

	ConvertVolumeDesc(VolumeDesc, *pDesc);

	return D3D_OK;
}

HRESULT STDMETHODCALLTYPE Direct3DVolumeTexture8::GetVolumeLevel(UINT Level, Direct3DVolume8 **ppVolumeLevel)
{
	if (ppVolumeLevel == nullptr)
	{
		return D3DERR_INVALIDCALL;
	}

	*ppVolumeLevel = nullptr;

	IDirect3DVolume9 *VolumeInterface = nullptr;

	const HRESULT hr = ProxyInterface->GetVolumeLevel(Level, &VolumeInterface);

	if (FAILED(hr))
	{
		return hr;
	}

	*ppVolumeLevel = Device->address_table->FindAddress<Direct3DVolume8>(VolumeInterface);

	return D3D_OK;
}

HRESULT STDMETHODCALLTYPE Direct3DVolumeTexture8::LockBox(UINT Level, D3DLOCKED_BOX *pLockedVolume, const D3DBOX *pBox, DWORD Flags)
{
	return ProxyInterface->LockBox(Level, pLockedVolume, pBox, Flags);
}

HRESULT STDMETHODCALLTYPE Direct3DVolumeTexture8::UnlockBox(UINT Level)
{
	return ProxyInterface->UnlockBox(Level);
}

HRESULT STDMETHODCALLTYPE Direct3DVolumeTexture8::AddDirtyBox(const D3DBOX *pDirtyBox)
{
	return ProxyInterface->AddDirtyBox(pDirtyBox);
}

#endif
