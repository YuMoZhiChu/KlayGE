// OGLESRenderView.hpp
// KlayGE OpenGL ES渲染视图类 头文件
// Ver 3.10.0
// 版权所有(C) 龚敏敏, 2010
// Homepage: http://www.klayge.org
//
// 3.10.0
// 初次建立 (2010.1.22)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#ifndef _OGLESRENDERVIEW_HPP
#define _OGLESRENDERVIEW_HPP

#pragma once

#include <KlayGE/RenderView.hpp>
#include <KlayGE/Texture.hpp>

#include <KlayGE/OpenGLES/OGLESTexture.hpp>

namespace KlayGE
{
	class OGLESRenderView : public RenderView
	{
	public:
		OGLESRenderView();
		virtual ~OGLESRenderView();

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

	typedef std::shared_ptr<OGLESRenderView> OGLESRenderViewPtr;

	class OGLESScreenColorRenderView : public OGLESRenderView
	{
	public:
		OGLESScreenColorRenderView(uint32_t width, uint32_t height, ElementFormat pf);

		void ClearColor(Color const & clr);
		void ClearDepth(float depth);
		void ClearStencil(int32_t stencil);
		void ClearDepthStencil(float depth, int32_t stencil);

		virtual void Discard() override;

		void OnAttached(FrameBuffer& fb, uint32_t att);
		void OnDetached(FrameBuffer& fb, uint32_t att);
	};

	typedef std::shared_ptr<OGLESScreenColorRenderView> OGLES2ScreenColorRenderViewPtr;


	class OGLESScreenDepthStencilRenderView : public OGLESRenderView
	{
	public:
		OGLESScreenDepthStencilRenderView(uint32_t width, uint32_t height, ElementFormat pf);

		void ClearColor(Color const & clr);

		virtual void Discard() override;

		void OnAttached(FrameBuffer& fb, uint32_t att);
		void OnDetached(FrameBuffer& fb, uint32_t att);
	};

	typedef std::shared_ptr<OGLESScreenDepthStencilRenderView> OGLES2ScreenDepthStencilRenderViewPtr;


	class OGLESTexture1DRenderView : public OGLESRenderView
	{
	public:
		OGLESTexture1DRenderView(TexturePtr const & texture_1d, ElementFormat pf, int array_index, int array_size, int level);

		void ClearColor(Color const & clr);

		virtual void Discard() override;

		void OnAttached(FrameBuffer& fb, uint32_t att);
		void OnDetached(FrameBuffer& fb, uint32_t att);

	private:
		int array_index_;
		int array_size_;
		int level_;
	};

	typedef std::shared_ptr<OGLESTexture1DRenderView> OGLES2Texture1DRenderViewPtr;


	class OGLESTexture2DRenderView : public OGLESRenderView
	{
	public:
		OGLESTexture2DRenderView(TexturePtr const & texture_2d, ElementFormat pf, int array_index, int array_size, int level);

		void ClearColor(Color const & clr);

		virtual void Discard() override;

		void OnAttached(FrameBuffer& fb, uint32_t att);
		void OnDetached(FrameBuffer& fb, uint32_t att);

	private:
		int array_index_;
		int array_size_;
		int level_;
	};

	typedef std::shared_ptr<OGLESTexture2DRenderView> OGLES2Texture2DRenderViewPtr;


	class OGLESTexture3DRenderView : public OGLESRenderView
	{
	public:
		OGLESTexture3DRenderView(TexturePtr const & texture_3d, ElementFormat pf, int array_index, uint32_t slice, int level);
		~OGLESTexture3DRenderView();

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

	typedef std::shared_ptr<OGLESTexture3DRenderView> OGLES2Texture3DRenderViewPtr;


	class OGLESTextureCubeRenderView : public OGLESRenderView
	{
	public:
		OGLESTextureCubeRenderView(TexturePtr const & texture_cube, ElementFormat pf, int array_index, Texture::CubeFaces face, int level);
		OGLESTextureCubeRenderView(TexturePtr const & texture_cube, ElementFormat pf, int array_index, int level);

		void ClearColor(Color const & clr);

		virtual void Discard() override;

		void OnAttached(FrameBuffer& fb, uint32_t att);
		void OnDetached(FrameBuffer& fb, uint32_t att);

	private:
		Texture::CubeFaces face_;
		int level_;
	};

	typedef std::shared_ptr<OGLESTextureCubeRenderView> OGLES2TextureCubeRenderViewPtr;


	class OGLESDepthStencilRenderView : public OGLESRenderView
	{
	public:
		OGLESDepthStencilRenderView(uint32_t width, uint32_t height, ElementFormat pf, uint32_t sample_count, uint32_t sample_quality);
		OGLESDepthStencilRenderView(TexturePtr const & texture, ElementFormat pf, int array_index, int array_size, int level);
		~OGLESDepthStencilRenderView();

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
		GLuint gl_rbos_[2];
	};

	typedef std::shared_ptr<OGLESDepthStencilRenderView> OGLES2DepthStencilRenderViewPtr;


	class OGLESTextureCubeDepthStencilRenderView : public OGLESRenderView
	{
	public:
		OGLESTextureCubeDepthStencilRenderView(TexturePtr const & texture_cube, ElementFormat pf, int array_index, Texture::CubeFaces face,
			int level);

		void ClearColor(Color const & clr);

		virtual void Discard() override;

		void OnAttached(FrameBuffer& fb, uint32_t att);
		void OnDetached(FrameBuffer& fb, uint32_t att);

	private:
		Texture::CubeFaces face_;
		int level_;
	};

	typedef std::shared_ptr<OGLESTextureCubeDepthStencilRenderView> OGLES2TextureCubeDepthStencilRenderViewPtr;

#if defined(KLAYGE_PLATFORM_IOS)
	class OGLESEAGLRenderView : public OGLESRenderView
	{
	public:
		explicit OGLESEAGLRenderView(ElementFormat pf);

		void ClearColor(Color const & clr);
		void ClearDepth(float depth);

		virtual void Discard() override;

		void OnAttached(FrameBuffer& fb, uint32_t att);
		void OnDetached(FrameBuffer& fb, uint32_t att);

		void BindRenderBuffer();

	protected:
		GLuint gl_rf_;
	};

	typedef std::shared_ptr<OGLESEAGLRenderView> OGLESEAGLRenderViewPtr;
#endif
}

#endif			// _OGLESRENDERVIEW_HPP
