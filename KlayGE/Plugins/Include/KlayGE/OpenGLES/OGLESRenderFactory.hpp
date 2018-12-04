// OGLESRenderFactoryInternal.h
// KlayGE OpenGL ES��Ⱦ������ ͷ�ļ�
// Ver 3.10.0
// ��Ȩ����(C) ������, 2010
// Homepage: http://www.klayge.org
//
// 3.10.0
// ���ν��� (2010.1.22)
//
// �޸ļ�¼
//////////////////////////////////////////////////////////////////////////////////

#ifndef KLAYGE_PLUGINS_OGLES_RENDER_FACTORY_HPP
#define KLAYGE_PLUGINS_OGLES_RENDER_FACTORY_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/RenderFactory.hpp>

namespace KlayGE
{
	class OGLESRenderFactory : public RenderFactory
	{
	public:
		OGLESRenderFactory();

		std::wstring const & Name() const;

		virtual TexturePtr MakeDelayCreationTexture1D(uint32_t width, uint32_t numMipMaps, uint32_t array_size,
				ElementFormat format, uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint) override;
		virtual TexturePtr MakeDelayCreationTexture2D(uint32_t width, uint32_t height, uint32_t numMipMaps, uint32_t array_size,
				ElementFormat format, uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint) override;
		virtual TexturePtr MakeDelayCreationTexture3D(uint32_t width, uint32_t height, uint32_t depth, uint32_t array_size,
				uint32_t numMipMaps, ElementFormat format, uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint) override;
		virtual TexturePtr MakeDelayCreationTextureCube(uint32_t size, uint32_t numMipMaps, uint32_t array_size,
				ElementFormat format, uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint) override;

		FrameBufferPtr MakeFrameBuffer();

		RenderLayoutPtr MakeRenderLayout();
		virtual GraphicsBufferPtr MakeDelayCreationVertexBuffer(BufferUsage usage, uint32_t access_hint,
			uint32_t size_in_byte, ElementFormat fmt = EF_Unknown) override;
		virtual GraphicsBufferPtr MakeDelayCreationIndexBuffer(BufferUsage usage, uint32_t access_hint,
			uint32_t size_in_byte, ElementFormat fmt = EF_Unknown) override;
		virtual GraphicsBufferPtr MakeDelayCreationConstantBuffer(BufferUsage usage, uint32_t access_hint,
			uint32_t size_in_byte, ElementFormat fmt = EF_Unknown) override;

		QueryPtr MakeOcclusionQuery();
		QueryPtr MakeConditionalRender();
		QueryPtr MakeTimerQuery();
		QueryPtr MakeSOStatisticsQuery() override;

		virtual FencePtr MakeFence() override;

		RenderViewPtr Make1DRenderView(TexturePtr const & texture, int first_array_index, int array_size, int level,
			ElementFormat pf) override;
		RenderViewPtr Make2DRenderView(TexturePtr const & texture, int first_array_index, int array_size, int level,
			ElementFormat pf) override;
		RenderViewPtr Make2DRenderView(TexturePtr const & texture, int array_index, Texture::CubeFaces face, int level,
			ElementFormat pf) override;
		RenderViewPtr Make2DRenderView(TexturePtr const & texture, int array_index, uint32_t slice, int level,
			ElementFormat pf) override;
		RenderViewPtr MakeCubeRenderView(TexturePtr const & texture, int array_index, int level,
			ElementFormat pf) override;
		RenderViewPtr Make3DRenderView(TexturePtr const & texture, int array_index, uint32_t first_slice, uint32_t num_slices, int level,
			ElementFormat pf) override;
		RenderViewPtr MakeGraphicsBufferRenderView(GraphicsBufferPtr const & gbuffer, ElementFormat pf, uint32_t first_elem,
			uint32_t num_elems) override;

		RenderViewPtr Make2DDepthStencilRenderView(uint32_t width, uint32_t height, ElementFormat pf,
			uint32_t sample_count, uint32_t sample_quality) override;
		RenderViewPtr Make1DDepthStencilRenderView(TexturePtr const & texture, int first_array_index, int array_size, int level,
			ElementFormat pf) override;
		RenderViewPtr Make2DDepthStencilRenderView(TexturePtr const & texture, int first_array_index, int array_size, int level,
			ElementFormat pf) override;
		RenderViewPtr Make2DDepthStencilRenderView(TexturePtr const & texture, int array_index, Texture::CubeFaces face, int level,
			ElementFormat pf) override;
		RenderViewPtr Make2DDepthStencilRenderView(TexturePtr const & texture, int array_index, uint32_t slice, int level,
			ElementFormat pf) override;
		RenderViewPtr MakeCubeDepthStencilRenderView(TexturePtr const & texture, int array_index, int level,
			ElementFormat pf) override;
		RenderViewPtr Make3DDepthStencilRenderView(TexturePtr const & texture, int array_index, uint32_t first_slice, uint32_t num_slices,
			int level, ElementFormat pf) override;

		UnorderedAccessViewPtr Make1DUnorderedAccessView(TexturePtr const & texture, int first_array_index, int array_size, int level,
			ElementFormat pf) override;
		UnorderedAccessViewPtr Make2DUnorderedAccessView(TexturePtr const & texture, int first_array_index, int array_size, int level,
			ElementFormat pf) override;
		UnorderedAccessViewPtr Make2DUnorderedAccessView(TexturePtr const & texture, int array_index, Texture::CubeFaces face, int level,
			ElementFormat pf) override;
		UnorderedAccessViewPtr Make2DUnorderedAccessView(TexturePtr const & texture, int array_index, uint32_t slice, int level,
			ElementFormat pf) override;
		UnorderedAccessViewPtr MakeCubeUnorderedAccessView(TexturePtr const & texture, int array_index, int level,
			ElementFormat pf) override;
		UnorderedAccessViewPtr Make3DUnorderedAccessView(TexturePtr const & texture, int array_index, uint32_t first_slice,
			uint32_t num_slices, int level, ElementFormat pf) override;
		UnorderedAccessViewPtr MakeGraphicsBufferUnorderedAccessView(GraphicsBufferPtr const & gbuffer, ElementFormat pf,
			uint32_t first_elem, uint32_t num_elems) override;

		ShaderObjectPtr MakeShaderObject();

	private:
		virtual std::unique_ptr<RenderEngine> DoMakeRenderEngine() override;

		RenderStateObjectPtr DoMakeRenderStateObject(RasterizerStateDesc const & rs_desc, DepthStencilStateDesc const & dss_desc,
			BlendStateDesc const & bs_desc) override;
		SamplerStateObjectPtr DoMakeSamplerStateObject(SamplerStateDesc const & desc);

		virtual void DoSuspend() override;
		virtual void DoResume() override;

	private:
		OGLESRenderFactory(OGLESRenderFactory const &);
		OGLESRenderFactory& operator=(OGLESRenderFactory const &);
	};
}

#endif			// KLAYGE_PLUGINS_OGLES_RENDER_FACTORY_HPP
