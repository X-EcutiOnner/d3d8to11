/**
 * Copyright (C) 2015 Patrick Mours. All rights reserved.
 * License: https://github.com/crosire/d3d8to9#license
 */

#include "stdafx.h"
#include "d3d8to11.hpp"

extern "C" Direct3D8* WINAPI Direct3DCreate8(UINT SDKVersion)
{
	auto result = new Direct3D8();
	result->AddRef();

	try
	{
		result->create_native();
		return result;
	}
	catch (std::exception& ex)
	{
		delete result;
		printf("%s\n", ex.what());
		return nullptr;
	}
}

namespace d3d8to11
{
	const std::filesystem::path storage_directory(".d3d8to11");

	const std::filesystem::path config_file_path      = storage_directory / "config.ini";
	const std::filesystem::path permutation_file_path = storage_directory / "permutations.bin";

	DXGI_FORMAT to_dxgi(D3DFORMAT value)
	{
		switch (static_cast<uint32_t>(value))
		{
			default:
				return DXGI_FORMAT_UNKNOWN;

			case D3DFMT_D24X8:
				return DXGI_FORMAT_R24_UNORM_X8_TYPELESS;

			case D3DFMT_D24S8:
				return DXGI_FORMAT_D24_UNORM_S8_UINT;

			case D3DFMT_D32:
				return DXGI_FORMAT_D32_FLOAT;

			case D3DFMT_A8R8G8B8:
				return DXGI_FORMAT_B8G8R8A8_UNORM;// & DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;

			case D3DFMT_X8R8G8B8:
				return DXGI_FORMAT_B8G8R8X8_UNORM; // & DXGI_FORMAT_B8G8R8X8_UNORM_SRGB;

			case D3DFMT_R5G6B5:
				return DXGI_FORMAT_B5G6R5_UNORM;

			case D3DFMT_A1R5G5B5:
				return DXGI_FORMAT_B5G5R5A1_UNORM;

			case D3DFMT_A4R4G4B4:
				return DXGI_FORMAT_B4G4R4A4_UNORM;

			case D3DFMT_A8:
				return DXGI_FORMAT_A8_UNORM;

			case D3DFMT_A2B10G10R10:
				return DXGI_FORMAT_R10G10B10A2_UNORM;

			case D3DFMT_G16R16:
				return DXGI_FORMAT_R16G16_UNORM;

			case D3DFMT_L8:
				return DXGI_FORMAT_R8_UNORM;

			case D3DFMT_A8L8:
				return DXGI_FORMAT_R8G8_UNORM;

			case D3DFMT_V8U8:
				return DXGI_FORMAT_R8G8_SNORM;

			case D3DFMT_Q8W8V8U8:
				return DXGI_FORMAT_R8G8B8A8_SNORM;

			case D3DFMT_V16U16:
				return DXGI_FORMAT_R16G16_SNORM;

			case D3DFMT_DXT1:
				return DXGI_FORMAT_BC1_UNORM; // & DXGI_FORMAT_BC1_UNORM_SRGB;

			case D3DFMT_DXT2:
				return DXGI_FORMAT_BC2_UNORM; // & DXGI_FORMAT_BC1_UNORM_SRGB;

			case D3DFMT_DXT3:
				return DXGI_FORMAT_BC2_UNORM; // & DXGI_FORMAT_BC2_UNORM_SRGB;

			case D3DFMT_DXT4:
				return DXGI_FORMAT_BC3_UNORM; // & DXGI_FORMAT_BC2_UNORM_SRGB;

			case D3DFMT_DXT5:
				return DXGI_FORMAT_BC3_UNORM; // & DXGI_FORMAT_BC3_UNORM_SRGB;

			case D3DFMT_D16:
			case D3DFMT_D16_LOCKABLE:
				return DXGI_FORMAT_D16_UNORM;

			case D3DFMT_INDEX16:
				return DXGI_FORMAT_R16_UINT;

			case D3DFMT_INDEX32:
				return DXGI_FORMAT_R32_UINT;

			case /*FourCC 'ATI1'*/ MAKEFOURCC('A', 'T', 'I', '1'):
				return DXGI_FORMAT_BC4_UNORM;

			case /*FourCC 'ATI2'*/ MAKEFOURCC('A', 'T', 'I', '2'):
				return DXGI_FORMAT_BC5_UNORM;
		}
	}

	D3DFORMAT to_d3d8(DXGI_FORMAT value)
	{
		switch (value)
		{
			default:
				return D3DFMT_UNKNOWN;

			case DXGI_FORMAT_B8G8R8A8_UNORM:
			case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
				return D3DFMT_A8R8G8B8;

			case DXGI_FORMAT_B8G8R8X8_UNORM:
			case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
				return D3DFMT_X8R8G8B8;

			case DXGI_FORMAT_B5G6R5_UNORM:
				return D3DFMT_R5G6B5;

			case DXGI_FORMAT_B5G5R5A1_UNORM:
				return D3DFMT_A1R5G5B5;

			case DXGI_FORMAT_B4G4R4A4_UNORM:
				return D3DFMT_A4R4G4B4;

			case DXGI_FORMAT_A8_UNORM:
				return D3DFMT_A8;

			case DXGI_FORMAT_R10G10B10A2_UNORM:
				return D3DFMT_A2B10G10R10;

			case DXGI_FORMAT_R16G16_UNORM:
				return D3DFMT_G16R16;

			case DXGI_FORMAT_R8_UNORM:
				return D3DFMT_L8;

			case DXGI_FORMAT_R8G8_UNORM:
				return D3DFMT_A8L8;

			case DXGI_FORMAT_R8G8_SNORM:
				return D3DFMT_V8U8;

			case DXGI_FORMAT_R8G8B8A8_SNORM:
				return D3DFMT_Q8W8V8U8;

			case DXGI_FORMAT_R16G16_SNORM:
				return D3DFMT_V16U16;

			case DXGI_FORMAT_BC1_UNORM:
			case DXGI_FORMAT_BC1_UNORM_SRGB:
				return D3DFMT_DXT1;

				/*case DXGI_FORMAT_BC1_UNORM:
				case DXGI_FORMAT_BC1_UNORM_SRGB:
				return D3DFMT_DXT2;*/

			case DXGI_FORMAT_BC2_UNORM:
			case DXGI_FORMAT_BC2_UNORM_SRGB:
				return D3DFMT_DXT3;

				/*case DXGI_FORMAT_BC2_UNORM:
				case DXGI_FORMAT_BC2_UNORM_SRGB:
				return D3DFMT_DXT4;*/

			case DXGI_FORMAT_BC3_UNORM:
			case DXGI_FORMAT_BC3_UNORM_SRGB:
				return D3DFMT_DXT5;

			case DXGI_FORMAT_D16_UNORM:
				return D3DFMT_D16;
				//return D3DFMT_D16_LOCKABLE;

			case DXGI_FORMAT_R16_UINT:
				return D3DFMT_INDEX16;

			case DXGI_FORMAT_R32_UINT:
				return D3DFMT_INDEX32;

			case DXGI_FORMAT_BC4_UNORM:
				return /*FourCC 'ATI1'*/ static_cast<D3DFORMAT>(MAKEFOURCC('A', 'T', 'I', '1'));

			case DXGI_FORMAT_BC5_UNORM:
				return /*FourCC 'ATI2'*/ static_cast<D3DFORMAT>(MAKEFOURCC('A', 'T', 'I', '2'));
		}
	}

	D3D11_PRIMITIVE_TOPOLOGY to_d3d11(D3DPRIMITIVETYPE value)
	{
		switch (value)
		{
			case D3DPT_POINTLIST:
				return D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;
			case D3DPT_LINELIST:
				return D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
			case D3DPT_LINESTRIP:
				return D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP;
			case D3DPT_TRIANGLELIST:
				return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
			case D3DPT_TRIANGLESTRIP:
				return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
			case D3DPT_TRIANGLEFAN:
				return D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;
			default:
				return D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;
		}
	}

	DXGI_FORMAT to_typeless(DXGI_FORMAT format)
	{
		switch (format)
		{
			case DXGI_FORMAT_UNKNOWN:
				break;

			case DXGI_FORMAT_R32G32B32A32_TYPELESS:
				break;

			case DXGI_FORMAT_R32G32B32A32_FLOAT:
			case DXGI_FORMAT_R32G32B32A32_UINT:
			case DXGI_FORMAT_R32G32B32A32_SINT:
				return DXGI_FORMAT_R32G32B32A32_TYPELESS;

			case DXGI_FORMAT_R32G32B32_TYPELESS:
				break;

			case DXGI_FORMAT_R32G32B32_FLOAT:
			case DXGI_FORMAT_R32G32B32_UINT:
			case DXGI_FORMAT_R32G32B32_SINT:
				return DXGI_FORMAT_R32G32B32_TYPELESS;

			case DXGI_FORMAT_R16G16B16A16_TYPELESS:
				break;

			case DXGI_FORMAT_R16G16B16A16_FLOAT:
			case DXGI_FORMAT_R16G16B16A16_UNORM:
			case DXGI_FORMAT_R16G16B16A16_UINT:
			case DXGI_FORMAT_R16G16B16A16_SNORM:
			case DXGI_FORMAT_R16G16B16A16_SINT:
				return DXGI_FORMAT_R16G16B16A16_TYPELESS;

			case DXGI_FORMAT_R32G32_TYPELESS:
				break;

			case DXGI_FORMAT_R32G32_FLOAT:
			case DXGI_FORMAT_R32G32_UINT:
			case DXGI_FORMAT_R32G32_SINT:
				return DXGI_FORMAT_R32G32_TYPELESS;

			case DXGI_FORMAT_R32G8X24_TYPELESS:
				break;

			case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
				return DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS;

			case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
			case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
				return DXGI_FORMAT_R32G8X24_TYPELESS;

			case DXGI_FORMAT_R10G10B10A2_TYPELESS:
				break;

			case DXGI_FORMAT_R10G10B10A2_UNORM:
			case DXGI_FORMAT_R10G10B10A2_UINT:
				return DXGI_FORMAT_R10G10B10A2_TYPELESS;

			case DXGI_FORMAT_R8G8B8A8_TYPELESS:
				break;

			case DXGI_FORMAT_R8G8B8A8_UNORM:
			case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
			case DXGI_FORMAT_R8G8B8A8_UINT:
			case DXGI_FORMAT_R8G8B8A8_SNORM:
			case DXGI_FORMAT_R8G8B8A8_SINT:
				return DXGI_FORMAT_R8G8B8A8_TYPELESS;

			case DXGI_FORMAT_R16G16_TYPELESS:
				break;

			case DXGI_FORMAT_R16G16_FLOAT:
			case DXGI_FORMAT_R16G16_UNORM:
			case DXGI_FORMAT_R16G16_UINT:
			case DXGI_FORMAT_R16G16_SNORM:
			case DXGI_FORMAT_R16G16_SINT:
				return DXGI_FORMAT_R16G16_FLOAT;

			case DXGI_FORMAT_R32_TYPELESS:
				break;

			case DXGI_FORMAT_D32_FLOAT:
			case DXGI_FORMAT_R32_FLOAT:
			case DXGI_FORMAT_R32_UINT:
			case DXGI_FORMAT_R32_SINT:
				return DXGI_FORMAT_R32_TYPELESS;

			case DXGI_FORMAT_R24G8_TYPELESS:
				break;

			case DXGI_FORMAT_D24_UNORM_S8_UINT:
			case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
			case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
				return DXGI_FORMAT_R24G8_TYPELESS;

			case DXGI_FORMAT_R8G8_TYPELESS:
				break;

			case DXGI_FORMAT_R8G8_UNORM:
			case DXGI_FORMAT_R8G8_UINT:
			case DXGI_FORMAT_R8G8_SNORM:
			case DXGI_FORMAT_R8G8_SINT:
				return DXGI_FORMAT_R8G8_TYPELESS;

			case DXGI_FORMAT_R16_TYPELESS:
				break;

			case DXGI_FORMAT_R16_FLOAT:
			case DXGI_FORMAT_D16_UNORM:
			case DXGI_FORMAT_R16_UNORM:
			case DXGI_FORMAT_R16_UINT:
			case DXGI_FORMAT_R16_SNORM:
			case DXGI_FORMAT_R16_SINT:
				return DXGI_FORMAT_R16_TYPELESS;

			case DXGI_FORMAT_R8_TYPELESS:
				break;

			case DXGI_FORMAT_R8_UNORM:
			case DXGI_FORMAT_R8_UINT:
			case DXGI_FORMAT_R8_SNORM:
			case DXGI_FORMAT_R8_SINT:
			case DXGI_FORMAT_A8_UNORM:
			case DXGI_FORMAT_R1_UNORM:
				return DXGI_FORMAT_R8_TYPELESS;

			case DXGI_FORMAT_BC1_TYPELESS:
				break;

			case DXGI_FORMAT_BC1_UNORM:
			case DXGI_FORMAT_BC1_UNORM_SRGB:
				return DXGI_FORMAT_BC1_TYPELESS;

			case DXGI_FORMAT_BC2_TYPELESS:
				break;

			case DXGI_FORMAT_BC2_UNORM:
			case DXGI_FORMAT_BC2_UNORM_SRGB:
				return DXGI_FORMAT_BC2_TYPELESS;

			case DXGI_FORMAT_BC3_TYPELESS:
				break;

			case DXGI_FORMAT_BC3_UNORM:
			case DXGI_FORMAT_BC3_UNORM_SRGB:
				return DXGI_FORMAT_BC3_TYPELESS;

			case DXGI_FORMAT_BC4_TYPELESS:
				break;

			case DXGI_FORMAT_BC4_UNORM:
			case DXGI_FORMAT_BC4_SNORM:
				return DXGI_FORMAT_BC4_TYPELESS;

			case DXGI_FORMAT_BC5_TYPELESS:
				break;

			case DXGI_FORMAT_BC5_UNORM:
			case DXGI_FORMAT_BC5_SNORM:
				return DXGI_FORMAT_BC5_TYPELESS;

			case DXGI_FORMAT_B8G8R8A8_TYPELESS:
				break;

			case DXGI_FORMAT_B8G8R8A8_UNORM:
			case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
				return DXGI_FORMAT_B8G8R8A8_TYPELESS;

			case DXGI_FORMAT_B8G8R8X8_TYPELESS:
				break;

			case DXGI_FORMAT_B8G8R8X8_UNORM:
			case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
				return DXGI_FORMAT_B8G8R8X8_TYPELESS;

			case DXGI_FORMAT_BC6H_TYPELESS:
				break;

			case DXGI_FORMAT_BC6H_UF16:
			case DXGI_FORMAT_BC6H_SF16:
				return DXGI_FORMAT_BC6H_TYPELESS;

			case DXGI_FORMAT_BC7_TYPELESS:
				break;

			case DXGI_FORMAT_BC7_UNORM:
			case DXGI_FORMAT_BC7_UNORM_SRGB:
				return DXGI_FORMAT_BC7_TYPELESS;

			default:
				return DXGI_FORMAT_UNKNOWN;
		}

		return format;
	}

	DXGI_FORMAT typeless_to_depth(DXGI_FORMAT format)
	{
		switch (format)
		{
			case DXGI_FORMAT_UNKNOWN:
				break;

			case DXGI_FORMAT_R32G8X24_TYPELESS:
				return DXGI_FORMAT_D32_FLOAT_S8X24_UINT;

			case DXGI_FORMAT_R32_TYPELESS:
				return DXGI_FORMAT_D32_FLOAT;

			case DXGI_FORMAT_R24G8_TYPELESS:
				return DXGI_FORMAT_D24_UNORM_S8_UINT;

			case DXGI_FORMAT_R16_TYPELESS:
				return DXGI_FORMAT_D16_UNORM;

			default:
				return DXGI_FORMAT_UNKNOWN;
		}

		return format;
	}

	DXGI_FORMAT typeless_to_uint(DXGI_FORMAT format)
	{
		switch (format)
		{
			case DXGI_FORMAT_R32G32B32A32_TYPELESS:
				return DXGI_FORMAT_R32G32B32A32_UINT;
			case DXGI_FORMAT_R32G32B32A32_UINT:
				break;
			case DXGI_FORMAT_R32G32B32_TYPELESS:
				return DXGI_FORMAT_R32G32B32_UINT;
			case DXGI_FORMAT_R32G32B32_UINT:
				break;
			case DXGI_FORMAT_R16G16B16A16_TYPELESS:
				return DXGI_FORMAT_R16G16B16A16_UINT;
			case DXGI_FORMAT_R16G16B16A16_UINT:
				break;
			case DXGI_FORMAT_R32G32_TYPELESS:
				return DXGI_FORMAT_R32G32_UINT;
			case DXGI_FORMAT_R32G32_UINT:
				break;
			case DXGI_FORMAT_R10G10B10A2_TYPELESS:
				return DXGI_FORMAT_R10G10B10A2_UINT;
			case DXGI_FORMAT_R10G10B10A2_UINT:
				break;
			case DXGI_FORMAT_R8G8B8A8_TYPELESS:
				return DXGI_FORMAT_R8G8B8A8_UINT;
			case DXGI_FORMAT_R8G8B8A8_UINT:
				break;
			case DXGI_FORMAT_R16G16_TYPELESS:
				return DXGI_FORMAT_R16G16_UINT;
			case DXGI_FORMAT_R16G16_UINT:
				break;
			case DXGI_FORMAT_R32_TYPELESS:
				return DXGI_FORMAT_R32_UINT;
			case DXGI_FORMAT_R32_UINT:
				break;
			case DXGI_FORMAT_D24_UNORM_S8_UINT:
				break;
			case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
				return DXGI_FORMAT_D24_UNORM_S8_UINT;
			case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
				break;
			case DXGI_FORMAT_R8G8_TYPELESS:
				return DXGI_FORMAT_R8G8_UINT;
			case DXGI_FORMAT_R8G8_UINT:
				break;
			case DXGI_FORMAT_R16_TYPELESS:
				return DXGI_FORMAT_R16_UINT;
			case DXGI_FORMAT_R16_UINT:
				break;
			case DXGI_FORMAT_R8_TYPELESS:
				return DXGI_FORMAT_R8_UINT;
			case DXGI_FORMAT_R8_UINT:
				break;
			default:
				return DXGI_FORMAT_UNKNOWN;
		}

		return format;
	}

	DXGI_FORMAT typeless_to_sint(DXGI_FORMAT format)
	{
		switch (format)
		{
			case DXGI_FORMAT_R32G32B32A32_TYPELESS:
				return DXGI_FORMAT_R32G32B32A32_SINT;
			case DXGI_FORMAT_R32G32B32A32_SINT:
				break;
			case DXGI_FORMAT_R32G32B32_TYPELESS:
				return DXGI_FORMAT_R32G32B32_SINT;
			case DXGI_FORMAT_R32G32B32_SINT:
				break;
			case DXGI_FORMAT_R16G16B16A16_TYPELESS:
				return DXGI_FORMAT_R16G16B16A16_SINT;
			case DXGI_FORMAT_R16G16B16A16_SINT:
				break;
			case DXGI_FORMAT_R32G32_TYPELESS:
				return DXGI_FORMAT_R32G32_SINT;
			case DXGI_FORMAT_R32G32_SINT:
				break;
			case DXGI_FORMAT_R8G8B8A8_TYPELESS:
				return DXGI_FORMAT_R8G8B8A8_SINT;
			case DXGI_FORMAT_R8G8B8A8_SINT:
				break;
			case DXGI_FORMAT_R16G16_TYPELESS:
				return DXGI_FORMAT_R16G16_SINT;
			case DXGI_FORMAT_R16G16_SINT:
				break;
			case DXGI_FORMAT_R32_TYPELESS:
				return DXGI_FORMAT_R32_SINT;
			case DXGI_FORMAT_R32_SINT:
				break;
			case DXGI_FORMAT_R8G8_TYPELESS:
				return DXGI_FORMAT_R8G8_SINT;
			case DXGI_FORMAT_R8G8_SINT:
				break;
			case DXGI_FORMAT_R16_TYPELESS:
				return DXGI_FORMAT_R16_SINT;
			case DXGI_FORMAT_R16_SINT:
				break;
			case DXGI_FORMAT_R8_TYPELESS:
				return DXGI_FORMAT_R8_SINT;
			case DXGI_FORMAT_R8_SINT:
				break;
			default:
				return DXGI_FORMAT_UNKNOWN;
		}

		return format;
	}

	DXGI_FORMAT typeless_to_snorm(DXGI_FORMAT format)
	{
		switch (format)
		{
			case DXGI_FORMAT_R16G16B16A16_TYPELESS:
				return DXGI_FORMAT_R16G16B16A16_SNORM;
			case DXGI_FORMAT_R16G16B16A16_SNORM:
				break;
			case DXGI_FORMAT_R8G8B8A8_TYPELESS:
				return DXGI_FORMAT_R8G8B8A8_SNORM;
			case DXGI_FORMAT_R8G8B8A8_SNORM:
				break;
			case DXGI_FORMAT_R16G16_TYPELESS:
				return DXGI_FORMAT_R16G16_SNORM;
			case DXGI_FORMAT_R16G16_SNORM:
				break;
			case DXGI_FORMAT_R8G8_TYPELESS:
				return DXGI_FORMAT_R8G8_SNORM;
			case DXGI_FORMAT_R8G8_SNORM:
				break;
			case DXGI_FORMAT_R16_TYPELESS:
				return DXGI_FORMAT_R16_SNORM;
			case DXGI_FORMAT_R16_SNORM:
				break;
			case DXGI_FORMAT_R8_TYPELESS:
				return DXGI_FORMAT_R8_SNORM;
			case DXGI_FORMAT_R8_SNORM:
				break;
			case DXGI_FORMAT_BC4_TYPELESS:
				return DXGI_FORMAT_BC4_SNORM;
			case DXGI_FORMAT_BC4_SNORM:
				break;
			case DXGI_FORMAT_BC5_TYPELESS:
				return DXGI_FORMAT_BC5_SNORM;
			case DXGI_FORMAT_BC5_SNORM:
				break;
			default:
				return DXGI_FORMAT_UNKNOWN;
		}

		return format;
	}

	DXGI_FORMAT typeless_to_float(DXGI_FORMAT format)
	{
		{
			switch (format)
			{
				case DXGI_FORMAT_R24G8_TYPELESS:
					return DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
				case DXGI_FORMAT_R32G32B32A32_TYPELESS:
					return DXGI_FORMAT_R32G32B32A32_FLOAT;
				case DXGI_FORMAT_R32G32B32A32_FLOAT:
					break;
				case DXGI_FORMAT_R32G32B32_TYPELESS:
					return DXGI_FORMAT_R32G32B32_FLOAT;
				case DXGI_FORMAT_R32G32B32_FLOAT:
					break;
				case DXGI_FORMAT_R16G16B16A16_TYPELESS:
					return DXGI_FORMAT_R16G16B16A16_FLOAT;
				case DXGI_FORMAT_R16G16B16A16_FLOAT:
					break;
				case DXGI_FORMAT_R32G32_TYPELESS:
					return DXGI_FORMAT_R32G32_FLOAT;
				case DXGI_FORMAT_R32G32_FLOAT:
					break;
				case DXGI_FORMAT_R32G32_UINT:
					break;
				case DXGI_FORMAT_R32G32_SINT:
					break;
				case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
					break;
				case DXGI_FORMAT_R11G11B10_FLOAT:
					break;
				case DXGI_FORMAT_R16G16_TYPELESS:
					return DXGI_FORMAT_R16G16_FLOAT;
				case DXGI_FORMAT_R16G16_FLOAT:
					break;
				case DXGI_FORMAT_R32_TYPELESS:
					return DXGI_FORMAT_R32_FLOAT;
				case DXGI_FORMAT_D32_FLOAT:
					break;
				case DXGI_FORMAT_R32_FLOAT:
					break;
				case DXGI_FORMAT_R16_TYPELESS:
					return DXGI_FORMAT_R16_FLOAT;
				case DXGI_FORMAT_R16_FLOAT:
					break;
				default:
					return DXGI_FORMAT_UNKNOWN;
			}

			return format;
		}
	}

	DXGI_FORMAT typeless_to_unorm(DXGI_FORMAT format, bool srgb)
	{
		switch (format)
		{
			case DXGI_FORMAT_R16G16B16A16_TYPELESS:
				return srgb ? DXGI_FORMAT_UNKNOWN : DXGI_FORMAT_R16G16B16A16_UNORM;
			case DXGI_FORMAT_R16G16B16A16_UNORM:
				break;
			case DXGI_FORMAT_R10G10B10A2_TYPELESS:
				return srgb ? DXGI_FORMAT_UNKNOWN : DXGI_FORMAT_R10G10B10A2_UNORM;
			case DXGI_FORMAT_R10G10B10A2_UNORM:
				break;
			case DXGI_FORMAT_R8G8B8A8_TYPELESS:
				return srgb ? DXGI_FORMAT_R8G8B8A8_UNORM_SRGB : DXGI_FORMAT_R8G8B8A8_UNORM;
			case DXGI_FORMAT_R8G8B8A8_UNORM:
				break;
			case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
				break;
			case DXGI_FORMAT_R16G16_TYPELESS:
				return srgb ? DXGI_FORMAT_UNKNOWN : DXGI_FORMAT_R16G16_UNORM;
			case DXGI_FORMAT_R16G16_UNORM:
				break;
			case DXGI_FORMAT_R8G8_TYPELESS:
				return srgb ? DXGI_FORMAT_UNKNOWN : DXGI_FORMAT_R8G8_UNORM;
			case DXGI_FORMAT_R8G8_UNORM:
				break;
			case DXGI_FORMAT_R16_TYPELESS:
				return srgb ? DXGI_FORMAT_UNKNOWN : DXGI_FORMAT_R16_UNORM;
			case DXGI_FORMAT_D16_UNORM:
				break;
			case DXGI_FORMAT_R16_UNORM:
				break;
			case DXGI_FORMAT_R8_TYPELESS:
				return srgb ? DXGI_FORMAT_UNKNOWN : DXGI_FORMAT_R8_UNORM;
			case DXGI_FORMAT_R8_UNORM:
				break;
			case DXGI_FORMAT_A8_UNORM:
				break;
			case DXGI_FORMAT_R1_UNORM:
				break;
			case DXGI_FORMAT_R8G8_B8G8_UNORM:
				break;
			case DXGI_FORMAT_G8R8_G8B8_UNORM:
				break;
			case DXGI_FORMAT_BC1_TYPELESS:
				return srgb ? DXGI_FORMAT_BC1_UNORM_SRGB : DXGI_FORMAT_BC1_UNORM;
			case DXGI_FORMAT_BC1_UNORM:
				break;
			case DXGI_FORMAT_BC1_UNORM_SRGB:
				break;
			case DXGI_FORMAT_BC2_TYPELESS:
				return srgb ? DXGI_FORMAT_BC2_UNORM_SRGB : DXGI_FORMAT_BC2_UNORM;
			case DXGI_FORMAT_BC2_UNORM:
				break;
			case DXGI_FORMAT_BC2_UNORM_SRGB:
				break;
			case DXGI_FORMAT_BC3_TYPELESS:
				return srgb ? DXGI_FORMAT_BC3_UNORM_SRGB : DXGI_FORMAT_BC3_UNORM;
			case DXGI_FORMAT_BC3_UNORM:
				break;
			case DXGI_FORMAT_BC3_UNORM_SRGB:
				break;
			case DXGI_FORMAT_BC4_TYPELESS:
				return srgb ? DXGI_FORMAT_UNKNOWN : DXGI_FORMAT_BC4_UNORM;
			case DXGI_FORMAT_BC4_UNORM:
				break;
			case DXGI_FORMAT_BC5_TYPELESS:
				return srgb ? DXGI_FORMAT_UNKNOWN : DXGI_FORMAT_BC5_UNORM;
			case DXGI_FORMAT_BC5_UNORM:
				break;
			case DXGI_FORMAT_B5G6R5_UNORM:
				break;
			case DXGI_FORMAT_B5G5R5A1_UNORM:
				break;
			case DXGI_FORMAT_B8G8R8A8_UNORM:
				break;
			case DXGI_FORMAT_B8G8R8X8_UNORM:
				break;
			case DXGI_FORMAT_B8G8R8A8_TYPELESS:
				return srgb ? DXGI_FORMAT_B8G8R8A8_UNORM_SRGB : DXGI_FORMAT_B8G8R8A8_UNORM;
			case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
				break;
			case DXGI_FORMAT_B8G8R8X8_TYPELESS:
				return srgb ? DXGI_FORMAT_B8G8R8X8_UNORM_SRGB : DXGI_FORMAT_B8G8R8X8_UNORM;
			case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
				break;
			case DXGI_FORMAT_BC7_TYPELESS:
				return srgb ? DXGI_FORMAT_BC7_UNORM_SRGB : DXGI_FORMAT_BC7_UNORM;
			case DXGI_FORMAT_BC7_UNORM:
				break;
			case DXGI_FORMAT_BC7_UNORM_SRGB:
				break;
			case DXGI_FORMAT_B4G4R4A4_UNORM:
				break;
			case DXGI_FORMAT_D24_UNORM_S8_UINT:
				return DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
			default:
				return DXGI_FORMAT_UNKNOWN;
		}

		return format;
	}

	size_t dxgi_stride(DXGI_FORMAT format)
	{
		switch (format)
		{
			default:
				return 0;
			case DXGI_FORMAT_R32G32B32A32_TYPELESS:
			case DXGI_FORMAT_R32G32B32A32_FLOAT:
			case DXGI_FORMAT_R32G32B32A32_UINT:
			case DXGI_FORMAT_R32G32B32A32_SINT:
				return 16;
			case DXGI_FORMAT_R32G32B32_TYPELESS:
			case DXGI_FORMAT_R32G32B32_FLOAT:
			case DXGI_FORMAT_R32G32B32_UINT:
			case DXGI_FORMAT_R32G32B32_SINT:
				return 12;
			case DXGI_FORMAT_R16G16B16A16_TYPELESS:
			case DXGI_FORMAT_R16G16B16A16_FLOAT:
			case DXGI_FORMAT_R16G16B16A16_UNORM:
			case DXGI_FORMAT_R16G16B16A16_UINT:
			case DXGI_FORMAT_R16G16B16A16_SNORM:
			case DXGI_FORMAT_R16G16B16A16_SINT:
				return 8;
			case DXGI_FORMAT_R32G32_TYPELESS:
			case DXGI_FORMAT_R32G32_FLOAT:
			case DXGI_FORMAT_R32G32_UINT:
			case DXGI_FORMAT_R32G32_SINT:
			case DXGI_FORMAT_R32G8X24_TYPELESS:
			case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
			case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
			case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
				return 64;
			case DXGI_FORMAT_R10G10B10A2_TYPELESS:
			case DXGI_FORMAT_R10G10B10A2_UNORM:
			case DXGI_FORMAT_R10G10B10A2_UINT:
			case DXGI_FORMAT_R11G11B10_FLOAT:
			case DXGI_FORMAT_R8G8B8A8_TYPELESS:
			case DXGI_FORMAT_R8G8B8A8_UNORM:
			case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
			case DXGI_FORMAT_R8G8B8A8_UINT:
			case DXGI_FORMAT_R8G8B8A8_SNORM:
			case DXGI_FORMAT_R8G8B8A8_SINT:
			case DXGI_FORMAT_R16G16_TYPELESS:
			case DXGI_FORMAT_R16G16_FLOAT:
			case DXGI_FORMAT_R16G16_UNORM:
			case DXGI_FORMAT_R16G16_UINT:
			case DXGI_FORMAT_R16G16_SNORM:
			case DXGI_FORMAT_R16G16_SINT:
			case DXGI_FORMAT_R32_TYPELESS:
			case DXGI_FORMAT_D32_FLOAT:
			case DXGI_FORMAT_R32_FLOAT:
			case DXGI_FORMAT_R32_UINT:
			case DXGI_FORMAT_R32_SINT:
			case DXGI_FORMAT_R24G8_TYPELESS:
			case DXGI_FORMAT_D24_UNORM_S8_UINT:
			case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
			case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
				return 4;
			case DXGI_FORMAT_R8G8_TYPELESS:
			case DXGI_FORMAT_R8G8_UNORM:
			case DXGI_FORMAT_R8G8_UINT:
			case DXGI_FORMAT_R8G8_SNORM:
			case DXGI_FORMAT_R8G8_SINT:
			case DXGI_FORMAT_R16_TYPELESS:
			case DXGI_FORMAT_R16_FLOAT:
			case DXGI_FORMAT_D16_UNORM:
			case DXGI_FORMAT_R16_UNORM:
			case DXGI_FORMAT_R16_UINT:
			case DXGI_FORMAT_R16_SNORM:
			case DXGI_FORMAT_R16_SINT:
				return 2;
			case DXGI_FORMAT_R8_TYPELESS:
			case DXGI_FORMAT_R8_UNORM:
			case DXGI_FORMAT_R8_UINT:
			case DXGI_FORMAT_R8_SNORM:
			case DXGI_FORMAT_R8_SINT:
			case DXGI_FORMAT_A8_UNORM:
			case DXGI_FORMAT_R1_UNORM:
				return 1;
			case DXGI_FORMAT_R9G9B9E5_SHAREDEXP:
			case DXGI_FORMAT_R8G8_B8G8_UNORM:
			case DXGI_FORMAT_G8R8_G8B8_UNORM:
				return 4;
			case DXGI_FORMAT_B5G6R5_UNORM:
			case DXGI_FORMAT_B5G5R5A1_UNORM:
				return 2;
			case DXGI_FORMAT_B8G8R8A8_UNORM:
			case DXGI_FORMAT_B8G8R8X8_UNORM:
			case DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM:
			case DXGI_FORMAT_B8G8R8A8_TYPELESS:
			case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
			case DXGI_FORMAT_B8G8R8X8_TYPELESS:
			case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
				return 4;
			case DXGI_FORMAT_P8:
				return 1;
			case DXGI_FORMAT_A8P8:
			case DXGI_FORMAT_B4G4R4A4_UNORM:
				return 2;
		}
	}

	D3D11_FILTER to_d3d11(D3DTEXTUREFILTERTYPE min, D3DTEXTUREFILTERTYPE mag, D3DTEXTUREFILTERTYPE mip)
	{
		if (mag == D3DTEXF_NONE)
		{
			mag = D3DTEXF_POINT;
		}

		if (min == D3DTEXF_NONE)
		{
			min = D3DTEXF_POINT;
		}

		if (mip == D3DTEXF_NONE)
		{
			mip = D3DTEXF_POINT;
		}

		if (min == D3DTEXF_POINT && mag == D3DTEXF_POINT && mip == D3DTEXF_POINT)
		{
			return D3D11_FILTER_MIN_MAG_MIP_POINT;
		}

		if (min == D3DTEXF_POINT && mag == D3DTEXF_POINT && mip == D3DTEXF_LINEAR)
		{
			return D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR;
		}

		if (min == D3DTEXF_POINT && mag == D3DTEXF_LINEAR && mip == D3DTEXF_POINT)
		{
			return D3D11_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT;
		}

		if (min == D3DTEXF_POINT && mag == D3DTEXF_LINEAR && mip == D3DTEXF_LINEAR)
		{
			return D3D11_FILTER_MIN_POINT_MAG_MIP_LINEAR;
		}

		if (min == D3DTEXF_LINEAR && mag == D3DTEXF_POINT && mip == D3DTEXF_POINT)
		{
			return D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT;
		}

		if (min == D3DTEXF_LINEAR && mag == D3DTEXF_POINT && mip == D3DTEXF_LINEAR)
		{
			return D3D11_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
		}

		if (min == D3DTEXF_LINEAR && mag == D3DTEXF_LINEAR && mip == D3DTEXF_POINT)
		{
			return D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
		}

		if (min == D3DTEXF_ANISOTROPIC || mag == D3DTEXF_ANISOTROPIC || mip == D3DTEXF_ANISOTROPIC)
		{
			return D3D11_FILTER_ANISOTROPIC;
		}

		return D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	}

	bool is_block_compressed(DXGI_FORMAT value)
	{
		switch (value)
		{
			case DXGI_FORMAT_BC1_TYPELESS:
			case DXGI_FORMAT_BC1_UNORM:
			case DXGI_FORMAT_BC1_UNORM_SRGB:
			case DXGI_FORMAT_BC2_TYPELESS:
			case DXGI_FORMAT_BC2_UNORM:
			case DXGI_FORMAT_BC2_UNORM_SRGB:
			case DXGI_FORMAT_BC3_TYPELESS:
			case DXGI_FORMAT_BC3_UNORM:
			case DXGI_FORMAT_BC3_UNORM_SRGB:
			case DXGI_FORMAT_BC4_TYPELESS:
			case DXGI_FORMAT_BC4_UNORM:
			case DXGI_FORMAT_BC4_SNORM:
			case DXGI_FORMAT_BC5_TYPELESS:
			case DXGI_FORMAT_BC5_UNORM:
			case DXGI_FORMAT_BC5_SNORM:
			case DXGI_FORMAT_BC6H_TYPELESS:
			case DXGI_FORMAT_BC6H_UF16:
			case DXGI_FORMAT_BC6H_SF16:
			case DXGI_FORMAT_BC7_TYPELESS:
			case DXGI_FORMAT_BC7_UNORM:
			case DXGI_FORMAT_BC7_UNORM_SRGB:
				return true;

			default:
				return false;
		}
	}
}