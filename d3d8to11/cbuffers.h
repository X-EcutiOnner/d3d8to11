#pragma once

#include <cstdint>

#include "simple_math.h"
#include <dirty_t.h>
#include <CBufferWriter.h>
#include "Light.h"
#include "Material.h"
#include "defs.h"

class UberShaderFlagsBuffer final : public ICBuffer, dirty_impl
{
public:
	dirty_t<bool> rs_lighting;
	dirty_t<bool> rs_specular;
	dirty_t<bool> rs_alpha;
	dirty_t<bool> rs_alpha_test;
	dirty_t<bool> rs_fog;
	dirty_t<bool> rs_oit;
	dirty_t<uint32_t> rs_alpha_test_mode;
	dirty_t<uint32_t> rs_fog_mode;

	void write(CBufferBase& cbuff) const override;

	[[nodiscard]] bool dirty() const override;
	void clear() override;
	void mark() override;
};

class PerSceneBuffer final : public ICBuffer, dirty_impl
{
public:
	dirty_t<matrix, dirty_mode::on_assignment> view_matrix;
	dirty_t<matrix, dirty_mode::on_assignment> projection_matrix;
	dirty_t<float2> screen_dimensions;
	dirty_t<float3> view_position;
	dirty_t<uint32_t> oit_buffer_length;

	void write(CBufferBase& cbuff) const override;

	[[nodiscard]] bool dirty() const override;
	void clear() override;
	void mark() override;
};

class MaterialSources final : dirty_impl
{
public:
	dirty_t<uint32_t> diffuse;
	dirty_t<uint32_t> specular;
	dirty_t<uint32_t> ambient;
	dirty_t<uint32_t> emissive;

	[[nodiscard]] bool dirty() const override;
	void clear() override;
	void mark() override;
};

class PerModelBuffer final : public ICBuffer, dirty_impl
{
public:
	dirty_t<uint32_t> draw_call;

	dirty_t<matrix, dirty_mode::on_assignment> world_matrix;
	dirty_t<matrix, dirty_mode::on_assignment> wv_matrix_inv_t;
	std::array<dirty_t<Light>, LIGHT_COUNT>    lights;
	dirty_t<Material>                          material;
	MaterialSources                            material_sources;
	dirty_t<float4>                            ambient;
	dirty_t<bool>                              color_vertex;

	void write(CBufferBase& cbuff) const override;

	[[nodiscard]] bool dirty() const override;
	void clear() override;
	void mark() override;
};

class PerPixelBuffer final : public ICBuffer, dirty_impl
{
public:
	dirty_t<uint32_t> src_blend;
	dirty_t<uint32_t> dst_blend;
	dirty_t<uint32_t> blend_op;
	dirty_t<float>    fog_start;
	dirty_t<float>    fog_end;
	dirty_t<float>    fog_density;
	dirty_t<float4>   fog_color;
	dirty_t<float>    alpha_test_reference;
	dirty_t<float4>   texture_factor;

	void write(CBufferBase& cbuff) const override;

	[[nodiscard]] bool dirty() const override;
	void clear() override;
	void mark() override;
};

struct TextureStage final : dirty_impl
{
	dirty_t<bool>                            bound;
	dirty_t<matrix, dirty_mode::until_dirty> transform;
	dirty_t<D3DTEXTUREOP>                    color_op;
	dirty_t<uint32_t>                        color_arg1; // D3DTA
	dirty_t<uint32_t>                        color_arg2; // D3DTA
	dirty_t<D3DTEXTUREOP>                    alpha_op;
	dirty_t<uint32_t>                        alpha_arg1; // D3DTA
	dirty_t<uint32_t>                        alpha_arg2; // D3DTA
	dirty_t<float>                           bump_env_mat00;
	dirty_t<float>                           bump_env_mat01;
	dirty_t<float>                           bump_env_mat10;
	dirty_t<float>                           bump_env_mat11;
	dirty_t<uint32_t>                        tex_coord_index; // D3DTSS_TCI
	dirty_t<float>                           bump_env_lscale;
	dirty_t<float>                           bump_env_loffset;
	dirty_t<D3DTEXTURETRANSFORMFLAGS>        texture_transform_flags;
	dirty_t<uint32_t>                        color_arg0; // D3DTA
	dirty_t<uint32_t>                        alpha_arg0; // D3DTA
	dirty_t<uint32_t>                        result_arg; // D3DTA_CURRENT or D3DTA_TEMP

	[[nodiscard]] bool dirty() const override;
	void clear() override;
	void mark() override;
};

class TextureStages final : public ICBuffer, dirty_impl
{
public:
	std::array<TextureStage, TEXTURE_STAGE_MAX> stages {};
	void write(CBufferBase& cbuff) const override;
	[[nodiscard]] bool dirty() const override;
	void clear() override;
	void mark() override;
};
