// D3D11RenderFactory.cpp
// KlayGE D3D11渲染引擎抽象工厂 实现文件
// Ver 3.8.0
// 版权所有(C) 龚敏敏, 2009
// Homepage: http://www.klayge.org
//
// 3.8.0
// 初次建立 (2009.1.30)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KFL/Util.hpp>
#include <KFL/Math.hpp>

#include <KlayGE/D3D11/D3D11RenderEngine.hpp>
#include <KlayGE/D3D11/D3D11Texture.hpp>
#include <KlayGE/D3D11/D3D11FrameBuffer.hpp>
#include <KlayGE/D3D11/D3D11RenderLayout.hpp>
#include <KlayGE/D3D11/D3D11GraphicsBuffer.hpp>
#include <KlayGE/D3D11/D3D11Query.hpp>
#include <KlayGE/D3D11/D3D11RenderView.hpp>
#include <KlayGE/D3D11/D3D11RenderStateObject.hpp>
#include <KlayGE/D3D11/D3D11ShaderObject.hpp>
#include <KlayGE/D3D11/D3D11Fence.hpp>

#include <KlayGE/D3D11/D3D11RenderFactory.hpp>

namespace KlayGE
{
	D3D11RenderFactory::D3D11RenderFactory()
	{
	}

	std::wstring const & D3D11RenderFactory::Name() const
	{
		static std::wstring const name(L"Direct3D11 Render Factory");
		return name;
	}

	TexturePtr D3D11RenderFactory::MakeDelayCreationTexture1D(uint32_t width, uint32_t num_mip_maps, uint32_t array_size,
			ElementFormat format, uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint)
	{
		return MakeSharedPtr<D3D11Texture1D>(width, num_mip_maps, array_size, format, sample_count, sample_quality, access_hint);
	}
	TexturePtr D3D11RenderFactory::MakeDelayCreationTexture2D(uint32_t width, uint32_t height, uint32_t num_mip_maps, uint32_t array_size,
			ElementFormat format, uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint)
	{
		return MakeSharedPtr<D3D11Texture2D>(width, height, num_mip_maps, array_size, format, sample_count, sample_quality, access_hint);
	}
	TexturePtr D3D11RenderFactory::MakeDelayCreationTexture3D(uint32_t width, uint32_t height, uint32_t depth, uint32_t num_mip_maps, uint32_t array_size,
			ElementFormat format, uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint)
	{
		return MakeSharedPtr<D3D11Texture3D>(width, height, depth, num_mip_maps, array_size, format, sample_count, sample_quality, access_hint);
	}
	TexturePtr D3D11RenderFactory::MakeDelayCreationTextureCube(uint32_t size, uint32_t num_mip_maps, uint32_t array_size,
			ElementFormat format, uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint)
	{
		return MakeSharedPtr<D3D11TextureCube>(size, num_mip_maps, array_size, format, sample_count, sample_quality, access_hint);
	}

	FrameBufferPtr D3D11RenderFactory::MakeFrameBuffer()
	{
		return MakeSharedPtr<D3D11FrameBuffer>();
	}

	RenderLayoutPtr D3D11RenderFactory::MakeRenderLayout()
	{
		return MakeSharedPtr<D3D11RenderLayout>();
	}

	GraphicsBufferPtr D3D11RenderFactory::MakeDelayCreationVertexBuffer(BufferUsage usage, uint32_t access_hint,
			uint32_t size_in_byte, ElementFormat fmt)
	{
		return MakeSharedPtr<D3D11GraphicsBuffer>(usage, access_hint, D3D11_BIND_VERTEX_BUFFER, size_in_byte, fmt);
	}

	GraphicsBufferPtr D3D11RenderFactory::MakeDelayCreationIndexBuffer(BufferUsage usage, uint32_t access_hint,
			uint32_t size_in_byte, ElementFormat fmt)
	{
		return MakeSharedPtr<D3D11GraphicsBuffer>(usage, access_hint, D3D11_BIND_INDEX_BUFFER, size_in_byte, fmt);
	}

	GraphicsBufferPtr D3D11RenderFactory::MakeDelayCreationConstantBuffer(BufferUsage usage, uint32_t access_hint,
			uint32_t size_in_byte, ElementFormat fmt)
	{
		return MakeSharedPtr<D3D11GraphicsBuffer>(usage, access_hint, D3D11_BIND_CONSTANT_BUFFER, size_in_byte, fmt);
	}

	QueryPtr D3D11RenderFactory::MakeOcclusionQuery()
	{
		return MakeSharedPtr<D3D11OcclusionQuery>();
	}

	QueryPtr D3D11RenderFactory::MakeConditionalRender()
	{
		return MakeSharedPtr<D3D11ConditionalRender>();
	}

	QueryPtr D3D11RenderFactory::MakeTimerQuery()
	{
		return MakeSharedPtr<D3D11TimerQuery>();
	}

	QueryPtr D3D11RenderFactory::MakeSOStatisticsQuery()
	{
		return MakeSharedPtr<D3D11SOStatisticsQuery>();
	}

	FencePtr D3D11RenderFactory::MakeFence()
	{
		FencePtr ret;

		auto* d3d_device_5 = checked_cast<D3D11RenderEngine*>(re_.get())->D3DDevice5();
		if (d3d_device_5 != nullptr)
		{
			ret = MakeSharedPtr<D3D11_4Fence>();
		}
		else
		{
			ret = MakeSharedPtr<D3D11Fence>();
		}

		return ret;
	}

	RenderViewPtr D3D11RenderFactory::Make1DRenderView(TexturePtr const & texture, int first_array_index, int array_size, int level,
		ElementFormat pf)
	{
		return MakeSharedPtr<D3D11RenderTargetRenderView>(texture, pf, first_array_index, array_size, level);
	}

	RenderViewPtr D3D11RenderFactory::Make2DRenderView(TexturePtr const & texture, int first_array_index, int array_size, int level,
		ElementFormat pf)
	{
		return MakeSharedPtr<D3D11RenderTargetRenderView>(texture, pf, first_array_index, array_size, level);
	}

	RenderViewPtr D3D11RenderFactory::Make2DRenderView(TexturePtr const & texture, int array_index, Texture::CubeFaces face, int level,
		ElementFormat pf)
	{
		return MakeSharedPtr<D3D11RenderTargetRenderView>(texture, pf, array_index, face, level);
	}

	RenderViewPtr D3D11RenderFactory::Make2DRenderView(TexturePtr const & texture, int array_index, uint32_t slice, int level,
		ElementFormat pf)
	{
		return this->Make3DRenderView(texture, array_index, slice, 1, level, pf);
	}

	RenderViewPtr D3D11RenderFactory::MakeCubeRenderView(TexturePtr const & texture, int array_index, int level, ElementFormat pf)
	{
		int array_size = 1;
		return MakeSharedPtr<D3D11RenderTargetRenderView>(texture, pf, array_index, array_size, level);
	}

	RenderViewPtr D3D11RenderFactory::Make3DRenderView(TexturePtr const & texture, int array_index, uint32_t first_slice,
		uint32_t num_slices, int level, ElementFormat pf)
	{
		return MakeSharedPtr<D3D11RenderTargetRenderView>(texture, pf, array_index, first_slice, num_slices, level);
	}

	RenderViewPtr D3D11RenderFactory::MakeGraphicsBufferRenderView(GraphicsBufferPtr const & gbuffer, ElementFormat pf,
		uint32_t first_elem, uint32_t num_elems)
	{
		return MakeSharedPtr<D3D11RenderTargetRenderView>(gbuffer, pf, first_elem, num_elems);
	}

	RenderViewPtr D3D11RenderFactory::Make2DDepthStencilRenderView(uint32_t width, uint32_t height,
		ElementFormat pf, uint32_t sample_count, uint32_t sample_quality)
	{
		return MakeSharedPtr<D3D11DepthStencilRenderView>(width, height, pf, sample_count, sample_quality);
	}

	RenderViewPtr D3D11RenderFactory::Make1DDepthStencilRenderView(TexturePtr const & texture, int first_array_index, int array_size,
		int level, ElementFormat pf)
	{
		return MakeSharedPtr<D3D11DepthStencilRenderView>(texture, pf, first_array_index, array_size, level);
	}

	RenderViewPtr D3D11RenderFactory::Make2DDepthStencilRenderView(TexturePtr const & texture, int first_array_index, int array_size,
		int level, ElementFormat pf)
	{
		return MakeSharedPtr<D3D11DepthStencilRenderView>(texture, pf, first_array_index, array_size, level);
	}

	RenderViewPtr D3D11RenderFactory::Make2DDepthStencilRenderView(TexturePtr const & texture, int array_index, Texture::CubeFaces face,
		int level, ElementFormat pf)
	{
		return MakeSharedPtr<D3D11DepthStencilRenderView>(texture, pf, array_index, face, level);
	}
	
	RenderViewPtr D3D11RenderFactory::Make2DDepthStencilRenderView(TexturePtr const & texture, int array_index, uint32_t slice, int level,
		ElementFormat pf)
	{
		return this->Make3DDepthStencilRenderView(texture, array_index, slice, 1, level, pf);
	}

	RenderViewPtr D3D11RenderFactory::MakeCubeDepthStencilRenderView(TexturePtr const & texture, int array_index, int level,
		ElementFormat pf)
	{
		int array_size = 1;
		return MakeSharedPtr<D3D11DepthStencilRenderView>(texture, pf, array_index, array_size, level);
	}
	
	RenderViewPtr D3D11RenderFactory::Make3DDepthStencilRenderView(TexturePtr const & texture, int array_index, uint32_t first_slice,
		uint32_t num_slices, int level, ElementFormat pf)
	{
		return MakeSharedPtr<D3D11DepthStencilRenderView>(texture, pf, array_index, first_slice, num_slices, level);
	}

	UnorderedAccessViewPtr D3D11RenderFactory::Make1DUnorderedAccessView(TexturePtr const & texture, int first_array_index, int array_size,
		int level, ElementFormat pf)
	{
		return MakeSharedPtr<D3D11UnorderedAccessView>(texture, pf, first_array_index, array_size, level);
	}

	UnorderedAccessViewPtr D3D11RenderFactory::Make2DUnorderedAccessView(TexturePtr const & texture, int first_array_index, int array_size,
		int level, ElementFormat pf)
	{
		return MakeSharedPtr<D3D11UnorderedAccessView>(texture, pf, first_array_index, array_size, level);
	}

	UnorderedAccessViewPtr D3D11RenderFactory::Make2DUnorderedAccessView(TexturePtr const & texture, int array_index,
		Texture::CubeFaces face, int level, ElementFormat pf)
	{
		return MakeSharedPtr<D3D11UnorderedAccessView>(texture, pf, array_index, face, level);
	}

	UnorderedAccessViewPtr D3D11RenderFactory::Make2DUnorderedAccessView(TexturePtr const & texture, int array_index, uint32_t slice,
		int level, ElementFormat pf)
	{
		return MakeSharedPtr<D3D11UnorderedAccessView>(texture, pf, array_index, slice, level);
	}

	UnorderedAccessViewPtr D3D11RenderFactory::MakeCubeUnorderedAccessView(TexturePtr const & texture, int array_index, int level,
		ElementFormat pf)
	{
		int array_size = 1;
		return MakeSharedPtr<D3D11UnorderedAccessView>(texture, pf, array_index, array_size, level);
	}

	UnorderedAccessViewPtr D3D11RenderFactory::Make3DUnorderedAccessView(TexturePtr const & texture, int array_index, uint32_t first_slice,
		uint32_t num_slices, int level, ElementFormat pf)
	{
		return MakeSharedPtr<D3D11UnorderedAccessView>(texture, pf, array_index, first_slice, num_slices, level);
	}

	UnorderedAccessViewPtr D3D11RenderFactory::MakeGraphicsBufferUnorderedAccessView(GraphicsBufferPtr const & gbuffer, ElementFormat pf,
		uint32_t first_elem, uint32_t num_elems)
	{
		return MakeSharedPtr<D3D11UnorderedAccessView>(gbuffer, pf, first_elem, num_elems);
	}

	ShaderObjectPtr D3D11RenderFactory::MakeShaderObject()
	{
		return MakeSharedPtr<D3D11ShaderObject>();
	}

	std::unique_ptr<RenderEngine> D3D11RenderFactory::DoMakeRenderEngine()
	{
		return MakeUniquePtr<D3D11RenderEngine>();
	}

	RenderStateObjectPtr D3D11RenderFactory::DoMakeRenderStateObject(RasterizerStateDesc const & rs_desc,
		DepthStencilStateDesc const & dss_desc, BlendStateDesc const & bs_desc)
	{
		return MakeSharedPtr<D3D11RenderStateObject>(rs_desc, dss_desc, bs_desc);
	}

	SamplerStateObjectPtr D3D11RenderFactory::DoMakeSamplerStateObject(SamplerStateDesc const & desc)
	{
		return MakeSharedPtr<D3D11SamplerStateObject>(desc);
	}

	void D3D11RenderFactory::DoSuspend()
	{
		// TODO
	}

	void D3D11RenderFactory::DoResume()
	{
		// TODO
	}
}

extern "C"
{
	KLAYGE_SYMBOL_EXPORT void MakeRenderFactory(std::unique_ptr<KlayGE::RenderFactory>& ptr)
	{
		ptr = KlayGE::MakeUniquePtr<KlayGE::D3D11RenderFactory>();
	}
}
