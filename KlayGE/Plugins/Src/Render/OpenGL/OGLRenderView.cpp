// OGLRenderView.cpp
// KlayGE OGL渲染视图类 实现文件
// Ver 3.9.0
// 版权所有(C) 龚敏敏, 2006-2009
// Homepage: http://www.klayge.org
//
// 3.9.0
// 支持Texture Array (2009.8.5)
//
// 3.3.0
// 初次建立 (2006.5.31)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KFL/ErrorHandling.hpp>
#include <KFL/Util.hpp>
#include <KFL/Math.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/RenderFactory.hpp>

#include <boost/assert.hpp>

#include <KlayGE/OpenGL/OGLMapping.hpp>
#include <KlayGE/OpenGL/OGLTexture.hpp>
#include <KlayGE/OpenGL/OGLGraphicsBuffer.hpp>
#include <KlayGE/OpenGL/OGLFrameBuffer.hpp>
#include <KlayGE/OpenGL/OGLRenderEngine.hpp>
#include <KlayGE/OpenGL/OGLRenderView.hpp>

namespace KlayGE
{
	OGLRenderView::OGLRenderView()
		: gl_tex_(0), gl_fbo_(0)
	{
	}

	OGLRenderView::~OGLRenderView()
	{
	}

	void OGLRenderView::ClearDepth(float depth)
	{
		this->DoClear(GL_DEPTH_BUFFER_BIT, Color(), depth, 0);
	}

	void OGLRenderView::ClearStencil(int32_t stencil)
	{
		this->DoClear(GL_STENCIL_BUFFER_BIT, Color(), 0, stencil);
	}

	void OGLRenderView::ClearDepthStencil(float depth, int32_t stencil)
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

	void OGLRenderView::DoClear(uint32_t flags, Color const & clr, float depth, int32_t stencil)
	{
		OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());

		GLuint old_fbo = re.BindFramebuffer();
		re.BindFramebuffer(gl_fbo_);

		DepthStencilStateDesc const & ds_desc = re.CurRenderStateObject()->GetDepthStencilStateDesc();
		BlendStateDesc const & blend_desc = re.CurRenderStateObject()->GetBlendStateDesc();

		if (flags & GL_COLOR_BUFFER_BIT)
		{
			for (int i = 0; i < 8; ++ i)
			{
				if (blend_desc.color_write_mask[i] != CMASK_All)
				{
					glColorMaski(i, true, true, true, true);
				}
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
			for (int i = 0; i < 8; ++ i)
			{
				if (blend_desc.color_write_mask[i] != CMASK_All)
				{
					glColorMaski(i, (blend_desc.color_write_mask[i] & CMASK_Red) != 0,
						(blend_desc.color_write_mask[i] & CMASK_Green) != 0,
						(blend_desc.color_write_mask[i] & CMASK_Blue) != 0,
						(blend_desc.color_write_mask[i] & CMASK_Alpha) != 0);
				}
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

	void OGLRenderView::DoDiscardColor()
	{
		if (glloader_GL_VERSION_4_3() || glloader_GL_ARB_invalidate_subdata())
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

			if (glloader_GL_VERSION_4_5() || glloader_GL_ARB_direct_state_access())
			{
				glInvalidateNamedFramebufferData(gl_fbo_, 1, &attachment);
			}
			else
			{
				OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());

				GLuint old_fbo = re.BindFramebuffer();
				re.BindFramebuffer(gl_fbo_);

				glInvalidateFramebuffer(GL_FRAMEBUFFER, 1, &attachment);

				re.BindFramebuffer(old_fbo);
			}
		}
		else
		{
			this->ClearColor(Color(0, 0, 0, 0));
		}
	}

	void OGLRenderView::DoDiscardDepthStencil()
	{
		if (glloader_GL_VERSION_4_3() || glloader_GL_ARB_invalidate_subdata())
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

			if (glloader_GL_VERSION_4_5() || glloader_GL_ARB_direct_state_access())
			{
				glInvalidateNamedFramebufferData(gl_fbo_, 2, attachments);
			}
			else
			{
				OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());

				GLuint old_fbo = re.BindFramebuffer();
				re.BindFramebuffer(gl_fbo_);

				glInvalidateFramebuffer(GL_FRAMEBUFFER, 2, attachments);

				re.BindFramebuffer(old_fbo);
			}
		}
		else
		{
			this->ClearDepthStencil(1, 0);
		}
	}


	OGLScreenColorRenderView::OGLScreenColorRenderView(uint32_t width, uint32_t height, ElementFormat pf)
	{
		width_ = width;
		height_ = height;
		pf_ = pf;
		sample_count_ = 1;
		sample_quality_ = 0;
	}

	void OGLScreenColorRenderView::ClearColor(Color const & clr)
	{
		this->DoClear(GL_COLOR_BUFFER_BIT, clr, 0, 0);
	}

	void OGLScreenColorRenderView::ClearDepth(float /*depth*/)
	{
		KFL_UNREACHABLE("Can't be called");
	}

	void OGLScreenColorRenderView::ClearStencil(int32_t /*stencil*/)
	{
		KFL_UNREACHABLE("Can't be called");
	}

	void OGLScreenColorRenderView::ClearDepthStencil(float /*depth*/, int32_t /*stencil*/)
	{
		KFL_UNREACHABLE("Can't be called");
	}

	void OGLScreenColorRenderView::Discard()
	{
		this->DoDiscardColor();
	}

	void OGLScreenColorRenderView::OnAttached(FrameBuffer& fb, uint32_t att)
	{
		KFL_UNUSED(fb);

		BOOST_ASSERT(0 == checked_cast<OGLFrameBuffer*>(&fb)->OGLFbo());

		index_ = att - FrameBuffer::ATT_Color0;

		OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		re.BindFramebuffer(0);
	}

	void OGLScreenColorRenderView::OnDetached(FrameBuffer& fb, uint32_t /*att*/)
	{
		KFL_UNUSED(fb);

		BOOST_ASSERT(0 == checked_cast<OGLFrameBuffer*>(&fb)->OGLFbo());

		OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		re.BindFramebuffer(0);
	}


	OGLScreenDepthStencilRenderView::OGLScreenDepthStencilRenderView(uint32_t width, uint32_t height,
									ElementFormat pf)
	{
		BOOST_ASSERT(IsDepthFormat(pf));

		width_ = width;
		height_ = height;
		pf_ = pf;
		sample_count_ = 1;
		sample_quality_ = 0;
	}

	void OGLScreenDepthStencilRenderView::ClearColor(Color const & /*clr*/)
	{
		KFL_UNREACHABLE("Can't be called");
	}

	void OGLScreenDepthStencilRenderView::Discard()
	{
		this->DoDiscardDepthStencil();
	}

	void OGLScreenDepthStencilRenderView::OnAttached(FrameBuffer& fb, uint32_t /*att*/)
	{
		KFL_UNUSED(fb);

		BOOST_ASSERT(0 == checked_cast<OGLFrameBuffer*>(&fb)->OGLFbo());

		index_ = 0;

		OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		re.BindFramebuffer(0);
	}

	void OGLScreenDepthStencilRenderView::OnDetached(FrameBuffer& fb, uint32_t /*att*/)
	{
		KFL_UNUSED(fb);

		BOOST_ASSERT(0 == checked_cast<OGLFrameBuffer*>(&fb)->OGLFbo());

		OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		re.BindFramebuffer(0);
	}


	OGLTexture1DRenderView::OGLTexture1DRenderView(TexturePtr const & texture_1d, ElementFormat pf, int array_index, int array_size,
		int level)
		: array_index_(array_index), array_size_(array_size), level_(level)
	{
		BOOST_ASSERT(Texture::TT_1D == texture_1d->Type());
		BOOST_ASSERT((1 == array_size) || ((0 == array_index) && (static_cast<uint32_t>(array_size) == texture_1d->ArraySize())));

		tex_ = texture_1d;
		gl_tex_ = checked_cast<OGLTexture*>(tex_.get())->GLTexture();

		width_ = texture_1d->Width(level);
		height_ = 1;
		pf_ = pf == EF_Unknown ? texture_1d->Format() : pf;
		sample_count_ = texture_1d->SampleCount();
		sample_quality_ = texture_1d->SampleQuality();
	}

	void OGLTexture1DRenderView::ClearColor(Color const & clr)
	{
		if (gl_fbo_ != 0)
		{
			this->DoClear(GL_COLOR_BUFFER_BIT, clr, 0, 0);
		}
		else
		{
			GLenum const gl_target = checked_cast<OGLTexture*>(tex_.get())->GLType();

			OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			re.BindTexture(0, gl_target, gl_tex_);

			std::vector<Color> mem_clr(width_, clr);
			glTexSubImage1D(gl_target, level_, 0, width_, GL_RGBA, GL_FLOAT, &mem_clr[0]);
		}
	}

	void OGLTexture1DRenderView::Discard()
	{
		this->DoDiscardColor();
	}

	void OGLTexture1DRenderView::OnAttached(FrameBuffer& fb, uint32_t att)
	{
		BOOST_ASSERT(att != FrameBuffer::ATT_DepthStencil);

		index_ = att - FrameBuffer::ATT_Color0;
		gl_fbo_ = checked_cast<OGLFrameBuffer*>(&fb)->OGLFbo();
		GLenum const gl_target = checked_cast<OGLTexture*>(tex_.get())->GLType();

		OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		if (GL_TEXTURE_1D == gl_target)
		{
			if (sample_count_ <= 1)
			{
				if (glloader_GL_VERSION_4_5() || glloader_GL_ARB_direct_state_access())
				{
					glNamedFramebufferTexture(gl_fbo_,
							GL_COLOR_ATTACHMENT0 + att - FrameBuffer::ATT_Color0, gl_tex_, level_);
				}
				else if (glloader_GL_EXT_direct_state_access())
				{
					glNamedFramebufferTexture1DEXT(gl_fbo_,
							GL_COLOR_ATTACHMENT0 + att - FrameBuffer::ATT_Color0, gl_target, gl_tex_, level_);
				}
				else
				{
					re.BindFramebuffer(gl_fbo_);

					glFramebufferTexture1D(GL_FRAMEBUFFER,
							GL_COLOR_ATTACHMENT0 + att - FrameBuffer::ATT_Color0, gl_target, gl_tex_, level_);

					re.BindFramebuffer(0);
				}
			}
			else
			{
				if (glloader_GL_VERSION_4_5() || glloader_GL_ARB_direct_state_access())
				{
					glNamedFramebufferRenderbuffer(gl_fbo_,
											GL_COLOR_ATTACHMENT0 + att - FrameBuffer::ATT_Color0,
											GL_RENDERBUFFER, gl_tex_);
				}
				else if (glloader_GL_EXT_direct_state_access())
				{
					glNamedFramebufferRenderbufferEXT(gl_fbo_,
											GL_COLOR_ATTACHMENT0 + att - FrameBuffer::ATT_Color0,
											GL_RENDERBUFFER, gl_tex_);
				}
				else
				{
					re.BindFramebuffer(gl_fbo_);

					glFramebufferRenderbuffer(GL_FRAMEBUFFER,
											GL_COLOR_ATTACHMENT0 + att - FrameBuffer::ATT_Color0,
											GL_RENDERBUFFER, gl_tex_);

					re.BindFramebuffer(0);
				}
			}
		}
		else
		{
			if (glloader_GL_VERSION_4_5() || glloader_GL_ARB_direct_state_access())
			{
				if (array_size_ > 1)
				{
					glNamedFramebufferTexture(gl_fbo_, GL_COLOR_ATTACHMENT0 + att - FrameBuffer::ATT_Color0, gl_tex_, level_);
				}
				else
				{
					glNamedFramebufferTextureLayer(gl_fbo_, GL_COLOR_ATTACHMENT0 + att - FrameBuffer::ATT_Color0,
						gl_tex_, level_, array_index_);
				}
			}
			else if (glloader_GL_EXT_direct_state_access())
			{
				if (array_size_ > 1)
				{
					glNamedFramebufferTextureEXT(gl_fbo_, GL_COLOR_ATTACHMENT0 + att - FrameBuffer::ATT_Color0, gl_tex_, level_);
				}
				else
				{
					glNamedFramebufferTextureLayerEXT(gl_fbo_, GL_COLOR_ATTACHMENT0 + att - FrameBuffer::ATT_Color0,
						gl_tex_, level_, array_index_);
				}
			}
			else
			{
				re.BindFramebuffer(gl_fbo_);

				if (array_size_ > 1)
				{
					glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + att - FrameBuffer::ATT_Color0, gl_tex_, level_);
				}
				else
				{
					glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + att - FrameBuffer::ATT_Color0,
						gl_tex_, level_, array_index_);
				}

				re.BindFramebuffer(0);
			}
		}
	}

	void OGLTexture1DRenderView::OnDetached(FrameBuffer& fb, uint32_t att)
	{
		KFL_UNUSED(fb);

		BOOST_ASSERT(att != FrameBuffer::ATT_DepthStencil);

		GLenum const gl_target = checked_cast<OGLTexture*>(tex_.get())->GLType();

		OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		if (GL_TEXTURE_1D == gl_target)
		{
			if (sample_count_ <= 1)
			{
				if (glloader_GL_VERSION_4_5() || glloader_GL_ARB_direct_state_access())
				{
					glNamedFramebufferTexture(gl_fbo_,
							GL_COLOR_ATTACHMENT0 + att - FrameBuffer::ATT_Color0, 0, 0);
				}
				else if (glloader_GL_EXT_direct_state_access())
				{
					glNamedFramebufferTexture1DEXT(gl_fbo_,
							GL_COLOR_ATTACHMENT0 + att - FrameBuffer::ATT_Color0, gl_target, 0, 0);
				}
				else
				{
					re.BindFramebuffer(gl_fbo_);

					glFramebufferTexture1D(GL_FRAMEBUFFER,
							GL_COLOR_ATTACHMENT0 + att - FrameBuffer::ATT_Color0, gl_target, 0, 0);

					re.BindFramebuffer(0);
				}
			}
			else
			{
				if (glloader_GL_VERSION_4_5() || glloader_GL_ARB_direct_state_access())
				{
					glNamedFramebufferRenderbuffer(gl_fbo_,
											GL_COLOR_ATTACHMENT0 + att - FrameBuffer::ATT_Color0,
											GL_RENDERBUFFER, 0);
				}
				else if (glloader_GL_EXT_direct_state_access())
				{
					glNamedFramebufferRenderbufferEXT(gl_fbo_,
											GL_COLOR_ATTACHMENT0 + att - FrameBuffer::ATT_Color0,
											GL_RENDERBUFFER, 0);
				}
				else
				{
					re.BindFramebuffer(gl_fbo_);

					glFramebufferRenderbuffer(GL_FRAMEBUFFER,
											GL_COLOR_ATTACHMENT0 + att - FrameBuffer::ATT_Color0,
											GL_RENDERBUFFER, 0);

					re.BindFramebuffer(0);
				}
			}
		}
		else
		{
			if (glloader_GL_VERSION_4_5() || glloader_GL_ARB_direct_state_access())
			{
				if (array_size_ > 1)
				{
					glNamedFramebufferTexture(gl_fbo_, GL_COLOR_ATTACHMENT0 + att - FrameBuffer::ATT_Color0, 0, 0);
				}
				else
				{
					glNamedFramebufferTextureLayer(gl_fbo_, GL_COLOR_ATTACHMENT0 + att - FrameBuffer::ATT_Color0,
						0, 0, 0);
				}
			}
			else if (glloader_GL_EXT_direct_state_access())
			{
				if (array_size_ > 1)
				{
					glNamedFramebufferTextureEXT(gl_fbo_, GL_COLOR_ATTACHMENT0 + att - FrameBuffer::ATT_Color0, 0, 0);
				}
				else
				{
					glNamedFramebufferTextureLayerEXT(gl_fbo_, GL_COLOR_ATTACHMENT0 + att - FrameBuffer::ATT_Color0,
						0, 0, 0);
				}
			}
			else
			{
				re.BindFramebuffer(gl_fbo_);

				if (array_size_ > 1)
				{
					glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + att - FrameBuffer::ATT_Color0, 0, 0);
				}
				else
				{
					glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + att - FrameBuffer::ATT_Color0,
						0, 0, 0);
				}

				re.BindFramebuffer(0);
			}
		}
	}


	OGLTexture2DRenderView::OGLTexture2DRenderView(TexturePtr const & texture_2d, ElementFormat pf, int array_index, int array_size, int level)
		: array_index_(array_index), array_size_(array_size), level_(level)
	{
		BOOST_ASSERT(Texture::TT_2D == texture_2d->Type());
		BOOST_ASSERT((1 == array_size) || ((0 == array_index) && (static_cast<uint32_t>(array_size) == texture_2d->ArraySize())));

		tex_ = texture_2d;
		gl_tex_ = checked_cast<OGLTexture*>(tex_.get())->GLTexture();

		width_ = texture_2d->Width(level);
		height_ = texture_2d->Height(level);
		pf_ = pf == EF_Unknown ? texture_2d->Format() : pf;
		sample_count_ = texture_2d->SampleCount();
		sample_quality_ = texture_2d->SampleQuality();
	}

	void OGLTexture2DRenderView::ClearColor(Color const & clr)
	{
		if (gl_fbo_ != 0)
		{
			this->DoClear(GL_COLOR_BUFFER_BIT, clr, 0, 0);
		}
		else
		{
			GLenum const gl_target = checked_cast<OGLTexture*>(tex_.get())->GLType();

			OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			re.BindTexture(0, gl_target, gl_tex_);

			std::vector<Color> mem_clr(width_ * height_, clr);
			glTexSubImage2D(gl_target, level_, 0, 0, width_, height_, GL_RGBA, GL_FLOAT, &mem_clr[0]);
		}
	}

	void OGLTexture2DRenderView::Discard()
	{
		this->DoDiscardColor();
	}

	void OGLTexture2DRenderView::OnAttached(FrameBuffer& fb, uint32_t att)
	{
		BOOST_ASSERT(att != FrameBuffer::ATT_DepthStencil);

		index_ = att - FrameBuffer::ATT_Color0;
		gl_fbo_ = checked_cast<OGLFrameBuffer*>(&fb)->OGLFbo();
		GLenum const gl_target = checked_cast<OGLTexture*>(tex_.get())->GLType();

		OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		if (GL_TEXTURE_2D == gl_target)
		{
			if (sample_count_ <= 1)
			{
				if (glloader_GL_VERSION_4_5() || glloader_GL_ARB_direct_state_access())
				{
					glNamedFramebufferTexture(gl_fbo_,
						GL_COLOR_ATTACHMENT0 + att - FrameBuffer::ATT_Color0, gl_tex_, level_);
				}
				else if (glloader_GL_EXT_direct_state_access())
				{
					glNamedFramebufferTexture2DEXT(gl_fbo_,
						GL_COLOR_ATTACHMENT0 + att - FrameBuffer::ATT_Color0, gl_target, gl_tex_, level_);
				}
				else
				{
					re.BindFramebuffer(gl_fbo_);

					glFramebufferTexture2D(GL_FRAMEBUFFER,
						GL_COLOR_ATTACHMENT0 + att - FrameBuffer::ATT_Color0, gl_target, gl_tex_, level_);

					re.BindFramebuffer(0);
				}
			}
			else
			{
				if (glloader_GL_VERSION_4_5() || glloader_GL_ARB_direct_state_access())
				{
					glNamedFramebufferRenderbuffer(gl_fbo_,
											GL_COLOR_ATTACHMENT0 + att - FrameBuffer::ATT_Color0,
											GL_RENDERBUFFER, gl_tex_);
				}
				else if (glloader_GL_EXT_direct_state_access())
				{
					glNamedFramebufferRenderbufferEXT(gl_fbo_,
											GL_COLOR_ATTACHMENT0 + att - FrameBuffer::ATT_Color0,
											GL_RENDERBUFFER, gl_tex_);
				}
				else
				{
					re.BindFramebuffer(gl_fbo_);

					glFramebufferRenderbuffer(GL_FRAMEBUFFER,
											GL_COLOR_ATTACHMENT0 + att - FrameBuffer::ATT_Color0,
											GL_RENDERBUFFER, gl_tex_);

					re.BindFramebuffer(0);
				}
			}
		}
		else
		{
			if (glloader_GL_VERSION_4_5() || glloader_GL_ARB_direct_state_access())
			{
				if (array_size_ > 1)
				{
					glNamedFramebufferTexture(gl_fbo_, GL_COLOR_ATTACHMENT0 + att - FrameBuffer::ATT_Color0, gl_tex_, level_);
				}
				else
				{
					glNamedFramebufferTextureLayer(gl_fbo_, GL_COLOR_ATTACHMENT0 + att - FrameBuffer::ATT_Color0,
						gl_tex_, level_, array_index_);
				}
			}
			else if (glloader_GL_EXT_direct_state_access())
			{
				if (array_size_ > 1)
				{
					glNamedFramebufferTextureEXT(gl_fbo_, GL_COLOR_ATTACHMENT0 + att - FrameBuffer::ATT_Color0, gl_tex_, level_);
				}
				else
				{
					glNamedFramebufferTextureLayerEXT(gl_fbo_, GL_COLOR_ATTACHMENT0 + att - FrameBuffer::ATT_Color0,
						gl_tex_, level_, array_index_);
				}
			}
			else
			{
				re.BindFramebuffer(gl_fbo_);

				if (array_size_ > 1)
				{
					glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + att - FrameBuffer::ATT_Color0, gl_tex_, level_);
				}
				else
				{
					glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + att - FrameBuffer::ATT_Color0,
						gl_tex_, level_, array_index_);
				}

				re.BindFramebuffer(0);
			}
		}
	}

	void OGLTexture2DRenderView::OnDetached(FrameBuffer& fb, uint32_t att)
	{
		KFL_UNUSED(fb);

		BOOST_ASSERT(att != FrameBuffer::ATT_DepthStencil);

		GLenum const gl_target = checked_cast<OGLTexture*>(tex_.get())->GLType();

		OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		if (GL_TEXTURE_2D == gl_target)
		{
			if (sample_count_ <= 1)
			{
				if (glloader_GL_VERSION_4_5() || glloader_GL_ARB_direct_state_access())
				{
					glNamedFramebufferTexture(gl_fbo_,
							GL_COLOR_ATTACHMENT0 + att - FrameBuffer::ATT_Color0, 0, 0);
				}
				else if (glloader_GL_EXT_direct_state_access())
				{
					glNamedFramebufferTexture2DEXT(gl_fbo_,
							GL_COLOR_ATTACHMENT0 + att - FrameBuffer::ATT_Color0, gl_target, 0, 0);
				}
				else
				{
					re.BindFramebuffer(gl_fbo_);

					glFramebufferTexture2D(GL_FRAMEBUFFER,
							GL_COLOR_ATTACHMENT0 + att - FrameBuffer::ATT_Color0, gl_target, 0, 0);

					re.BindFramebuffer(0);
				}
			}
			else
			{
				if (glloader_GL_VERSION_4_5() || glloader_GL_ARB_direct_state_access())
				{
					glNamedFramebufferRenderbuffer(gl_fbo_,
											GL_COLOR_ATTACHMENT0 + att - FrameBuffer::ATT_Color0,
											GL_RENDERBUFFER, 0);
				}
				else if (glloader_GL_EXT_direct_state_access())
				{
					glNamedFramebufferRenderbufferEXT(gl_fbo_,
											GL_COLOR_ATTACHMENT0 + att - FrameBuffer::ATT_Color0,
											GL_RENDERBUFFER, 0);
				}
				else
				{
					re.BindFramebuffer(gl_fbo_);

					glFramebufferRenderbuffer(GL_FRAMEBUFFER,
											GL_COLOR_ATTACHMENT0 + att - FrameBuffer::ATT_Color0,
											GL_RENDERBUFFER, 0);

					re.BindFramebuffer(0);
				}
			}
		}
		else
		{
			if (glloader_GL_VERSION_4_5() || glloader_GL_ARB_direct_state_access())
			{
				if (array_size_ > 1)
				{
					glNamedFramebufferTexture(gl_fbo_, GL_COLOR_ATTACHMENT0 + att - FrameBuffer::ATT_Color0, 0, 0);
				}
				else
				{
					glNamedFramebufferTextureLayer(gl_fbo_, GL_COLOR_ATTACHMENT0 + att - FrameBuffer::ATT_Color0,
						0, 0, 0);
				}
			}
			else if (glloader_GL_EXT_direct_state_access())
			{
				if (array_size_ > 1)
				{
					glNamedFramebufferTextureEXT(gl_fbo_, GL_COLOR_ATTACHMENT0 + att - FrameBuffer::ATT_Color0, 0, 0);
				}
				else
				{
					glNamedFramebufferTextureLayerEXT(gl_fbo_, GL_COLOR_ATTACHMENT0 + att - FrameBuffer::ATT_Color0,
						0, 0, 0);
				}
			}
			else
			{
				re.BindFramebuffer(gl_fbo_);

				if (array_size_ > 1)
				{
					glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + att - FrameBuffer::ATT_Color0, 0, 0);
				}
				else
				{
					glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + att - FrameBuffer::ATT_Color0,
						0, 0, 0);
				}

				re.BindFramebuffer(0);
			}
		}
	}


	OGLTexture3DRenderView::OGLTexture3DRenderView(TexturePtr const & texture_3d, ElementFormat pf, int array_index, uint32_t slice,
		int level)
		: slice_(slice), level_(level), copy_to_tex_(0)
	{
		KFL_UNUSED(array_index);

		BOOST_ASSERT(Texture::TT_3D == texture_3d->Type());
		BOOST_ASSERT(texture_3d->Depth(level) > slice);
		BOOST_ASSERT(0 == array_index);

		tex_ = texture_3d;
		gl_tex_ = checked_cast<OGLTexture*>(tex_.get())->GLTexture();

		width_ = texture_3d->Width(level);
		height_ = texture_3d->Height(level);
		pf_ = pf == EF_Unknown ? texture_3d->Format() : pf;
		sample_count_ = texture_3d->SampleCount();
		sample_quality_ = texture_3d->SampleQuality();
	}

	OGLTexture3DRenderView::~OGLTexture3DRenderView()
	{
		if (2 == copy_to_tex_)
		{
			if (Context::Instance().RenderFactoryValid())
			{
				auto& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
				re.DeleteTextures(1, &gl_tex_2d_);
			}
			else
			{
				glDeleteTextures(1, &gl_tex_2d_);
			}
		}
	}

	void OGLTexture3DRenderView::ClearColor(Color const & clr)
	{
		BOOST_ASSERT(gl_fbo_ != 0);

		this->DoClear(GL_COLOR_BUFFER_BIT, clr, 0, 0);
	}

	void OGLTexture3DRenderView::Discard()
	{
		this->DoDiscardColor();
	}

	void OGLTexture3DRenderView::OnAttached(FrameBuffer& fb, uint32_t att)
	{
		BOOST_ASSERT(att != FrameBuffer::ATT_DepthStencil);

		index_ = att - FrameBuffer::ATT_Color0;

		OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		gl_fbo_ = checked_cast<OGLFrameBuffer*>(&fb)->OGLFbo();
		if (glloader_GL_EXT_direct_state_access())
		{
			if (0 == copy_to_tex_)
			{
				glNamedFramebufferTexture3DEXT(gl_fbo_,
					GL_COLOR_ATTACHMENT0, GL_TEXTURE_3D, gl_tex_, level_, slice_);

				GLenum status = glCheckNamedFramebufferStatusEXT(gl_fbo_, GL_FRAMEBUFFER);
				if (status != GL_FRAMEBUFFER_COMPLETE)
				{
					glGenTextures(1, &gl_tex_2d_);
					re.BindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
					glTextureImage2DEXT(gl_tex_2d_, GL_TEXTURE_RECTANGLE_ARB,
							   0, GL_RGBA32F_ARB,
							   width_, height_, 0,
							   GL_RGBA, GL_FLOAT, nullptr);

					copy_to_tex_ = 2;
				}
				else
				{
					copy_to_tex_ = 1;
				}
			}

			if (1 == copy_to_tex_)
			{
				glNamedFramebufferTexture3DEXT(gl_fbo_,
						GL_COLOR_ATTACHMENT0 + att - FrameBuffer::ATT_Color0,
						GL_TEXTURE_3D, gl_tex_, level_, slice_);
			}
			else
			{
				BOOST_ASSERT(2 == copy_to_tex_);

				glNamedFramebufferTexture2DEXT(gl_fbo_,
						GL_COLOR_ATTACHMENT0 + att - FrameBuffer::ATT_Color0,
						GL_TEXTURE_RECTANGLE_ARB, gl_tex_2d_, 0);
			}
		}
		else
		{
			re.BindFramebuffer(gl_fbo_);

			if (0 == copy_to_tex_)
			{
				glFramebufferTexture3D(GL_FRAMEBUFFER,
					GL_COLOR_ATTACHMENT0, GL_TEXTURE_3D, gl_tex_, level_, slice_);

				GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
				if (status != GL_FRAMEBUFFER_COMPLETE)
				{
					glGenTextures(1, &gl_tex_2d_);
					glBindTexture(GL_TEXTURE_RECTANGLE_ARB, gl_tex_2d_);
					re.BindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
					glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, GL_RGBA32F_ARB, width_, height_,
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
				glFramebufferTexture3D(GL_FRAMEBUFFER,
						GL_COLOR_ATTACHMENT0 + att - FrameBuffer::ATT_Color0,
						GL_TEXTURE_3D, gl_tex_, level_, slice_);
			}
			else
			{
				BOOST_ASSERT(2 == copy_to_tex_);

				glFramebufferTexture2D(GL_FRAMEBUFFER,
						GL_COLOR_ATTACHMENT0 + att - FrameBuffer::ATT_Color0,
						GL_TEXTURE_RECTANGLE_ARB, gl_tex_2d_, 0);
			}

			re.BindFramebuffer(0);
		}
	}

	void OGLTexture3DRenderView::OnDetached(FrameBuffer& fb, uint32_t att)
	{
		KFL_UNUSED(fb);

		BOOST_ASSERT(att != FrameBuffer::ATT_DepthStencil);

		if (glloader_GL_EXT_direct_state_access())
		{
			BOOST_ASSERT(copy_to_tex_ != 0);
			if (1 == copy_to_tex_)
			{
				glNamedFramebufferTexture3DEXT(gl_fbo_,
						GL_COLOR_ATTACHMENT0 + att - FrameBuffer::ATT_Color0,
						GL_TEXTURE_3D, 0, 0, 0);
			}
			else
			{
				BOOST_ASSERT(2 == copy_to_tex_);

				glNamedFramebufferTexture2DEXT(gl_fbo_,
						GL_COLOR_ATTACHMENT0 + att - FrameBuffer::ATT_Color0,
						GL_TEXTURE_RECTANGLE_ARB, 0, 0);

				this->CopyToSlice(att);
			}
		}
		else
		{
			OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			re.BindFramebuffer(gl_fbo_);

			BOOST_ASSERT(copy_to_tex_ != 0);
			if (1 == copy_to_tex_)
			{
				glFramebufferTexture3D(GL_FRAMEBUFFER,
						GL_COLOR_ATTACHMENT0 + att - FrameBuffer::ATT_Color0,
						GL_TEXTURE_3D, 0, 0, 0);
			}
			else
			{
				BOOST_ASSERT(2 == copy_to_tex_);

				glFramebufferTexture2D(GL_FRAMEBUFFER,
						GL_COLOR_ATTACHMENT0 + att - FrameBuffer::ATT_Color0,
						GL_TEXTURE_RECTANGLE_ARB, 0, 0);

				this->CopyToSlice(att);
			}

			re.BindFramebuffer(0);
		}
	}

	void OGLTexture3DRenderView::OnUnbind(FrameBuffer& /*fb*/, uint32_t att)
	{
		BOOST_ASSERT(copy_to_tex_ != 0);
		if (2 == copy_to_tex_)
		{
			this->CopyToSlice(att);
		}
	}

	void OGLTexture3DRenderView::CopyToSlice(uint32_t att)
	{
		BOOST_ASSERT(att != FrameBuffer::ATT_DepthStencil);

		glReadBuffer(GL_COLOR_ATTACHMENT0 + att - FrameBuffer::ATT_Color0);

		OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		re.BindTexture(0, GL_TEXTURE_3D, gl_tex_);
		glCopyTexSubImage3D(GL_TEXTURE_3D, level_, 0, 0, slice_, 0, 0, width_, height_);
	}


	OGLTextureCubeRenderView::OGLTextureCubeRenderView(TexturePtr const & texture_cube, ElementFormat pf, int array_index,
		Texture::CubeFaces face, int level)
		: face_(face), level_(level)
	{
		KFL_UNUSED(array_index);

		BOOST_ASSERT(Texture::TT_Cube == texture_cube->Type());
		BOOST_ASSERT(0 == array_index);

		tex_ = texture_cube;
		gl_tex_ = checked_cast<OGLTexture*>(tex_.get())->GLTexture();

		width_ = texture_cube->Width(level);
		height_ = texture_cube->Height(level);
		pf_ = pf == EF_Unknown ? texture_cube->Format() : pf;
		sample_count_ = texture_cube->SampleCount();
		sample_quality_ = texture_cube->SampleQuality();
	}

	OGLTextureCubeRenderView::OGLTextureCubeRenderView(TexturePtr const & texture_cube, ElementFormat pf, int array_index, int level)
		: face_(static_cast<Texture::CubeFaces>(-1)),
			level_(level)
	{
		KFL_UNUSED(array_index);

		BOOST_ASSERT(Texture::TT_Cube == texture_cube->Type());
		BOOST_ASSERT(0 == array_index);

		tex_ = texture_cube;
		gl_tex_ = checked_cast<OGLTexture*>(tex_.get())->GLTexture();

		width_ = texture_cube->Width(level);
		height_ = texture_cube->Height(level);
		pf_ = pf == EF_Unknown ? texture_cube->Format() : pf;
		sample_count_ = texture_cube->SampleCount();
		sample_quality_ = texture_cube->SampleQuality();
	}

	void OGLTextureCubeRenderView::ClearColor(Color const & clr)
	{
		if (gl_fbo_ != 0)
		{
			this->DoClear(GL_COLOR_BUFFER_BIT, clr, 0, 0);
		}
		else
		{
			OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			re.BindTexture(0, GL_TEXTURE_CUBE_MAP, gl_tex_);

			std::vector<Color> mem_clr(width_ * height_, clr);
			glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face_ - Texture::CF_Positive_X,
				level_, 0, 0, width_, height_, GL_RGBA, GL_FLOAT, &mem_clr[0]);
		}
	}

	void OGLTextureCubeRenderView::Discard()
	{
		this->DoDiscardColor();
	}

	void OGLTextureCubeRenderView::OnAttached(FrameBuffer& fb, uint32_t att)
	{
		BOOST_ASSERT(att != FrameBuffer::ATT_DepthStencil);

		index_ = att - FrameBuffer::ATT_Color0;

		gl_fbo_ = checked_cast<OGLFrameBuffer*>(&fb)->OGLFbo();
		if (face_ >= 0)
		{
			GLenum face = GL_TEXTURE_CUBE_MAP_POSITIVE_X + face_ - Texture::CF_Positive_X;
			if (glloader_GL_VERSION_4_5() || glloader_GL_ARB_direct_state_access())
			{
				glNamedFramebufferTextureLayer(gl_fbo_,
						GL_COLOR_ATTACHMENT0 + att - FrameBuffer::ATT_Color0,
					gl_tex_, level_, face_);
			}
			else if (glloader_GL_EXT_direct_state_access())
			{
				glNamedFramebufferTextureFaceEXT(gl_fbo_,
						GL_COLOR_ATTACHMENT0 + att - FrameBuffer::ATT_Color0,
					gl_tex_, level_, face);
			}
			else
			{
				OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
				re.BindFramebuffer(gl_fbo_);

				glFramebufferTexture2D(GL_FRAMEBUFFER,
						GL_COLOR_ATTACHMENT0 + att - FrameBuffer::ATT_Color0,
						face, gl_tex_, level_);

				re.BindFramebuffer(0);
			}
		}
		else
		{
			if (glloader_GL_VERSION_4_5() || glloader_GL_ARB_direct_state_access())
			{
				glNamedFramebufferTexture(gl_fbo_,
					GL_COLOR_ATTACHMENT0 + att - FrameBuffer::ATT_Color0,
					gl_tex_, level_);
			}
			else if (glloader_GL_EXT_direct_state_access())
			{
				glNamedFramebufferTextureEXT(gl_fbo_,
					GL_COLOR_ATTACHMENT0 + att - FrameBuffer::ATT_Color0,
					gl_tex_, level_);
			}
			else
			{
				OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
				re.BindFramebuffer(gl_fbo_);

				glFramebufferTexture(GL_FRAMEBUFFER,
					GL_COLOR_ATTACHMENT0 + att - FrameBuffer::ATT_Color0,
					gl_tex_, level_);

				re.BindFramebuffer(0);
			}
		}
	}

	void OGLTextureCubeRenderView::OnDetached(FrameBuffer& fb, uint32_t att)
	{
		KFL_UNUSED(fb);

		BOOST_ASSERT(att != FrameBuffer::ATT_DepthStencil);

		if (face_ >= 0)
		{
			if (glloader_GL_VERSION_4_5() || glloader_GL_ARB_direct_state_access())
			{
				glNamedFramebufferTextureLayer(gl_fbo_,
						GL_COLOR_ATTACHMENT0 + att - FrameBuffer::ATT_Color0,
						0, 0, 0);
			}
			else if (glloader_GL_EXT_direct_state_access())
			{
				glNamedFramebufferTextureFaceEXT(gl_fbo_,
						GL_COLOR_ATTACHMENT0 + att - FrameBuffer::ATT_Color0,
						0, 0, 0);
			}
			else
			{
				OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
				re.BindFramebuffer(gl_fbo_);

				glFramebufferTexture2D(GL_FRAMEBUFFER,
						GL_COLOR_ATTACHMENT0 + att - FrameBuffer::ATT_Color0,
						0, 0, 0);

				re.BindFramebuffer(0);
			}
		}
		else
		{
			if (glloader_GL_VERSION_4_5() || glloader_GL_ARB_direct_state_access())
			{
				glNamedFramebufferTexture(gl_fbo_,
					GL_COLOR_ATTACHMENT0 + att - FrameBuffer::ATT_Color0,
					0, 0);
			}
			else if (glloader_GL_EXT_direct_state_access())
			{
				glNamedFramebufferTextureEXT(gl_fbo_,
					GL_COLOR_ATTACHMENT0 + att - FrameBuffer::ATT_Color0,
					0, 0);
			}
			else
			{
				OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
				re.BindFramebuffer(gl_fbo_);

				glFramebufferTexture(GL_FRAMEBUFFER,
					GL_COLOR_ATTACHMENT0 + att - FrameBuffer::ATT_Color0,
					0, 0);

				re.BindFramebuffer(0);
			}
		}
	}


	OGLGraphicsBufferRenderView::OGLGraphicsBufferRenderView(GraphicsBufferPtr const & gb, ElementFormat pf, uint32_t first_elem,
		uint32_t num_elems)
	{
		buff_ = gb;
		width_ = num_elems;
		height_ = 1;
		pf_ = pf;
		sample_count_ = 1;
		sample_quality_ = 0;
		first_elem_ = first_elem;
		num_elems_ = num_elems;

		glGenTextures(1, &gl_tex_);
		glBindTexture(GL_TEXTURE_RECTANGLE_ARB, gl_tex_);
		OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		re.BindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
		glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, GL_RGBA32F_ARB, width_, height_,
			0, GL_RGBA, GL_FLOAT, nullptr);
	}

	OGLGraphicsBufferRenderView::~OGLGraphicsBufferRenderView()
	{
		if (Context::Instance().RenderFactoryValid())
		{
			auto& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			re.DeleteTextures(1, &gl_tex_);
		}
		else
		{
			glDeleteTextures(1, &gl_tex_);
		}
	}

	void OGLGraphicsBufferRenderView::ClearColor(Color const & clr)
	{
		if (gl_fbo_ != 0)
		{
			this->DoClear(GL_COLOR_BUFFER_BIT, clr, 0, 0);
		}
		else
		{
			OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			re.BindTexture(0, GL_TEXTURE_RECTANGLE_ARB, gl_tex_);

			std::vector<Color> mem_clr(width_ * height_, clr);
			glTexSubImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, 0, 0, width_, height_, GL_RGBA, GL_FLOAT, &mem_clr[0]);
		}
	}

	void OGLGraphicsBufferRenderView::Discard()
	{
		this->DoDiscardColor();
	}

	void OGLGraphicsBufferRenderView::OnAttached(FrameBuffer& fb, uint32_t att)
	{
		BOOST_ASSERT(att != FrameBuffer::ATT_DepthStencil);

		index_ = att - FrameBuffer::ATT_Color0;

		gl_fbo_ = checked_cast<OGLFrameBuffer*>(&fb)->OGLFbo();
		if (glloader_GL_EXT_direct_state_access())
		{
			glNamedFramebufferTexture2DEXT(gl_fbo_,
					GL_COLOR_ATTACHMENT0 + att - FrameBuffer::ATT_Color0,
					GL_TEXTURE_RECTANGLE_ARB, gl_tex_, 0);
		}
		else
		{
			OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			re.BindFramebuffer(gl_fbo_);

			glFramebufferTexture2D(GL_FRAMEBUFFER,
					GL_COLOR_ATTACHMENT0 + att - FrameBuffer::ATT_Color0,
					GL_TEXTURE_RECTANGLE_ARB, gl_tex_, 0);

			re.BindFramebuffer(0);
		}
	}

	void OGLGraphicsBufferRenderView::OnDetached(FrameBuffer& fb, uint32_t att)
	{
		KFL_UNUSED(fb);

		if (glloader_GL_EXT_direct_state_access())
		{
			this->CopyToGB(att);

			glNamedFramebufferTexture2DEXT(gl_fbo_,
					GL_COLOR_ATTACHMENT0 + att - FrameBuffer::ATT_Color0,
					GL_TEXTURE_RECTANGLE_ARB, 0, 0);
		}
		else
		{
			OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			re.BindFramebuffer(gl_fbo_);

			this->CopyToGB(att);

			glFramebufferTexture2D(GL_FRAMEBUFFER,
					GL_COLOR_ATTACHMENT0 + att - FrameBuffer::ATT_Color0,
					GL_TEXTURE_RECTANGLE_ARB, 0, 0);

			re.BindFramebuffer(0);
		}
	}

	void OGLGraphicsBufferRenderView::OnUnbind(FrameBuffer& fb, uint32_t att)
	{
		KFL_UNUSED(fb);

		OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		re.BindFramebuffer(gl_fbo_);

		this->CopyToGB(att);

		re.BindFramebuffer(0);
	}

	void OGLGraphicsBufferRenderView::CopyToGB(uint32_t att)
	{
		GLint internalFormat;
		GLenum glformat;
		GLenum gltype;
		OGLMapping::MappingFormat(internalFormat, glformat, gltype, pf_);

		glReadBuffer(GL_COLOR_ATTACHMENT0 + att - FrameBuffer::ATT_Color0);

		OGLGraphicsBuffer* ogl_gb = checked_cast<OGLGraphicsBuffer*>(buff_.get());
		OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		re.BindBuffer(GL_PIXEL_PACK_BUFFER, ogl_gb->GLvbo());
		glReadPixels(0, 0, width_, height_, glformat, gltype, nullptr);
	}


	OGLDepthStencilRenderView::OGLDepthStencilRenderView(uint32_t width, uint32_t height,
									ElementFormat pf, uint32_t sample_count, uint32_t sample_quality)
		: target_type_(0), array_index_(0), level_(-1),
			sample_count_(sample_count), sample_quality_(sample_quality)
	{
		BOOST_ASSERT(IsDepthFormat(pf));
		KFL_UNUSED(sample_count_);
		KFL_UNUSED(sample_quality_);

		width_ = width;
		height_ = height;
		pf_ = pf;
		sample_count_ = sample_count;
		sample_quality_ = sample_quality;

		GLint internalFormat;
		GLenum glformat;
		GLenum gltype;
		OGLMapping::MappingFormat(internalFormat, glformat, gltype, pf_);

		if (glloader_GL_VERSION_4_5() || glloader_GL_ARB_direct_state_access())
		{
			glCreateRenderbuffers(1, &gl_rbo_);
			if (sample_count <= 1)
			{
				glNamedRenderbufferStorage(gl_rbo_, internalFormat, width_, height_);
			}
			else
			{
				glNamedRenderbufferStorageMultisample(gl_rbo_, sample_count, internalFormat, width_, height_);
			}
		}
		else
		{
			glGenRenderbuffers(1, &gl_rbo_);
			glBindRenderbuffer(GL_RENDERBUFFER, gl_rbo_);
			if (sample_count <= 1)
			{
				glRenderbufferStorage(GL_RENDERBUFFER,
					internalFormat, width_, height_);
			}
			else
			{
				glRenderbufferStorageMultisample(GL_RENDERBUFFER, sample_count,
					internalFormat, width_, height_);
			}
		}
	}

	OGLDepthStencilRenderView::OGLDepthStencilRenderView(TexturePtr const & texture, ElementFormat pf, int array_index, int array_size,
		int level)
		: target_type_(checked_cast<OGLTexture*>(texture.get())->GLType()),
			array_index_(array_index), array_size_(array_size), level_(level)
	{
		BOOST_ASSERT((Texture::TT_2D == texture->Type()) || (Texture::TT_Cube == texture->Type()));
		BOOST_ASSERT((1 == array_size) || ((0 == array_index) && (static_cast<uint32_t>(array_size) == texture->ArraySize())));
		BOOST_ASSERT(IsDepthFormat(texture->Format()));

		tex_ = texture;
		gl_tex_ = checked_cast<OGLTexture*>(tex_.get())->GLTexture();

		width_ = texture->Width(level);
		height_ = texture->Height(level);
		pf_ = pf == EF_Unknown ? texture->Format() : pf;
		sample_count_ = texture->SampleCount();
		sample_quality_ = texture->SampleQuality();
	}

	OGLDepthStencilRenderView::~OGLDepthStencilRenderView()
	{
		glDeleteRenderbuffers(1, &gl_rbo_);
	}

	void OGLDepthStencilRenderView::ClearColor(Color const & /*clr*/)
	{
		KFL_UNREACHABLE("Can't be called");
	}

	void OGLDepthStencilRenderView::Discard()
	{
		this->DoDiscardDepthStencil();
	}

	void OGLDepthStencilRenderView::OnAttached(FrameBuffer& fb, uint32_t att)
	{
		KFL_UNUSED(att);

		BOOST_ASSERT(FrameBuffer::ATT_DepthStencil == att);

		index_ = 0;

		gl_fbo_ = checked_cast<OGLFrameBuffer*>(&fb)->OGLFbo();
		if (level_ < 0)
		{
			if (glloader_GL_VERSION_4_5() || glloader_GL_ARB_direct_state_access())
			{
				glNamedFramebufferRenderbuffer(gl_fbo_,
										GL_DEPTH_ATTACHMENT,
										GL_RENDERBUFFER, gl_rbo_);
				if (IsStencilFormat(pf_))
				{
					glNamedFramebufferRenderbuffer(gl_fbo_,
										GL_STENCIL_ATTACHMENT,
										GL_RENDERBUFFER, gl_rbo_);
				}
			}
			else if (glloader_GL_EXT_direct_state_access())
			{
				glNamedFramebufferRenderbufferEXT(gl_fbo_,
										GL_DEPTH_ATTACHMENT,
										GL_RENDERBUFFER, gl_rbo_);
				if (IsStencilFormat(pf_))
				{
					glNamedFramebufferRenderbufferEXT(gl_fbo_,
										GL_STENCIL_ATTACHMENT,
										GL_RENDERBUFFER, gl_rbo_);
				}
			}
			else
			{
				OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
				re.BindFramebuffer(gl_fbo_);

				glFramebufferRenderbuffer(GL_FRAMEBUFFER,
										GL_DEPTH_ATTACHMENT,
										GL_RENDERBUFFER, gl_rbo_);
				if (IsStencilFormat(pf_))
				{
					glFramebufferRenderbuffer(GL_FRAMEBUFFER,
										GL_STENCIL_ATTACHMENT,
										GL_RENDERBUFFER, gl_rbo_);
				}

				re.BindFramebuffer(0);
			}
		}
		else
		{
			OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			if (GL_TEXTURE_2D == target_type_)
			{
				if (glloader_GL_VERSION_4_5() || glloader_GL_ARB_direct_state_access())
				{
					if (IsDepthFormat(pf_))
					{
						glNamedFramebufferTexture(gl_fbo_,
							GL_DEPTH_ATTACHMENT, gl_tex_, level_);
					}
					if (IsStencilFormat(pf_))
					{
						glNamedFramebufferTexture(gl_fbo_,
							GL_STENCIL_ATTACHMENT, gl_tex_, level_);
					}
				}
				else if (glloader_GL_EXT_direct_state_access())
				{
					if (IsDepthFormat(pf_))
					{
						glNamedFramebufferTexture2DEXT(gl_fbo_,
							GL_DEPTH_ATTACHMENT, target_type_, gl_tex_, level_);
					}
					if (IsStencilFormat(pf_))
					{
						glNamedFramebufferTexture2DEXT(gl_fbo_,
							GL_STENCIL_ATTACHMENT, target_type_, gl_tex_, level_);
					}
				}
				else
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
			}
			else if (GL_TEXTURE_CUBE_MAP == target_type_)
			{
				if (glloader_GL_VERSION_4_5() || glloader_GL_ARB_direct_state_access())
				{
					if (IsDepthFormat(pf_))
					{
						glNamedFramebufferTexture(gl_fbo_, GL_DEPTH_ATTACHMENT, gl_tex_, level_);
					}
					if (IsStencilFormat(pf_))
					{
						glNamedFramebufferTexture(gl_fbo_, GL_STENCIL_ATTACHMENT, gl_tex_, level_);
					}
				}
				else if (glloader_GL_EXT_direct_state_access())
				{
					if (IsDepthFormat(pf_))
					{
						glNamedFramebufferTextureEXT(gl_fbo_, GL_DEPTH_ATTACHMENT, gl_tex_, level_);
					}
					if (IsStencilFormat(pf_))
					{
						glNamedFramebufferTextureEXT(gl_fbo_, GL_STENCIL_ATTACHMENT, gl_tex_, level_);
					}
				}
				else
				{
					re.BindFramebuffer(gl_fbo_);

					if (IsDepthFormat(pf_))
					{
						glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, gl_tex_, level_);
					}
					if (IsStencilFormat(pf_))
					{
						glFramebufferTexture(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, gl_tex_, level_);
					}

					re.BindFramebuffer(0);
				}
			}
			else
			{
				if (array_size_ > 1)
				{
					if (glloader_GL_VERSION_4_5() || glloader_GL_ARB_direct_state_access())
					{
						if (IsDepthFormat(pf_))
						{
							glNamedFramebufferTexture(gl_fbo_, GL_DEPTH_ATTACHMENT, gl_tex_, level_);
						}
						if (IsStencilFormat(pf_))
						{
							glNamedFramebufferTexture(gl_fbo_, GL_STENCIL_ATTACHMENT, gl_tex_, level_);
						}
					}
					else if (glloader_GL_EXT_direct_state_access())
					{
						if (IsDepthFormat(pf_))
						{
							glNamedFramebufferTextureEXT(gl_fbo_, GL_DEPTH_ATTACHMENT, gl_tex_, level_);
						}
						if (IsStencilFormat(pf_))
						{
							glNamedFramebufferTextureEXT(gl_fbo_, GL_STENCIL_ATTACHMENT, gl_tex_, level_);
						}
					}
					else
					{
						re.BindFramebuffer(gl_fbo_);

						if (IsDepthFormat(pf_))
						{
							glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, gl_tex_, level_);
						}
						if (IsStencilFormat(pf_))
						{
							glFramebufferTexture(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, gl_tex_, level_);
						}

						re.BindFramebuffer(0);
					}
				}
				else
				{
					if (glloader_GL_VERSION_4_5() || glloader_GL_ARB_direct_state_access())
					{
						if (IsDepthFormat(pf_))
						{
							glNamedFramebufferTextureLayer(gl_fbo_, GL_DEPTH_ATTACHMENT, gl_tex_, level_, array_index_);
						}
						if (IsStencilFormat(pf_))
						{
							glNamedFramebufferTextureLayer(gl_fbo_, GL_STENCIL_ATTACHMENT, gl_tex_, level_, array_index_);
						}
					}
					else if (glloader_GL_EXT_direct_state_access())
					{
						if (IsDepthFormat(pf_))
						{
							glNamedFramebufferTextureLayerEXT(gl_fbo_, GL_DEPTH_ATTACHMENT, gl_tex_, level_, array_index_);
						}
						if (IsStencilFormat(pf_))
						{
							glNamedFramebufferTextureLayerEXT(gl_fbo_, GL_STENCIL_ATTACHMENT, gl_tex_, level_, array_index_);
						}
					}
					else
					{
						re.BindFramebuffer(gl_fbo_);

						if (IsDepthFormat(pf_))
						{
							glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, gl_tex_, level_, array_index_);
						}
						if (IsStencilFormat(pf_))
						{
							glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, gl_tex_, level_, array_index_);
						}

						re.BindFramebuffer(0);
					}
				}
			}
		}
	}

	void OGLDepthStencilRenderView::OnDetached(FrameBuffer& fb, uint32_t att)
	{
		KFL_UNUSED(fb);
		KFL_UNUSED(att);

		BOOST_ASSERT(FrameBuffer::ATT_DepthStencil == att);

		if (level_ < 0)
		{
			if (glloader_GL_VERSION_4_5() || glloader_GL_ARB_direct_state_access())
			{
				glNamedFramebufferRenderbuffer(gl_fbo_,
										GL_DEPTH_ATTACHMENT,
										GL_RENDERBUFFER, 0);
				glNamedFramebufferRenderbuffer(gl_fbo_,
										GL_STENCIL_ATTACHMENT,
										GL_RENDERBUFFER, 0);
			}
			else if (glloader_GL_EXT_direct_state_access())
			{
				glNamedFramebufferRenderbufferEXT(gl_fbo_,
										GL_DEPTH_ATTACHMENT,
										GL_RENDERBUFFER, 0);
				glNamedFramebufferRenderbufferEXT(gl_fbo_,
										GL_STENCIL_ATTACHMENT,
										GL_RENDERBUFFER, 0);
			}
			else
			{
				OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
				re.BindFramebuffer(gl_fbo_);

				glFramebufferRenderbuffer(GL_FRAMEBUFFER,
										GL_DEPTH_ATTACHMENT,
										GL_RENDERBUFFER, 0);
				glFramebufferRenderbuffer(GL_FRAMEBUFFER,
										GL_STENCIL_ATTACHMENT,
										GL_RENDERBUFFER, 0);

				re.BindFramebuffer(0);
			}
		}
		else
		{
			OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			if (GL_TEXTURE_2D == target_type_)
			{
				if (glloader_GL_VERSION_4_5() || glloader_GL_ARB_direct_state_access())
				{
					if (IsDepthFormat(pf_))
					{
						glNamedFramebufferTexture(gl_fbo_,
							GL_DEPTH_ATTACHMENT, 0, 0);
					}
					if (IsStencilFormat(pf_))
					{
						glNamedFramebufferTexture(gl_fbo_,
							GL_STENCIL_ATTACHMENT, 0, 0);
					}
				}
				else if (glloader_GL_EXT_direct_state_access())
				{
					if (IsDepthFormat(pf_))
					{
						glNamedFramebufferTexture2DEXT(gl_fbo_,
							GL_DEPTH_ATTACHMENT, target_type_, 0, 0);
					}
					if (IsStencilFormat(pf_))
					{
						glNamedFramebufferTexture2DEXT(gl_fbo_,
							GL_STENCIL_ATTACHMENT, target_type_, 0, 0);
					}
				}
				else
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
			}
			else if (GL_TEXTURE_CUBE_MAP == target_type_)
			{
				if (glloader_GL_VERSION_4_5() || glloader_GL_ARB_direct_state_access())
				{
					if (IsDepthFormat(pf_))
					{
						glNamedFramebufferTexture(gl_fbo_, GL_DEPTH_ATTACHMENT, 0, 0);
					}
					if (IsStencilFormat(pf_))
					{
						glNamedFramebufferTexture(gl_fbo_, GL_STENCIL_ATTACHMENT, 0, 0);
					}
				}
				else if (glloader_GL_EXT_direct_state_access())
				{
					if (IsDepthFormat(pf_))
					{
						glNamedFramebufferTextureEXT(gl_fbo_, GL_DEPTH_ATTACHMENT, 0, 0);
					}
					if (IsStencilFormat(pf_))
					{
						glNamedFramebufferTextureEXT(gl_fbo_, GL_STENCIL_ATTACHMENT, 0, 0);
					}
				}
				else
				{
					re.BindFramebuffer(gl_fbo_);

					if (IsDepthFormat(pf_))
					{
						glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, 0, 0);
					}
					if (IsStencilFormat(pf_))
					{
						glFramebufferTexture(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, 0, 0);
					}

					re.BindFramebuffer(0);
				}
			}
			else
			{
				if (array_size_ > 1)
				{
					if (glloader_GL_VERSION_4_5() || glloader_GL_ARB_direct_state_access())
					{
						if (IsDepthFormat(pf_))
						{
							glNamedFramebufferTexture(gl_fbo_, GL_DEPTH_ATTACHMENT, 0, 0);
						}
						if (IsStencilFormat(pf_))
						{
							glNamedFramebufferTexture(gl_fbo_, GL_STENCIL_ATTACHMENT, 0, 0);
						}
					}
					else if (glloader_GL_EXT_direct_state_access())
					{
						if (IsDepthFormat(pf_))
						{
							glNamedFramebufferTextureEXT(gl_fbo_, GL_DEPTH_ATTACHMENT, 0, 0);
						}
						if (IsStencilFormat(pf_))
						{
							glNamedFramebufferTextureEXT(gl_fbo_, GL_STENCIL_ATTACHMENT, 0, 0);
						}
					}
					else
					{
						re.BindFramebuffer(gl_fbo_);

						if (IsDepthFormat(pf_))
						{
							glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, 0, 0, 0);
						}
						if (IsStencilFormat(pf_))
						{
							glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, 0, 0, 0);
						}

						re.BindFramebuffer(0);
					}
				}
				else
				{
					if (glloader_GL_VERSION_4_5() || glloader_GL_ARB_direct_state_access())
					{
						if (IsDepthFormat(pf_))
						{
							glNamedFramebufferTextureLayer(gl_fbo_, GL_DEPTH_ATTACHMENT, 0, 0, 0);
						}
						if (IsStencilFormat(pf_))
						{
							glNamedFramebufferTextureLayer(gl_fbo_, GL_STENCIL_ATTACHMENT, 0, 0, 0);
						}
					}
					else if (glloader_GL_EXT_direct_state_access())
					{
						if (IsDepthFormat(pf_))
						{
							glNamedFramebufferTextureLayerEXT(gl_fbo_, GL_DEPTH_ATTACHMENT, 0, 0, 0);
						}
						if (IsStencilFormat(pf_))
						{
							glNamedFramebufferTextureLayerEXT(gl_fbo_, GL_STENCIL_ATTACHMENT, 0, 0, 0);
						}
					}
					else
					{
						re.BindFramebuffer(gl_fbo_);

						if (IsDepthFormat(pf_))
						{
							glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, 0, 0, 0);
						}
						if (IsStencilFormat(pf_))
						{
							glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, 0, 0, 0);
						}

						re.BindFramebuffer(0);
					}
				}
			}
		}
	}


	OGLTextureCubeDepthStencilRenderView::OGLTextureCubeDepthStencilRenderView(TexturePtr const & texture_cube, ElementFormat pf,
		int array_index, Texture::CubeFaces face, int level)
		: face_(face), level_(level)
	{
		BOOST_ASSERT(Texture::TT_Cube == texture_cube->Type());
		BOOST_ASSERT(IsDepthFormat(texture_cube->Format()));
		BOOST_ASSERT(0 == array_index);
		KFL_UNUSED(array_index);

		tex_ = texture_cube;
		gl_tex_ = checked_cast<OGLTextureCube*>(tex_.get())->GLTexture();

		width_ = texture_cube->Width(level);
		height_ = texture_cube->Height(level);
		pf_ = pf == EF_Unknown ? texture_cube->Format() : pf;
		sample_count_ = texture_cube->SampleCount();
		sample_quality_ = texture_cube->SampleQuality();
	}

	void OGLTextureCubeDepthStencilRenderView::ClearColor(Color const & /*clr*/)
	{
		KFL_UNREACHABLE("Can't be called");
	}

	void OGLTextureCubeDepthStencilRenderView::Discard()
	{
		this->DoDiscardDepthStencil();
	}

	void OGLTextureCubeDepthStencilRenderView::OnAttached(FrameBuffer& fb, uint32_t att)
	{
		KFL_UNUSED(att);

		BOOST_ASSERT(FrameBuffer::ATT_DepthStencil == att);

		index_ = 0;

		gl_fbo_ = checked_cast<OGLFrameBuffer*>(&fb)->OGLFbo();
		GLenum face = GL_TEXTURE_CUBE_MAP_POSITIVE_X + face_ - Texture::CF_Positive_X;
		OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		if (glloader_GL_VERSION_4_5() || glloader_GL_ARB_direct_state_access())
		{
			if (IsDepthFormat(pf_))
			{
				glNamedFramebufferTextureLayer(gl_fbo_, GL_DEPTH_ATTACHMENT, gl_tex_, level_, face_);
			}
			if (IsStencilFormat(pf_))
			{
				glNamedFramebufferTextureLayer(gl_fbo_, GL_STENCIL_ATTACHMENT, gl_tex_, level_, face_);
			}
		}
		else if (glloader_GL_EXT_direct_state_access())
		{
			if (IsDepthFormat(pf_))
			{
				glNamedFramebufferTextureFaceEXT(gl_fbo_, GL_DEPTH_ATTACHMENT, gl_tex_, level_, face);
			}
			if (IsStencilFormat(pf_))
			{
				glNamedFramebufferTextureFaceEXT(gl_fbo_, GL_STENCIL_ATTACHMENT, gl_tex_, level_, face);
			}
		}
		else
		{
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
	}

	void OGLTextureCubeDepthStencilRenderView::OnDetached(FrameBuffer& fb, uint32_t att)
	{
		KFL_UNUSED(fb);
		KFL_UNUSED(att);

		BOOST_ASSERT(FrameBuffer::ATT_DepthStencil == att);

		OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		if (glloader_GL_VERSION_4_5() || glloader_GL_ARB_direct_state_access())
		{
			if (IsDepthFormat(pf_))
			{
				glNamedFramebufferTextureLayer(gl_fbo_, GL_DEPTH_ATTACHMENT, 0, 0, 0);
			}
			if (IsStencilFormat(pf_))
			{
				glNamedFramebufferTextureLayer(gl_fbo_, GL_STENCIL_ATTACHMENT, 0, 0, 0);
			}
		}
		else if (glloader_GL_EXT_direct_state_access())
		{
			if (IsDepthFormat(pf_))
			{
				glNamedFramebufferTextureFaceEXT(gl_fbo_, GL_DEPTH_ATTACHMENT, 0, 0, GL_TEXTURE_CUBE_MAP_POSITIVE_X);
			}
			if (IsStencilFormat(pf_))
			{
				glNamedFramebufferTextureFaceEXT(gl_fbo_, GL_STENCIL_ATTACHMENT, 0, 0, GL_TEXTURE_CUBE_MAP_POSITIVE_X);
			}
		}
		else
		{
			re.BindFramebuffer(gl_fbo_);

			if (IsDepthFormat(pf_))
			{
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, 0, 0, 0);
			}
			if (IsStencilFormat(pf_))
			{
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, 0, 0, 0);
			}

			re.BindFramebuffer(0);
		}
	}
}
