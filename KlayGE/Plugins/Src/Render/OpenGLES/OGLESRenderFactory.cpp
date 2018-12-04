// OGLESRenderFactory.cpp
// KlayGE OpenGL ES 2��Ⱦ������ ʵ���ļ�
// Ver 3.10.0
// ��Ȩ����(C) ������, 2010
// Homepage: http://www.klayge.org
//
// 3.10.0
// ���ν��� (2010.1.22)
//
// �޸ļ�¼
//////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KFL/Util.hpp>
#include <KFL/Math.hpp>

#include <KlayGE/OpenGLES/OGLESRenderEngine.hpp>
#include <KlayGE/OpenGLES/OGLESTexture.hpp>
#include <KlayGE/OpenGLES/OGLESFrameBuffer.hpp>
#include <KlayGE/OpenGLES/OGLESRenderLayout.hpp>
#include <KlayGE/OpenGLES/OGLESGraphicsBuffer.hpp>
#include <KlayGE/OpenGLES/OGLESQuery.hpp>
#include <KlayGE/OpenGLES/OGLESRenderView.hpp>
#include <KlayGE/OpenGLES/OGLESRenderStateObject.hpp>
#include <KlayGE/OpenGLES/OGLESShaderObject.hpp>
#include <KlayGE/OpenGLES/OGLESFence.hpp>

#include <KlayGE/OpenGLES/OGLESRenderFactory.hpp>

namespace KlayGE
{
	OGLESRenderFactory::OGLESRenderFactory()
	{
	}

	std::wstring const & OGLESRenderFactory::Name() const
	{
		static std::wstring const name(L"OpenGL ES Render Factory");
		return name;
	}

	TexturePtr OGLESRenderFactory::MakeDelayCreationTexture1D(uint32_t width, uint32_t numMipMaps, uint32_t array_size,
				ElementFormat format, uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint)
	{
		return MakeSharedPtr<OGLESTexture1D>(width, numMipMaps, array_size, format, sample_count, sample_quality, access_hint);
	}

	TexturePtr OGLESRenderFactory::MakeDelayCreationTexture2D(uint32_t width, uint32_t height, uint32_t numMipMaps, uint32_t array_size,
				ElementFormat format, uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint)
	{
		return MakeSharedPtr<OGLESTexture2D>(width, height, numMipMaps, array_size, format, sample_count, sample_quality, access_hint);
	}

	TexturePtr OGLESRenderFactory::MakeDelayCreationTexture3D(uint32_t width, uint32_t height, uint32_t depth, uint32_t numMipMaps, uint32_t array_size,
				ElementFormat format, uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint)
	{
		return MakeSharedPtr<OGLESTexture3D>(width, height, depth, numMipMaps, array_size, format, sample_count, sample_quality, access_hint);
	}

	TexturePtr OGLESRenderFactory::MakeDelayCreationTextureCube(uint32_t size, uint32_t numMipMaps, uint32_t array_size,
				ElementFormat format, uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint)
	{
		return MakeSharedPtr<OGLESTextureCube>(size, numMipMaps, array_size, format, sample_count, sample_quality, access_hint);
	}

	FrameBufferPtr OGLESRenderFactory::MakeFrameBuffer()
	{
		return MakeSharedPtr<OGLESFrameBuffer>(true);
	}

	RenderLayoutPtr OGLESRenderFactory::MakeRenderLayout()
	{
		return MakeSharedPtr<OGLESRenderLayout>();
	}

	GraphicsBufferPtr OGLESRenderFactory::MakeDelayCreationVertexBuffer(BufferUsage usage, uint32_t access_hint,
			uint32_t size_in_byte, ElementFormat fmt)
	{
		return MakeSharedPtr<OGLESGraphicsBuffer>(usage, access_hint, GL_ARRAY_BUFFER, size_in_byte, fmt);
	}

	GraphicsBufferPtr OGLESRenderFactory::MakeDelayCreationIndexBuffer(BufferUsage usage, uint32_t access_hint,
			uint32_t size_in_byte, ElementFormat fmt)
	{
		return MakeSharedPtr<OGLESGraphicsBuffer>(usage, access_hint, GL_ELEMENT_ARRAY_BUFFER, size_in_byte, fmt);
	}

	GraphicsBufferPtr OGLESRenderFactory::MakeDelayCreationConstantBuffer(BufferUsage usage, uint32_t access_hint,
			uint32_t size_in_byte, ElementFormat fmt)
	{
		return MakeSharedPtr<OGLESGraphicsBuffer>(usage, access_hint, GL_UNIFORM_BUFFER, size_in_byte, fmt);
	}

	QueryPtr OGLESRenderFactory::MakeOcclusionQuery()
	{
		return QueryPtr();
	}

	QueryPtr OGLESRenderFactory::MakeConditionalRender()
	{
		return MakeSharedPtr<OGLESConditionalRender>();
	}

	QueryPtr OGLESRenderFactory::MakeTimerQuery()
	{
		if (glloader_GLES_EXT_disjoint_timer_query())
		{
			return MakeSharedPtr<OGLESTimerQuery>();
		}
		else
		{
			return QueryPtr();
		}
	}

	QueryPtr OGLESRenderFactory::MakeSOStatisticsQuery()
	{
		return MakeSharedPtr<OGLESSOStatisticsQuery>();
	}

	FencePtr OGLESRenderFactory::MakeFence()
	{
		return MakeSharedPtr<OGLESFence>();
	}

	RenderViewPtr OGLESRenderFactory::Make1DRenderView(TexturePtr const & texture, int first_array_index, int array_size, int level,
		ElementFormat pf)
	{
		return MakeSharedPtr<OGLESTexture1DRenderView>(texture, pf, first_array_index, array_size, level);
	}

	RenderViewPtr OGLESRenderFactory::Make2DRenderView(TexturePtr const & texture, int first_array_index, int array_size, int level,
		ElementFormat pf)
	{
		return MakeSharedPtr<OGLESTexture2DRenderView>(texture, pf, first_array_index, array_size, level);
	}

	RenderViewPtr OGLESRenderFactory::Make2DRenderView(TexturePtr const & texture, int array_index, Texture::CubeFaces face, int level,
		ElementFormat pf)
	{
		return MakeSharedPtr<OGLESTextureCubeRenderView>(texture, pf, array_index, face, level);
	}

	RenderViewPtr OGLESRenderFactory::Make2DRenderView(TexturePtr const & texture, int array_index, uint32_t slice, int level,
		ElementFormat pf)
	{
		return MakeSharedPtr<OGLESTexture3DRenderView>(texture, pf, array_index, slice, level);
	}
	
	RenderViewPtr OGLESRenderFactory::MakeCubeRenderView(TexturePtr const & texture, int array_index, int level, ElementFormat pf)
	{
		return MakeSharedPtr<OGLESTextureCubeRenderView>(texture, pf, array_index, level);
	}

	RenderViewPtr OGLESRenderFactory::Make3DRenderView(TexturePtr const & /*texture*/, int /*array_index*/, uint32_t /*first_slice*/,
		uint32_t /*num_slices*/, int /*level*/, ElementFormat /*pf*/)
	{
		return RenderViewPtr();
	}

	RenderViewPtr OGLESRenderFactory::MakeGraphicsBufferRenderView(GraphicsBufferPtr const & /*gbuffer*/, ElementFormat /*pf*/,
		uint32_t /*first_elem*/, uint32_t /*num_elems*/)
	{
		return RenderViewPtr();
	}

	RenderViewPtr OGLESRenderFactory::Make2DDepthStencilRenderView(uint32_t width, uint32_t height, ElementFormat pf, uint32_t sample_count,
		uint32_t sample_quality)
	{
		return MakeSharedPtr<OGLESDepthStencilRenderView>(width, height, pf, sample_count, sample_quality);
	}

	RenderViewPtr OGLESRenderFactory::Make1DDepthStencilRenderView(TexturePtr const & texture, int first_array_index, int array_size,
		int level, ElementFormat pf)
	{
		return this->Make2DDepthStencilRenderView(texture, first_array_index, array_size, level, pf);
	}

	RenderViewPtr OGLESRenderFactory::Make2DDepthStencilRenderView(TexturePtr const & texture, int first_array_index, int array_size,
		int level, ElementFormat pf)
	{
		return MakeSharedPtr<OGLESDepthStencilRenderView>(texture, pf, first_array_index, array_size, level);
	}

	RenderViewPtr OGLESRenderFactory::Make2DDepthStencilRenderView(TexturePtr const & texture, int array_index, Texture::CubeFaces face,
		int level, ElementFormat pf)
	{
		return MakeSharedPtr<OGLESTextureCubeDepthStencilRenderView>(texture, pf, array_index, face, level);
	}

	RenderViewPtr OGLESRenderFactory::Make2DDepthStencilRenderView(TexturePtr const & /*texture*/, int /*array_index*/, uint32_t /*slice*/,
		int /*level*/, ElementFormat /*pf*/)
	{
		return RenderViewPtr();
	}

	RenderViewPtr OGLESRenderFactory::MakeCubeDepthStencilRenderView(TexturePtr const & texture, int array_index, int level,
		ElementFormat pf)
	{
		return MakeSharedPtr<OGLESDepthStencilRenderView>(texture, pf, array_index, 1, level);
	}

	RenderViewPtr OGLESRenderFactory::Make3DDepthStencilRenderView(TexturePtr const & /*texture*/, int /*array_index*/,
		uint32_t /*first_slice*/, uint32_t /*num_slices*/, int /*level*/, ElementFormat /*pf*/)
	{
		return RenderViewPtr();
	}

	UnorderedAccessViewPtr OGLESRenderFactory::Make1DUnorderedAccessView(TexturePtr const & /*texture*/, int /*first_array_index*/,
		int /*array_size*/, int /*level*/, ElementFormat /*pf*/)
	{
		return UnorderedAccessViewPtr();
	}

	UnorderedAccessViewPtr OGLESRenderFactory::Make2DUnorderedAccessView(TexturePtr const & /*texture*/, int /*first_array_index*/,
		int /*array_size*/, int /*level*/, ElementFormat /*pf*/)
	{
		return UnorderedAccessViewPtr();
	}

	UnorderedAccessViewPtr OGLESRenderFactory::Make2DUnorderedAccessView(TexturePtr const & /*texture*/, int /*array_index*/,
		Texture::CubeFaces /*face*/, int /*level*/, ElementFormat /*pf*/)
	{
		return UnorderedAccessViewPtr();
	}

	UnorderedAccessViewPtr OGLESRenderFactory::Make2DUnorderedAccessView(TexturePtr const & /*texture*/, int /*array_index*/,
		uint32_t /*slice*/, int /*level*/, ElementFormat /*pf*/)
	{
		return UnorderedAccessViewPtr();
	}

	UnorderedAccessViewPtr OGLESRenderFactory::MakeCubeUnorderedAccessView(TexturePtr const & /*texture*/, int /*array_index*/,
		int /*level*/, ElementFormat /*pf*/)
	{
		return UnorderedAccessViewPtr();
	}

	UnorderedAccessViewPtr OGLESRenderFactory::Make3DUnorderedAccessView(TexturePtr const & /*texture*/, int /*array_index*/,
		uint32_t /*first_slice*/, uint32_t /*num_slices*/, int /*level*/, ElementFormat /*pf*/)
	{
		return UnorderedAccessViewPtr();
	}

	UnorderedAccessViewPtr OGLESRenderFactory::MakeGraphicsBufferUnorderedAccessView(GraphicsBufferPtr const & /*gbuffer*/,
		ElementFormat /*pf*/, uint32_t /*first_elem*/, uint32_t /*num_elems*/)
	{
		return UnorderedAccessViewPtr();
	}

	ShaderObjectPtr OGLESRenderFactory::MakeShaderObject()
	{
		return MakeSharedPtr<OGLESShaderObject>();
	}

	std::unique_ptr<RenderEngine> OGLESRenderFactory::DoMakeRenderEngine()
	{
		return MakeUniquePtr<OGLESRenderEngine>();
	}

	RenderStateObjectPtr OGLESRenderFactory::DoMakeRenderStateObject(RasterizerStateDesc const & rs_desc, DepthStencilStateDesc const & dss_desc,
		BlendStateDesc const & bs_desc)
	{
		return MakeSharedPtr<OGLESRenderStateObject>(rs_desc, dss_desc, bs_desc);
	}

	SamplerStateObjectPtr OGLESRenderFactory::DoMakeSamplerStateObject(SamplerStateDesc const & desc)
	{
		return MakeSharedPtr<OGLESSamplerStateObject>(desc);
	}

	void OGLESRenderFactory::DoSuspend()
	{
		// TODO
	}

	void OGLESRenderFactory::DoResume()
	{
		// TODO
	}
}

extern "C"
{
	KLAYGE_SYMBOL_EXPORT void MakeRenderFactory(std::unique_ptr<KlayGE::RenderFactory>& ptr)
	{
		ptr = KlayGE::MakeUniquePtr<KlayGE::OGLESRenderFactory>();
	}
}
