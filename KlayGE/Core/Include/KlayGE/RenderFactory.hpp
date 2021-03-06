// RenderFactory.hpp
// KlayGE 渲染工厂类 头文件
// Ver 3.10.0
// 版权所有(C) 龚敏敏, 2003-2010
// Homepage: http://www.klayge.org
//
// 3.10.0
// 支持Cube和3D RenderView (2010.4.24)
//
// 3.8.0
// 增加了MakeSamplerStateObject (2008.9.21)
// 增加了MakeConditionalRender (2008.10.11)
// 支持depth texture (2008.11.6)
//
// 3.0.0
// 增加了MakeVertexBuffer (2005.9.7)
//
// 2.8.0
// 增加了LoadEffect (2005.7.25)
//
// 2.4.0
// 增加了1D/2D/3D/cube的支持 (2005.3.8)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#ifndef _RENDERFACTORY_HPP
#define _RENDERFACTORY_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/Texture.hpp>
#include <KlayGE/GraphicsBuffer.hpp>
#include <KlayGE/RenderStateObject.hpp>
#include <KFL/ArrayRef.hpp>

#include <string>
#include <unordered_map>

namespace KlayGE
{
	class KLAYGE_CORE_API RenderFactory : boost::noncopyable
	{
	public:
		virtual ~RenderFactory();

		virtual std::wstring const & Name() const = 0;

		RenderEngine& RenderEngineInstance();

		void Suspend();
		void Resume();

		virtual TexturePtr MakeDelayCreationTexture1D(uint32_t width, uint32_t num_mip_maps, uint32_t array_size,
			ElementFormat format, uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint) = 0;
		virtual TexturePtr MakeDelayCreationTexture2D(uint32_t width, uint32_t height, uint32_t num_mip_maps, uint32_t array_size,
			ElementFormat format, uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint) = 0;
		virtual TexturePtr MakeDelayCreationTexture3D(uint32_t width, uint32_t height, uint32_t depth, uint32_t num_mip_maps, uint32_t array_size,
			ElementFormat format, uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint) = 0;
		virtual TexturePtr MakeDelayCreationTextureCube(uint32_t size, uint32_t num_mip_maps, uint32_t array_size,
			ElementFormat format, uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint) = 0;

		TexturePtr MakeTexture1D(uint32_t width, uint32_t num_mip_maps, uint32_t array_size,
			ElementFormat format, uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint,
			ArrayRef<ElementInitData> init_data = {}, float4 const * clear_value_hint = nullptr);
		TexturePtr MakeTexture2D(uint32_t width, uint32_t height, uint32_t num_mip_maps, uint32_t array_size,
			ElementFormat format, uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint,
			ArrayRef<ElementInitData> init_data = {}, float4 const * clear_value_hint = nullptr);
		TexturePtr MakeTexture3D(uint32_t width, uint32_t height, uint32_t depth, uint32_t num_mip_maps, uint32_t array_size,
			ElementFormat format, uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint,
			ArrayRef<ElementInitData> init_data = {}, float4 const * clear_value_hint = nullptr);
		TexturePtr MakeTextureCube(uint32_t size, uint32_t num_mip_maps, uint32_t array_size,
			ElementFormat format, uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint,
			ArrayRef<ElementInitData> init_data = {}, float4 const * clear_value_hint = nullptr);

		virtual FrameBufferPtr MakeFrameBuffer() = 0;

		virtual RenderLayoutPtr MakeRenderLayout() = 0;

		virtual GraphicsBufferPtr MakeDelayCreationVertexBuffer(BufferUsage usage, uint32_t access_hint, uint32_t size_in_byte, ElementFormat fmt = EF_Unknown) = 0;
		virtual GraphicsBufferPtr MakeDelayCreationIndexBuffer(BufferUsage usage, uint32_t access_hint, uint32_t size_in_byte, ElementFormat fmt = EF_Unknown) = 0;
		virtual GraphicsBufferPtr MakeDelayCreationConstantBuffer(BufferUsage usage, uint32_t access_hint, uint32_t size_in_byte, ElementFormat fmt = EF_Unknown) = 0;

		GraphicsBufferPtr MakeVertexBuffer(BufferUsage usage, uint32_t access_hint, uint32_t size_in_byte, void const * init_data, ElementFormat fmt = EF_Unknown);
		GraphicsBufferPtr MakeIndexBuffer(BufferUsage usage, uint32_t access_hint, uint32_t size_in_byte, void const * init_data, ElementFormat fmt = EF_Unknown);
		GraphicsBufferPtr MakeConstantBuffer(BufferUsage usage, uint32_t access_hint, uint32_t size_in_byte, void const * init_data, ElementFormat fmt = EF_Unknown);

		virtual QueryPtr MakeOcclusionQuery() = 0;
		virtual QueryPtr MakeConditionalRender() = 0;
		virtual QueryPtr MakeTimerQuery() = 0;
		virtual QueryPtr MakeSOStatisticsQuery() = 0;

		virtual FencePtr MakeFence() = 0;

		virtual RenderViewPtr Make1DRenderView(Texture& texture, int first_array_index, int array_size, int level) = 0;
		virtual RenderViewPtr Make2DRenderView(Texture& texture, int first_array_index, int array_size, int level) = 0;
		virtual RenderViewPtr Make2DRenderView(Texture& texture, int array_index, Texture::CubeFaces face, int level) = 0;
		virtual RenderViewPtr Make2DRenderView(Texture& texture, int array_index, uint32_t slice, int level) = 0;
		virtual RenderViewPtr MakeCubeRenderView(Texture& texture, int array_index, int level) = 0;
		virtual RenderViewPtr Make3DRenderView(Texture& texture, int array_index, uint32_t first_slice, uint32_t num_slices, int level) = 0;
		virtual RenderViewPtr MakeGraphicsBufferRenderView(GraphicsBuffer& gbuffer, uint32_t width, uint32_t height, ElementFormat pf) = 0;
		virtual RenderViewPtr Make2DDepthStencilRenderView(uint32_t width, uint32_t height, ElementFormat pf,
			uint32_t sample_count, uint32_t sample_quality) = 0;
		virtual RenderViewPtr Make1DDepthStencilRenderView(Texture& texture, int first_array_index, int array_size, int level) = 0;
		virtual RenderViewPtr Make2DDepthStencilRenderView(Texture& texture, int first_array_index, int array_size, int level) = 0;
		virtual RenderViewPtr Make2DDepthStencilRenderView(Texture& texture, int array_index, Texture::CubeFaces face, int level) = 0;
		virtual RenderViewPtr Make2DDepthStencilRenderView(Texture& texture, int array_index, uint32_t slice, int level) = 0;
		virtual RenderViewPtr MakeCubeDepthStencilRenderView(Texture& texture, int array_index, int level) = 0;
		virtual RenderViewPtr Make3DDepthStencilRenderView(Texture& texture, int array_index, uint32_t first_slice, uint32_t num_slices, int level) = 0;

		virtual UnorderedAccessViewPtr Make1DUnorderedAccessView(TexturePtr const & texture, int first_array_index, int array_size,
			int level, ElementFormat pf = EF_Unknown) = 0;
		virtual UnorderedAccessViewPtr Make2DUnorderedAccessView(TexturePtr const & texture, int first_array_index, int array_size,
			int level, ElementFormat pf = EF_Unknown) = 0;
		virtual UnorderedAccessViewPtr Make2DUnorderedAccessView(TexturePtr const & texture, int array_index, Texture::CubeFaces face,
			int level, ElementFormat pf = EF_Unknown) = 0;
		virtual UnorderedAccessViewPtr Make2DUnorderedAccessView(TexturePtr const & texture, int array_index, uint32_t slice,
			int level, ElementFormat pf = EF_Unknown) = 0;
		virtual UnorderedAccessViewPtr MakeCubeUnorderedAccessView(TexturePtr const & texture, int array_index, int level,
			ElementFormat pf = EF_Unknown) = 0;
		virtual UnorderedAccessViewPtr Make3DUnorderedAccessView(TexturePtr const & texture, int array_index, uint32_t first_slice,
			uint32_t num_slices, int level, ElementFormat pf = EF_Unknown) = 0;
		virtual UnorderedAccessViewPtr MakeGraphicsBufferUnorderedAccessView(GraphicsBufferPtr const & gbuffer, ElementFormat pf,
			uint32_t first_elem, uint32_t num_elems) = 0;

		RenderStateObjectPtr MakeRenderStateObject(RasterizerStateDesc const & rs_desc, DepthStencilStateDesc const & dss_desc,
			BlendStateDesc const & bs_desc);
		SamplerStateObjectPtr MakeSamplerStateObject(SamplerStateDesc const & desc);
		virtual ShaderObjectPtr MakeShaderObject() = 0;

	private:
		virtual std::unique_ptr<RenderEngine> DoMakeRenderEngine() = 0;

		virtual RenderStateObjectPtr DoMakeRenderStateObject(RasterizerStateDesc const & rs_desc, DepthStencilStateDesc const & dss_desc,
			BlendStateDesc const & bs_desc) = 0;
		virtual SamplerStateObjectPtr DoMakeSamplerStateObject(SamplerStateDesc const & desc) = 0;

		virtual void DoSuspend() = 0;
		virtual void DoResume() = 0;

	protected:
		std::unique_ptr<RenderEngine> re_;

		std::unordered_map<size_t, RenderStateObjectPtr> rs_pool_;
		std::unordered_map<size_t, SamplerStateObjectPtr> ss_pool_;
	};
}

#endif			// _RENDERFACTORY_HPP
