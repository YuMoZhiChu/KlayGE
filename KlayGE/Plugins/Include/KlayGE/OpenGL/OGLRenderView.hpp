// OGLRenderView.hpp
// KlayGE OpenGL渲染视图类 头文件
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

#ifndef _OGLRENDERVIEW_HPP
#define _OGLRENDERVIEW_HPP

#pragma once

#include <KlayGE/RenderView.hpp>
#include <KlayGE/Texture.hpp>

#include <KlayGE/OpenGL/OGLTexture.hpp>

namespace KlayGE
{
	class OGLRenderView : public RenderView
	{
	public:
		OGLRenderView();
		virtual ~OGLRenderView();

		void ClearDepth(float depth);
		void ClearStencil(int32_t stencil);
		void ClearDepthStencil(float depth, int32_t stencil);

		GLuint GLTexture() const
		{
			return gl_tex_;
		}

	protected:
		void DoClear(uint32_t flags, Color const & clr, float depth, int32_t stencil);
		void DoDiscardColor();
		void DoDiscardDepthStencil();

	protected:
		GLuint gl_tex_;
		GLuint gl_fbo_;
		GLuint index_;
	};

	typedef std::shared_ptr<OGLRenderView> OGLRenderViewPtr;


	class OGLScreenColorRenderView : public OGLRenderView
	{
	public:
		OGLScreenColorRenderView(uint32_t width, uint32_t height, ElementFormat pf);

		void ClearColor(Color const & clr);
		void ClearDepth(float depth);
		void ClearStencil(int32_t stencil);
		void ClearDepthStencil(float depth, int32_t stencil);

		virtual void Discard() override;

		void OnAttached(FrameBuffer& fb, uint32_t att);
		void OnDetached(FrameBuffer& fb, uint32_t att);
	};

	typedef std::shared_ptr<OGLScreenColorRenderView> OGLScreenColorRenderViewPtr;


	class OGLScreenDepthStencilRenderView : public OGLRenderView
	{
	public:
		OGLScreenDepthStencilRenderView(uint32_t width, uint32_t height, ElementFormat pf);

		void ClearColor(Color const & clr);

		virtual void Discard() override;

		void OnAttached(FrameBuffer& fb, uint32_t att);
		void OnDetached(FrameBuffer& fb, uint32_t att);
	};

	typedef std::shared_ptr<OGLScreenDepthStencilRenderView> OGLScreenDepthStencilRenderViewPtr;


	class OGLTexture1DRenderView : public OGLRenderView
	{
	public:
		OGLTexture1DRenderView(TexturePtr const & texture_1d, ElementFormat pf, int array_index, int array_size, int level);

		void ClearColor(Color const & clr);

		virtual void Discard() override;

		void OnAttached(FrameBuffer& fb, uint32_t att);
		void OnDetached(FrameBuffer& fb, uint32_t att);

	private:
		int array_index_;
		int array_size_;
		int level_;
	};

	typedef std::shared_ptr<OGLTexture1DRenderView> OGLTexture1DRenderViewPtr;


	class OGLTexture2DRenderView : public OGLRenderView
	{
	public:
		OGLTexture2DRenderView(TexturePtr const & texture_2d, ElementFormat pf, int array_index, int array_size, int level);

		void ClearColor(Color const & clr);

		virtual void Discard() override;

		void OnAttached(FrameBuffer& fb, uint32_t att);
		void OnDetached(FrameBuffer& fb, uint32_t att);

	private:
		int array_index_;
		int array_size_;
		int level_;
	};

	typedef std::shared_ptr<OGLTexture2DRenderView> OGLTexture2DRenderViewPtr;


	class OGLTexture3DRenderView : public OGLRenderView
	{
	public:
		OGLTexture3DRenderView(TexturePtr const & texture_3d, ElementFormat pf, int array_index, uint32_t slice, int level);
		~OGLTexture3DRenderView();

		void ClearColor(Color const & clr);

		virtual void Discard() override;

		void OnAttached(FrameBuffer& fb, uint32_t att);
		void OnDetached(FrameBuffer& fb, uint32_t att);

		void OnUnbind(FrameBuffer& fb, uint32_t att);

	private:
		void CopyToSlice(uint32_t att);

	private:
		uint32_t slice_;
		int level_;
		int copy_to_tex_;
		GLuint gl_tex_2d_;
	};

	typedef std::shared_ptr<OGLTexture3DRenderView> OGLTexture3DRenderViewPtr;


	class OGLTextureCubeRenderView : public OGLRenderView
	{
	public:
		OGLTextureCubeRenderView(TexturePtr const & texture_cube, ElementFormat pf, int array_index, Texture::CubeFaces face, int level);
		OGLTextureCubeRenderView(TexturePtr const & texture_cube, ElementFormat pf, int array_index, int level);

		void ClearColor(Color const & clr);

		virtual void Discard() override;

		void OnAttached(FrameBuffer& fb, uint32_t att);
		void OnDetached(FrameBuffer& fb, uint32_t att);

	private:
		Texture::CubeFaces face_;
		int level_;
	};

	typedef std::shared_ptr<OGLTextureCubeRenderView> OGLTextureCubeRenderViewPtr;


	class OGLGraphicsBufferRenderView : public OGLRenderView
	{
	public:
		OGLGraphicsBufferRenderView(GraphicsBufferPtr const & gb, ElementFormat pf, uint32_t forst_elem, uint32_t num_elems);
		~OGLGraphicsBufferRenderView();

		void ClearColor(Color const & clr);

		virtual void Discard() override;

		void OnAttached(FrameBuffer& fb, uint32_t att);
		void OnDetached(FrameBuffer& fb, uint32_t att);

		void OnUnbind(FrameBuffer& fb, uint32_t att);

	private:
		void CopyToGB(uint32_t att);
	};

	typedef std::shared_ptr<OGLGraphicsBufferRenderView> OGLGraphicsBufferRenderViewPtr;


	class OGLDepthStencilRenderView : public OGLRenderView
	{
	public:
		OGLDepthStencilRenderView(uint32_t width, uint32_t height, ElementFormat pf, uint32_t sample_count, uint32_t sample_quality);
		OGLDepthStencilRenderView(TexturePtr const & texture, ElementFormat pf, int array_index, int array_size, int level);
		~OGLDepthStencilRenderView();

		void ClearColor(Color const & clr);

		virtual void Discard() override;

		void OnAttached(FrameBuffer& fb, uint32_t att);
		void OnDetached(FrameBuffer& fb, uint32_t att);

	private:
		GLenum target_type_;
		int array_index_;
		int array_size_;
		int level_;
		uint32_t sample_count_, sample_quality_;
		GLuint gl_rbo_;
	};

	typedef std::shared_ptr<OGLDepthStencilRenderView> OGLDepthStencilRenderViewPtr;

	class OGLTextureCubeDepthStencilRenderView : public OGLRenderView
	{
	public:
		OGLTextureCubeDepthStencilRenderView(TexturePtr const & texture_cube, ElementFormat pf, int array_index, Texture::CubeFaces face,
			int level);

		void ClearColor(Color const & clr);

		virtual void Discard() override;

		void OnAttached(FrameBuffer& fb, uint32_t att);
		void OnDetached(FrameBuffer& fb, uint32_t att);

	private:
		Texture::CubeFaces face_;
		int level_;
	};

	typedef std::shared_ptr<OGLTextureCubeDepthStencilRenderView> OGLTextureCubeDepthStencilRenderViewPtr;
}

#endif			// _OGLRENDERVIEW_HPP
