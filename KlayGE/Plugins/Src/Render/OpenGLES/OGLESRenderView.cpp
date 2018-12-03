// OGLESRenderView.cpp
// KlayGE OpenGL ES 2渲染视图类 实现文件
// Ver 3.10.0
// 版权所有(C) 龚敏敏, 2010
// Homepage: http://www.klayge.org
//
// 3.10.0
// 初次建立 (2010.1.22)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KFL/Util.hpp>
#include <KFL/ErrorHandling.hpp>
#include <KFL/Math.hpp>
#include <KlayGE/App3D.hpp>
#include <KlayGE/Window.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/RenderFactory.hpp>

#include <system_error>
#include <boost/assert.hpp>

#include <KlayGE/OpenGLES/OGLESMapping.hpp>
#include <KlayGE/OpenGLES/OGLESTexture.hpp>
#include <KlayGE/OpenGLES/OGLESGraphicsBuffer.hpp>
#include <KlayGE/OpenGLES/OGLESFrameBuffer.hpp>
#include <KlayGE/OpenGLES/OGLESRenderEngine.hpp>
#include <KlayGE/OpenGLES/OGLESRenderView.hpp>

namespace KlayGE
{
	OGLESRenderView::OGLESRenderView()
		: gl_tex_(0), gl_fbo_(0)
	{
	}

	OGLESRenderView::~OGLESRenderView()
	{
	}

	void OGLESRenderView::ClearDepth(float depth)
	{
		this->DoClear(GL_DEPTH_BUFFER_BIT, Color(), depth, 0);
	}

	void OGLESRenderView::ClearStencil(int32_t stencil)
	{
		this->DoClear(GL_STENCIL_BUFFER_BIT, Color(), 0, stencil);
	}

	void OGLESRenderView::ClearDepthStencil(float depth, int32_t stencil)
	{
		uint32_t flags = 0;
		if (IsDepthFormat(pf_))
		{
			flags |= GL_DEPTH_BUFFER_BIT;
		}
		if (IsStencilFormat(pf_))
		{
			flags |= GL_STENCIL_BUFFER_BIT;
		}

		this->DoClear(flags, Color(), depth, stencil);
	}

	void OGLESRenderView::DoClear(uint32_t flags, Color const & clr, float depth, int32_t stencil)
	{
		OGLESRenderEngine& re = *checked_cast<OGLESRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());

		GLuint old_fbo = re.BindFramebuffer();
		re.BindFramebuffer(gl_fbo_);

		DepthStencilStateDesc const & ds_desc = re.CurRenderStateObject()->GetDepthStencilStateDesc();
		BlendStateDesc const & blend_desc = re.CurRenderStateObject()->GetBlendStateDesc();

		if (flags & GL_COLOR_BUFFER_BIT)
		{
			if (blend_desc.color_write_mask[0] != CMASK_All)
			{
				glColorMask(true, true, true, true);
			}
		}
		if (flags & GL_DEPTH_BUFFER_BIT)
		{
			if (!ds_desc.depth_write_mask)
			{
				glDepthMask(GL_TRUE);
			}
		}
		if (flags & GL_STENCIL_BUFFER_BIT)
		{
			if (ds_desc.front_stencil_write_mask != 0xFF)
			{
				glStencilMaskSeparate(GL_FRONT, 0xFF);
			}
			if (ds_desc.back_stencil_write_mask != 0xFF)
			{
				glStencilMaskSeparate(GL_BACK, 0xFF);
			}
		}

		if (flags & GL_COLOR_BUFFER_BIT)
		{
			glClearBufferfv(GL_COLOR, index_, &clr[0]);
		}

		if ((flags & GL_DEPTH_BUFFER_BIT) && (flags & GL_STENCIL_BUFFER_BIT))
		{
			glClearBufferfi(GL_DEPTH_STENCIL, 0, depth, stencil);
		}
		else
		{
			if (flags & GL_DEPTH_BUFFER_BIT)
			{
				glClearBufferfv(GL_DEPTH, 0, &depth);
			}
			else
			{
				if (flags & GL_STENCIL_BUFFER_BIT)
				{
					GLint s = stencil;
					glClearBufferiv(GL_STENCIL, 0, &s);
				}
			}
		}

		if (flags & GL_COLOR_BUFFER_BIT)
		{
			if (blend_desc.color_write_mask[0] != CMASK_All)
			{
				glColorMask((blend_desc.color_write_mask[0] & CMASK_Red) != 0,
						(blend_desc.color_write_mask[0] & CMASK_Green) != 0,
						(blend_desc.color_write_mask[0] & CMASK_Blue) != 0,
						(blend_desc.color_write_mask[0] & CMASK_Alpha) != 0);
			}
		}
		if (flags & GL_DEPTH_BUFFER_BIT)
		{
			if (!ds_desc.depth_write_mask)
			{
				glDepthMask(GL_FALSE);
			}
		}
		if (flags & GL_STENCIL_BUFFER_BIT)
		{
			if (ds_desc.front_stencil_write_mask != 0xFF)
			{
				glStencilMaskSeparate(GL_FRONT, ds_desc.front_stencil_write_mask);
			}
			if (ds_desc.back_stencil_write_mask != 0xFF)
			{
				glStencilMaskSeparate(GL_BACK, ds_desc.back_stencil_write_mask);
			}
		}

		re.BindFramebuffer(old_fbo);
	}
	
	void OGLESRenderView::DoDiscardColor()
	{
		GLenum attachment;
		if (gl_fbo_ != 0)
		{
			attachment = GL_COLOR_ATTACHMENT0 + index_;
		}
		else
		{
			attachment = GL_COLOR;
		}

		OGLESRenderEngine& re = *checked_cast<OGLESRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());

		GLuint old_fbo = re.BindFramebuffer();
		re.BindFramebuffer(gl_fbo_);

		glInvalidateFramebuffer(GL_FRAMEBUFFER, 1, &attachment);

		re.BindFramebuffer(old_fbo);
	}

	void OGLESRenderView::DoDiscardDepthStencil()
	{
		GLenum attachments[2];
		if (gl_fbo_ != 0)
		{
			attachments[0] = GL_DEPTH_ATTACHMENT;
			attachments[1] = GL_STENCIL_ATTACHMENT;
		}
		else
		{
			attachments[0] = GL_DEPTH;
			attachments[1] = GL_STENCIL;
		}

		OGLESRenderEngine& re = *checked_cast<OGLESRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());

		GLuint old_fbo = re.BindFramebuffer();
		re.BindFramebuffer(gl_fbo_);

		glInvalidateFramebuffer(GL_FRAMEBUFFER, 2, attachments);

		re.BindFramebuffer(old_fbo);
	}


	OGLESScreenColorRenderView::OGLESScreenColorRenderView(uint32_t width, uint32_t height, ElementFormat pf)
	{
		width_ = width;
		height_ = height;
		pf_ = pf;
		sample_count_ = 1;
		sample_quality_ = 0;
	}

	void OGLESScreenColorRenderView::ClearColor(Color const & clr)
	{
		this->DoClear(GL_COLOR_BUFFER_BIT, clr, 0, 0);
	}

	void OGLESScreenColorRenderView::ClearDepth(float /*depth*/)
	{
		KFL_UNREACHABLE("Can't be called");
	}

	void OGLESScreenColorRenderView::ClearStencil(int32_t /*stencil*/)
	{
		KFL_UNREACHABLE("Can't be called");
	}

	void OGLESScreenColorRenderView::ClearDepthStencil(float /*depth*/, int32_t /*stencil*/)
	{
		KFL_UNREACHABLE("Can't be called");
	}

	void OGLESScreenColorRenderView::Discard()
	{
		this->DoDiscardColor();
	}

	void OGLESScreenColorRenderView::OnAttached(FrameBuffer& fb, uint32_t att)
	{
		KFL_UNUSED(fb);

		BOOST_ASSERT(0 == checked_cast<OGLESFrameBuffer*>(&fb)->OGLFbo());

		index_ = att - FrameBuffer::ATT_Color0;

		OGLESRenderEngine& re = *checked_cast<OGLESRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		re.BindFramebuffer(0);
	}

	void OGLESScreenColorRenderView::OnDetached(FrameBuffer& fb, uint32_t /*att*/)
	{
		KFL_UNUSED(fb);

		BOOST_ASSERT(0 == checked_cast<OGLESFrameBuffer*>(&fb)->OGLFbo());

		OGLESRenderEngine& re = *checked_cast<OGLESRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		re.BindFramebuffer(0);
	}


	OGLESScreenDepthStencilRenderView::OGLESScreenDepthStencilRenderView(uint32_t width, uint32_t height,
									ElementFormat pf)
	{
		BOOST_ASSERT(IsDepthFormat(pf));

		width_ = width;
		height_ = height;
		pf_ = pf;
		sample_count_ = 1;
		sample_quality_ = 0;
	}

	void OGLESScreenDepthStencilRenderView::ClearColor(Color const & /*clr*/)
	{
		KFL_UNREACHABLE("Can't be called");
	}

	void OGLESScreenDepthStencilRenderView::Discard()
	{
		this->DoDiscardDepthStencil();
	}

	void OGLESScreenDepthStencilRenderView::OnAttached(FrameBuffer& fb, uint32_t /*att*/)
	{
		KFL_UNUSED(fb);

		BOOST_ASSERT(0 == checked_cast<OGLESFrameBuffer*>(&fb)->OGLFbo());

		index_ = 0;

		OGLESRenderEngine& re = *checked_cast<OGLESRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		re.BindFramebuffer(0);
	}

	void OGLESScreenDepthStencilRenderView::OnDetached(FrameBuffer& fb, uint32_t /*att*/)
	{
		KFL_UNUSED(fb);

		BOOST_ASSERT(0 == checked_cast<OGLESFrameBuffer*>(&fb)->OGLFbo());

		OGLESRenderEngine& re = *checked_cast<OGLESRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		re.BindFramebuffer(0);
	}


	OGLESTexture1DRenderView::OGLESTexture1DRenderView(TexturePtr const & texture_1d, ElementFormat pf, int array_index, int array_size, int level)
		: array_index_(array_index), array_size_(array_size), level_(level)
	{
		BOOST_ASSERT(Texture::TT_1D == texture_1d->Type());
		BOOST_ASSERT((1 == array_size) || ((0 == array_index) && (static_cast<uint32_t>(array_size) == texture_1d->ArraySize())));

		if (array_index > 0)
		{
			TERRC(std::errc::function_not_supported);
		}

		tex_ = texture_1d;
		gl_tex_ = checked_cast<OGLESTexture*>(tex_.get())->GLTexture();

		width_ = texture_1d->Width(level);
		height_ = 1;
		pf_ = pf == EF_Unknown ? texture_1d->Format() : pf;
		sample_count_ = texture_1d->SampleCount();
		sample_quality_ = texture_1d->SampleQuality();
	}

	void OGLESTexture1DRenderView::ClearColor(Color const & clr)
	{
		if (gl_fbo_ != 0)
		{
			this->DoClear(GL_COLOR_BUFFER_BIT, clr, 0, 0);
		}
		else
		{
			GLenum const gl_target = checked_cast<OGLESTexture*>(tex_.get())->GLType();

			OGLESRenderEngine& re = *checked_cast<OGLESRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			re.BindTexture(0, gl_target, gl_tex_);

			std::vector<Color> mem_clr(width_, clr);
			glTexSubImage2D(gl_target, level_, 0, 0, width_, 1, GL_RGBA, GL_FLOAT, &mem_clr[0]);
		}
	}

	void OGLESTexture1DRenderView::Discard()
	{
		this->DoDiscardColor();
	}

	void OGLESTexture1DRenderView::OnAttached(FrameBuffer& fb, uint32_t att)
	{
		BOOST_ASSERT(att != FrameBuffer::ATT_DepthStencil);

		index_ = att - FrameBuffer::ATT_Color0;
		gl_fbo_ = checked_cast<OGLESFrameBuffer*>(&fb)->OGLFbo();
		GLenum const gl_target = checked_cast<OGLESTexture*>(tex_.get())->GLType();

		OGLESRenderEngine& re = *checked_cast<OGLESRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		re.BindFramebuffer(gl_fbo_);

		if (GL_TEXTURE_2D == gl_target)
		{
			glFramebufferTexture2D(GL_FRAMEBUFFER,
					GL_COLOR_ATTACHMENT0 + att - FrameBuffer::ATT_Color0, gl_target, gl_tex_, level_);
		}
		else
		{
			if (array_size_ > 1)
			{
				glFramebufferTextureEXT(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + att - FrameBuffer::ATT_Color0, gl_tex_, level_);
			}
			else
			{
				glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + att - FrameBuffer::ATT_Color0,
					gl_tex_, level_, array_index_);
			}
		}

		re.BindFramebuffer(0);
	}

	void OGLESTexture1DRenderView::OnDetached(FrameBuffer& fb, uint32_t att)
	{
		KFL_UNUSED(fb);

		BOOST_ASSERT(att != FrameBuffer::ATT_DepthStencil);
		BOOST_ASSERT(gl_fbo_ == checked_cast<OGLESFrameBuffer*>(&fb)->OGLFbo());

		GLenum const gl_target = checked_cast<OGLESTexture*>(tex_.get())->GLType();

		OGLESRenderEngine& re = *checked_cast<OGLESRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		re.BindFramebuffer(gl_fbo_);

		if (GL_TEXTURE_2D == gl_target)
		{
			glFramebufferTexture2D(GL_FRAMEBUFFER,
					GL_COLOR_ATTACHMENT0 + att - FrameBuffer::ATT_Color0, gl_target, 0, 0);
		}
		else
		{
			if (array_size_ > 1)
			{
				glFramebufferTextureEXT(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + att - FrameBuffer::ATT_Color0, 0, 0);
			}
			else
			{
				glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + att - FrameBuffer::ATT_Color0,
					0, 0, 0);
			}
		}

		re.BindFramebuffer(0);
	}


	OGLESTexture2DRenderView::OGLESTexture2DRenderView(TexturePtr const & texture_2d, ElementFormat pf, int array_index, int array_size,
		int level)
		: array_index_(array_index), array_size_(array_size), level_(level)
	{
		BOOST_ASSERT(Texture::TT_2D == texture_2d->Type());
		BOOST_ASSERT((1 == array_size) || ((0 == array_index) && (static_cast<uint32_t>(array_size) == texture_2d->ArraySize())));

		if (array_index > 0)
		{
			TERRC(std::errc::function_not_supported);
		}

		tex_ = texture_2d;
		gl_tex_ = checked_cast<OGLESTexture*>(tex_.get())->GLTexture();

		width_ = texture_2d->Width(level);
		height_ = texture_2d->Height(level);
		pf_ = pf == EF_Unknown ? texture_2d->Format() : pf;
		sample_count_ = texture_2d->SampleCount();
		sample_quality_ = texture_2d->SampleQuality();
	}

	void OGLESTexture2DRenderView::ClearColor(Color const & clr)
	{
		if (gl_fbo_ != 0)
		{
			this->DoClear(GL_COLOR_BUFFER_BIT, clr, 0, 0);
		}
		else
		{
			GLenum const gl_target = checked_cast<OGLESTexture*>(tex_.get())->GLType();

			OGLESRenderEngine& re = *checked_cast<OGLESRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			re.BindTexture(0, gl_target, gl_tex_);

			std::vector<Color> mem_clr(width_ * height_, clr);
			glTexSubImage2D(gl_target, level_, 0, 0, width_, height_, GL_RGBA, GL_FLOAT, &mem_clr[0]);
		}
	}

	void OGLESTexture2DRenderView::Discard()
	{
		this->DoDiscardColor();
	}

	void OGLESTexture2DRenderView::OnAttached(FrameBuffer& fb, uint32_t att)
	{
		BOOST_ASSERT(att != FrameBuffer::ATT_DepthStencil);

		index_ = att - FrameBuffer::ATT_Color0;
		gl_fbo_ = checked_cast<OGLESFrameBuffer*>(&fb)->OGLFbo();
		GLenum const gl_target = checked_cast<OGLESTexture*>(tex_.get())->GLType();

		OGLESRenderEngine& re = *checked_cast<OGLESRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		re.BindFramebuffer(gl_fbo_);

		if (GL_TEXTURE_2D == gl_target)
		{
			glFramebufferTexture2D(GL_FRAMEBUFFER,
					GL_COLOR_ATTACHMENT0 + att - FrameBuffer::ATT_Color0, gl_target, gl_tex_, level_);
		}
		else
		{
			if (array_size_ > 1)
			{
				glFramebufferTextureEXT(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + att - FrameBuffer::ATT_Color0, gl_tex_, level_);
			}
			else
			{
				glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + att - FrameBuffer::ATT_Color0,
					gl_tex_, level_, array_index_);
			}
		}

		re.BindFramebuffer(0);
	}

	void OGLESTexture2DRenderView::OnDetached(FrameBuffer& fb, uint32_t att)
	{
		KFL_UNUSED(fb);

		BOOST_ASSERT(att != FrameBuffer::ATT_DepthStencil);
		BOOST_ASSERT(gl_fbo_ == checked_cast<OGLESFrameBuffer*>(&fb)->OGLFbo());

		GLenum const gl_target = checked_cast<OGLESTexture*>(tex_.get())->GLType();

		OGLESRenderEngine& re = *checked_cast<OGLESRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		re.BindFramebuffer(gl_fbo_);

		if (GL_TEXTURE_2D == gl_target)
		{
			glFramebufferTexture2D(GL_FRAMEBUFFER,
					GL_COLOR_ATTACHMENT0 + att - FrameBuffer::ATT_Color0, gl_target, 0, 0);
		}
		else
		{
			if (array_size_ > 1)
			{
				glFramebufferTextureEXT(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + att - FrameBuffer::ATT_Color0, 0, 0);
			}
			else
			{
				glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + att - FrameBuffer::ATT_Color0,
					0, 0, 0);
			}
		}

		re.BindFramebuffer(0);
	}


	OGLESTexture3DRenderView::OGLESTexture3DRenderView(TexturePtr const & texture_3d, ElementFormat pf, int array_index, uint32_t slice,
		int level)
		: slice_(slice), level_(level), copy_to_tex_(0)
	{
		KFL_UNUSED(array_index);

		BOOST_ASSERT(Texture::TT_3D == texture_3d->Type());
		BOOST_ASSERT(texture_3d->Depth(level) > slice);
		BOOST_ASSERT(0 == array_index);

		tex_ = texture_3d;
		gl_tex_ = checked_cast<OGLESTexture*>(tex_.get())->GLTexture();

		width_ = texture_3d->Width(level);
		height_ = texture_3d->Height(level);
		pf_ = pf == EF_Unknown ? texture_3d->Format() : pf;
		sample_count_ = texture_3d->SampleCount();
		sample_quality_ = texture_3d->SampleQuality();
	}

	OGLESTexture3DRenderView::~OGLESTexture3DRenderView()
	{
		if (2 == copy_to_tex_)
		{
			if (Context::Instance().RenderFactoryValid())
			{
				auto& re = *checked_cast<OGLESRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
				re.DeleteTextures(1, &gl_tex_2d_);
			}
			else
			{
				glDeleteTextures(1, &gl_tex_2d_);
			}
		}
	}

	void OGLESTexture3DRenderView::ClearColor(Color const & clr)
	{
		BOOST_ASSERT(gl_fbo_ != 0);

		this->DoClear(GL_COLOR_BUFFER_BIT, clr, 0, 0);
	}

	void OGLESTexture3DRenderView::Discard()
	{
		this->DoDiscardColor();
	}

	void OGLESTexture3DRenderView::OnAttached(FrameBuffer& fb, uint32_t att)
	{
		BOOST_ASSERT(att != FrameBuffer::ATT_DepthStencil);

		index_ = att - FrameBuffer::ATT_Color0;

		gl_fbo_ = checked_cast<OGLESFrameBuffer*>(&fb)->OGLFbo();
		OGLESRenderEngine& re = *checked_cast<OGLESRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		re.BindFramebuffer(gl_fbo_);

		if (0 == copy_to_tex_)
		{
			glFramebufferTexture3DOES(GL_FRAMEBUFFER,
				GL_COLOR_ATTACHMENT0, GL_TEXTURE_3D_OES, gl_tex_, level_, slice_);

			GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
			if (GL_FRAMEBUFFER_COMPLETE == status)
			{
				glGenTextures(1, &gl_tex_2d_);
				glBindTexture(GL_TEXTURE_2D, gl_tex_2d_);
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width_, height_,
					0, GL_RGBA, GL_FLOAT, nullptr);

				copy_to_tex_ = 2;
			}
			else
			{
				copy_to_tex_ = 1;
			}
		}

		if (1 == copy_to_tex_)
		{
			glFramebufferTexture3DOES(GL_FRAMEBUFFER,
					GL_COLOR_ATTACHMENT0 + att - FrameBuffer::ATT_Color0,
					GL_TEXTURE_3D_OES, gl_tex_, level_, slice_);
		}
		else
		{
			BOOST_ASSERT(2 == copy_to_tex_);

			glFramebufferTexture2D(GL_FRAMEBUFFER,
					GL_COLOR_ATTACHMENT0 + att - FrameBuffer::ATT_Color0,
					GL_TEXTURE_2D, gl_tex_2d_, 0);
		}

		re.BindFramebuffer(0);
	}

	void OGLESTexture3DRenderView::OnDetached(FrameBuffer& fb, uint32_t att)
	{
		KFL_UNUSED(fb);

		BOOST_ASSERT(att != FrameBuffer::ATT_DepthStencil);
		BOOST_ASSERT(gl_fbo_ == checked_cast<OGLESFrameBuffer*>(&fb)->OGLFbo());

		OGLESRenderEngine& re = *checked_cast<OGLESRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		re.BindFramebuffer(gl_fbo_);

		BOOST_ASSERT(copy_to_tex_ != 0);
		if (1 == copy_to_tex_)
		{
			glFramebufferTexture3DOES(GL_FRAMEBUFFER,
					GL_COLOR_ATTACHMENT0 + att - FrameBuffer::ATT_Color0,
					GL_TEXTURE_3D_OES, 0, 0, 0);
		}
		else
		{
			BOOST_ASSERT(2 == copy_to_tex_);

			glFramebufferTexture2D(GL_FRAMEBUFFER,
					GL_COLOR_ATTACHMENT0 + att - FrameBuffer::ATT_Color0,
					GL_TEXTURE_2D, 0, 0);

			this->CopyToSlice(att);
		}

		re.BindFramebuffer(0);
	}

	void OGLESTexture3DRenderView::OnUnbind(FrameBuffer& /*fb*/, uint32_t att)
	{
		BOOST_ASSERT(copy_to_tex_ != 0);
		if (2 == copy_to_tex_)
		{
			this->CopyToSlice(att);
		}
	}

	void OGLESTexture3DRenderView::CopyToSlice(uint32_t att)
	{
		BOOST_ASSERT(att != FrameBuffer::ATT_DepthStencil);
		KFL_UNUSED(att);

		OGLESRenderEngine& re = *checked_cast<OGLESRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		re.BindTexture(0, GL_TEXTURE_3D, gl_tex_);
		glCopyTexSubImage3D(GL_TEXTURE_3D, level_, 0, 0, slice_, 0, 0, width_, height_);
	}


	OGLESTextureCubeRenderView::OGLESTextureCubeRenderView(TexturePtr const & texture_cube, ElementFormat pf, int array_index,
		Texture::CubeFaces face, int level)
		: face_(face), level_(level)
	{
		KFL_UNUSED(array_index);

		BOOST_ASSERT(Texture::TT_Cube == texture_cube->Type());
		BOOST_ASSERT(0 == array_index);

		tex_ = texture_cube;
		gl_tex_ = checked_cast<OGLESTexture*>(tex_.get())->GLTexture();

		width_ = texture_cube->Width(level);
		height_ = texture_cube->Height(level);
		pf_ = pf == EF_Unknown ? texture_cube->Format() : pf;
		sample_count_ = texture_cube->SampleCount();
		sample_quality_ = texture_cube->SampleQuality();
	}

	OGLESTextureCubeRenderView::OGLESTextureCubeRenderView(TexturePtr const & texture_cube, ElementFormat pf, int array_index, int level)
		: face_(static_cast<Texture::CubeFaces>(-1)), level_(level)
	{
		KFL_UNUSED(array_index);

		BOOST_ASSERT(Texture::TT_Cube == texture_cube->Type());
		BOOST_ASSERT(0 == array_index);

		tex_ = texture_cube;
		gl_tex_ = checked_cast<OGLESTexture*>(tex_.get())->GLTexture();

		width_ = texture_cube->Width(level);
		height_ = texture_cube->Height(level);
		pf_ = pf == EF_Unknown ? texture_cube->Format() : pf;
		sample_count_ = texture_cube->SampleCount();
		sample_quality_ = texture_cube->SampleQuality();
	}

	void OGLESTextureCubeRenderView::ClearColor(Color const & clr)
	{
		if (gl_fbo_ != 0)
		{
			this->DoClear(GL_COLOR_BUFFER_BIT, clr, 0, 0);
		}
		else
		{
			OGLESRenderEngine& re = *checked_cast<OGLESRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			re.BindTexture(0, GL_TEXTURE_CUBE_MAP, gl_tex_);

			std::vector<Color> mem_clr(width_ * height_, clr);
			glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face_ - Texture::CF_Positive_X,
				level_, 0, 0, width_, height_, GL_RGBA, GL_FLOAT, &mem_clr[0]);
		}
	}

	void OGLESTextureCubeRenderView::Discard()
	{
		this->DoDiscardColor();
	}

	void OGLESTextureCubeRenderView::OnAttached(FrameBuffer& fb, uint32_t att)
	{
		BOOST_ASSERT(att != FrameBuffer::ATT_DepthStencil);

		index_ = att - FrameBuffer::ATT_Color0;

		gl_fbo_ = checked_cast<OGLESFrameBuffer*>(&fb)->OGLFbo();
		OGLESRenderEngine& re = *checked_cast<OGLESRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		re.BindFramebuffer(gl_fbo_);
		if (face_ >= 0)
		{
			GLenum face = GL_TEXTURE_CUBE_MAP_POSITIVE_X + face_ - Texture::CF_Positive_X;
			glFramebufferTexture2D(GL_FRAMEBUFFER,
					GL_COLOR_ATTACHMENT0 + att - FrameBuffer::ATT_Color0,
					face, gl_tex_, level_);
		}
		else
		{
			glFramebufferTextureEXT(GL_FRAMEBUFFER,
				GL_COLOR_ATTACHMENT0 + att - FrameBuffer::ATT_Color0,
				gl_tex_, level_);
		}
		re.BindFramebuffer(0);
	}

	void OGLESTextureCubeRenderView::OnDetached(FrameBuffer& fb, uint32_t att)
	{
		KFL_UNUSED(fb);

		BOOST_ASSERT(att != FrameBuffer::ATT_DepthStencil);
		BOOST_ASSERT(gl_fbo_ == checked_cast<OGLESFrameBuffer*>(&fb)->OGLFbo());

		OGLESRenderEngine& re = *checked_cast<OGLESRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		re.BindFramebuffer(gl_fbo_);

		GLenum face = GL_TEXTURE_CUBE_MAP_POSITIVE_X + face_ - Texture::CF_Positive_X;
		if (face_ >= 0)
		{
			glFramebufferTexture2D(GL_FRAMEBUFFER,
					GL_COLOR_ATTACHMENT0 + att - FrameBuffer::ATT_Color0,
					face, 0, 0);
		}
		else
		{
			glFramebufferTextureEXT(GL_FRAMEBUFFER,
					GL_COLOR_ATTACHMENT0 + att - FrameBuffer::ATT_Color0,
					0, 0);
		}

		re.BindFramebuffer(0);
	}


	OGLESDepthStencilRenderView::OGLESDepthStencilRenderView(uint32_t width, uint32_t height,
									ElementFormat pf, uint32_t sample_count, uint32_t sample_quality)
		: target_type_(0), array_index_(0), level_(-1),
			sample_count_(sample_count), sample_quality_(sample_quality)
	{
		KFL_UNUSED(array_index_);
		KFL_UNUSED(level_);
		KFL_UNUSED(sample_count_);
		KFL_UNUSED(sample_quality_);
		BOOST_ASSERT(IsDepthFormat(pf));

		width_ = width;
		height_ = height;
		pf_ = pf;
		sample_count_ = sample_count;
		sample_quality_ = sample_quality;

		GLint internalFormat;
		GLenum glformat;
		GLenum gltype;
		OGLESMapping::MappingFormat(internalFormat, glformat, gltype, pf_);

		switch (internalFormat)
		{
		case GL_DEPTH_COMPONENT:
			internalFormat = GL_DEPTH_COMPONENT16;
			break;

		case GL_DEPTH_STENCIL_OES:
			internalFormat = GL_DEPTH24_STENCIL8_OES;
			break;

		default:
			break;
		}

		glGenRenderbuffers(1, gl_rbos_);
		glBindRenderbuffer(GL_RENDERBUFFER, gl_rbos_[0]);
		glRenderbufferStorage(GL_RENDERBUFFER,
								internalFormat, width_, height_);
		gl_rbos_[1] = gl_rbos_[0];
	}

	OGLESDepthStencilRenderView::OGLESDepthStencilRenderView(TexturePtr const & texture, ElementFormat pf, int array_index, int array_size,
		int level)
		: target_type_(checked_cast<OGLESTexture*>(texture.get())->GLType()),
			array_index_(array_index), array_size_(array_size), level_(level)
	{
		BOOST_ASSERT((Texture::TT_2D == texture->Type()) || (Texture::TT_Cube == texture->Type()));
		BOOST_ASSERT((1 == array_size) || ((0 == array_index) && (static_cast<uint32_t>(array_size) == texture->ArraySize())));
		BOOST_ASSERT(IsDepthFormat(texture->Format()));

		tex_ = texture;
		gl_tex_ = checked_cast<OGLESTexture*>(texture.get())->GLTexture();

		width_ = texture->Width(level);
		height_ = texture->Height(level);
		pf_ = pf == EF_Unknown ? texture->Format() : pf;
		sample_count_ = texture->SampleCount();
		sample_quality_ = texture->SampleQuality();
	}

	OGLESDepthStencilRenderView::~OGLESDepthStencilRenderView()
	{
		if (gl_rbos_[0] == gl_rbos_[1])
		{
			glDeleteRenderbuffers(1, gl_rbos_);
		}
		else
		{
			glDeleteRenderbuffers(2, gl_rbos_);
		}
	}

	void OGLESDepthStencilRenderView::ClearColor(Color const & /*clr*/)
	{
		KFL_UNREACHABLE("Can't be called");
	}

	void OGLESDepthStencilRenderView::Discard()
	{
		this->DoDiscardDepthStencil();
	}

	void OGLESDepthStencilRenderView::OnAttached(FrameBuffer& fb, uint32_t att)
	{
		KFL_UNUSED(att);

		BOOST_ASSERT(FrameBuffer::ATT_DepthStencil == att);

		index_ = 0;

		gl_fbo_ = checked_cast<OGLESFrameBuffer*>(&fb)->OGLFbo();
		if (level_ < 0)
		{
			OGLESRenderEngine& re = *checked_cast<OGLESRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			re.BindFramebuffer(gl_fbo_);

			glFramebufferRenderbuffer(GL_FRAMEBUFFER,
									GL_DEPTH_ATTACHMENT,
									GL_RENDERBUFFER, gl_rbos_[0]);
			if (IsStencilFormat(pf_))
			{
				glFramebufferRenderbuffer(GL_FRAMEBUFFER,
									GL_STENCIL_ATTACHMENT,
									GL_RENDERBUFFER, gl_rbos_[1]);
			}

			re.BindFramebuffer(0);
		}
		else
		{
			OGLESRenderEngine& re = *checked_cast<OGLESRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			if (GL_TEXTURE_2D == target_type_)
			{
				re.BindFramebuffer(gl_fbo_);

				if (IsDepthFormat(pf_))
				{
					glFramebufferTexture2D(GL_FRAMEBUFFER,
						GL_DEPTH_ATTACHMENT, target_type_, gl_tex_, level_);
				}
				if (IsStencilFormat(pf_))
				{
					glFramebufferTexture2D(GL_FRAMEBUFFER,
						GL_STENCIL_ATTACHMENT, target_type_, gl_tex_, level_);
				}

				re.BindFramebuffer(0);
			}
			else if (GL_TEXTURE_CUBE_MAP == target_type_)
			{
				re.BindFramebuffer(gl_fbo_);

				if (IsDepthFormat(pf_))
				{
					glFramebufferTextureEXT(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, gl_tex_, level_);
				}
				if (IsStencilFormat(pf_))
				{
					glFramebufferTextureEXT(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, gl_tex_, level_);
				}

				re.BindFramebuffer(0);
			}
			else
			{
				re.BindFramebuffer(gl_fbo_);

				if (array_size_ > 1)
				{
					if (IsDepthFormat(pf_))
					{
						glFramebufferTextureEXT(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, gl_tex_, level_);
					}
					if (IsStencilFormat(pf_))
					{
						glFramebufferTextureEXT(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, gl_tex_, level_);
					}
				}
				else
				{
					if (IsDepthFormat(pf_))
					{
						glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, gl_tex_, level_, array_index_);
					}
					if (IsStencilFormat(pf_))
					{
						glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, gl_tex_, level_, array_index_);
					}
				}

				re.BindFramebuffer(0);
			}
		}
	}

	void OGLESDepthStencilRenderView::OnDetached(FrameBuffer& fb, uint32_t att)
	{
		KFL_UNUSED(fb);
		KFL_UNUSED(att);

		BOOST_ASSERT(FrameBuffer::ATT_DepthStencil == att);

		BOOST_ASSERT(gl_fbo_ == checked_cast<OGLESFrameBuffer*>(&fb)->OGLFbo());
		if (level_ < 0)
		{
			OGLESRenderEngine& re = *checked_cast<OGLESRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			re.BindFramebuffer(gl_fbo_);

			glFramebufferRenderbuffer(GL_FRAMEBUFFER,
									GL_DEPTH_ATTACHMENT,
									GL_RENDERBUFFER, 0);
			glFramebufferRenderbuffer(GL_FRAMEBUFFER,
									GL_STENCIL_ATTACHMENT,
									GL_RENDERBUFFER, 0);

			re.BindFramebuffer(0);
		}
		else
		{
			OGLESRenderEngine& re = *checked_cast<OGLESRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			if (GL_TEXTURE_2D == target_type_)
			{
				re.BindFramebuffer(gl_fbo_);

				if (IsDepthFormat(pf_))
				{
					glFramebufferTexture2D(GL_FRAMEBUFFER,
						GL_DEPTH_ATTACHMENT, target_type_, 0, 0);
				}
				if (IsStencilFormat(pf_))
				{
					glFramebufferTexture2D(GL_FRAMEBUFFER,
						GL_STENCIL_ATTACHMENT, target_type_, 0, 0);
				}

				re.BindFramebuffer(0);
			}
			else if (GL_TEXTURE_CUBE_MAP == target_type_)
			{
				re.BindFramebuffer(gl_fbo_);

				if (IsDepthFormat(pf_))
				{
					glFramebufferTextureEXT(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, 0, 0);
				}
				if (IsStencilFormat(pf_))
				{
					glFramebufferTextureEXT(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, 0, 0);
				}

				re.BindFramebuffer(0);
			}
			else
			{
				re.BindFramebuffer(gl_fbo_);

				if (array_size_ > 1)
				{
					if (IsDepthFormat(pf_))
					{
						glFramebufferTextureEXT(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, 0, 0);
					}
					if (IsStencilFormat(pf_))
					{
						glFramebufferTextureEXT(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, 0, 0);
					}
				}
				else
				{
					if (IsDepthFormat(pf_))
					{
						glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, 0, 0, 0);
					}
					if (IsStencilFormat(pf_))
					{
						glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, 0, 0, 0);
					}
				}

				re.BindFramebuffer(0);
			}
		}
	}


	OGLESTextureCubeDepthStencilRenderView::OGLESTextureCubeDepthStencilRenderView(TexturePtr const & texture_cube, ElementFormat pf,
		int array_index, Texture::CubeFaces face, int level)
		: face_(face), level_(level)
	{
		BOOST_ASSERT(Texture::TT_Cube == texture_cube->Type());
		BOOST_ASSERT(IsDepthFormat(texture_cube->Format()));

		if (array_index > 0)
		{
			TERRC(std::errc::function_not_supported);
		}

		tex_ = texture_cube;
		gl_tex_ = checked_cast<OGLESTexture*>(tex_.get())->GLTexture();

		width_ = texture_cube->Width(level);
		height_ = texture_cube->Height(level);
		pf_ = pf == EF_Unknown ? texture_cube->Format() : pf;
		sample_count_ = texture_cube->SampleCount();
		sample_quality_ = texture_cube->SampleQuality();
	}

	void OGLESTextureCubeDepthStencilRenderView::ClearColor(Color const & /*clr*/)
	{
		KFL_UNREACHABLE("Can't be called");
	}

	void OGLESTextureCubeDepthStencilRenderView::Discard()
	{
		this->DoDiscardDepthStencil();
	}

	void OGLESTextureCubeDepthStencilRenderView::OnAttached(FrameBuffer& fb, uint32_t att)
	{
		KFL_UNUSED(att);

		BOOST_ASSERT(FrameBuffer::ATT_DepthStencil == att);

		index_ = 0;

		gl_fbo_ = checked_cast<OGLESFrameBuffer*>(&fb)->OGLFbo();
		GLenum face = GL_TEXTURE_CUBE_MAP_POSITIVE_X + face_ - Texture::CF_Positive_X;
		OGLESRenderEngine& re = *checked_cast<OGLESRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		re.BindFramebuffer(gl_fbo_);

		if (IsDepthFormat(pf_))
		{
			glFramebufferTexture2D(GL_FRAMEBUFFER,
				GL_DEPTH_ATTACHMENT, face, gl_tex_, level_);
		}
		if (IsStencilFormat(pf_))
		{
			glFramebufferTexture2D(GL_FRAMEBUFFER,
				GL_STENCIL_ATTACHMENT, face, gl_tex_, level_);
		}

		re.BindFramebuffer(0);
	}

	void OGLESTextureCubeDepthStencilRenderView::OnDetached(FrameBuffer& fb, uint32_t att)
	{
		KFL_UNUSED(fb);
		KFL_UNUSED(att);

		BOOST_ASSERT(FrameBuffer::ATT_DepthStencil == att);

		GLenum face = GL_TEXTURE_CUBE_MAP_POSITIVE_X + face_ - Texture::CF_Positive_X;
		OGLESRenderEngine& re = *checked_cast<OGLESRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		re.BindFramebuffer(gl_fbo_);

		if (IsDepthFormat(pf_))
		{
			glFramebufferTexture2D(GL_FRAMEBUFFER,
				GL_DEPTH_ATTACHMENT, face, 0, 0);
		}
		if (IsStencilFormat(pf_))
		{
			glFramebufferTexture2D(GL_FRAMEBUFFER,
				GL_STENCIL_ATTACHMENT, face, 0, 0);
		}

		re.BindFramebuffer(0);
	}

#if defined(KLAYGE_PLATFORM_IOS)
	OGLESEAGLRenderView::OGLESEAGLRenderView(ElementFormat pf)
	{
		glGenRenderbuffers(1, &gl_rf_);
		glBindRenderbuffer(GL_RENDERBUFFER, gl_rf_);
		
		WindowPtr const & app_window = KlayGE::Context::Instance().AppInstance().MainWnd();
		app_window->CreateColorRenderBuffer(pf);
		
		pf_ = pf;
		GLint param;
		glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &param);
		width_ = param;
		glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &param);
		height_ = param;
		glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_SAMPLES, &param);
		sample_count_ = param;
		sample_quality_ = 0;
	}
	
	void OGLESEAGLRenderView::ClearColor(Color const & clr)
	{
		this->DoClear(GL_COLOR_BUFFER_BIT, clr, 0, 0);
	}
	void OGLESEAGLRenderView::ClearDepth(float)
	{
		KFL_UNREACHABLE("Can't be called");
	}
	
	void OGLESEAGLRenderView::Discard()
	{
		this->DoDiscardColor();
	}
	
	void OGLESEAGLRenderView::OnAttached(FrameBuffer& fb, uint32_t att)
	{
		BOOST_ASSERT(att != FrameBuffer::ATT_DepthStencil);
		
		index_ = att - FrameBuffer::ATT_Color0;
		gl_fbo_ = checked_cast<OGLESFrameBuffer*>(&fb)->OGLFbo();
		
		OGLESRenderEngine& re = *checked_cast<OGLESRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		re.BindFramebuffer(gl_fbo_);
		
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
								  GL_RENDERBUFFER, gl_rf_);
		
		re.BindFramebuffer(0);
	}

	void OGLESEAGLRenderView::OnDetached(FrameBuffer& fb, uint32_t att)
	{
		KFL_UNUSED(fb);
		KFL_UNUSED(att);
		
		BOOST_ASSERT(att != FrameBuffer::ATT_DepthStencil);
		BOOST_ASSERT(gl_fbo_ == checked_cast<OGLESFrameBuffer*>(&fb)->OGLFbo());
		
		OGLESRenderEngine& re = *checked_cast<OGLESRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		re.BindFramebuffer(gl_fbo_);
		
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
								  GL_RENDERBUFFER, gl_rf_);
		
		re.BindFramebuffer(0);
	}

	void OGLESEAGLRenderView::BindRenderBuffer()
	{
		glBindRenderbuffer(GL_RENDERBUFFER, gl_rf_);
	}
#endif
}
