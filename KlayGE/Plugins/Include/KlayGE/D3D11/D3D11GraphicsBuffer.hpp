// D3D11GraphicsBuffer.hpp
// KlayGE D3D11ͼ�λ������� ͷ�ļ�
// Ver 3.8.0
// ��Ȩ����(C) ������, 2009
// Homepage: http://www.klayge.org
//
// 3.8.0
// ���ν��� (2009.1.30)
//
// �޸ļ�¼
/////////////////////////////////////////////////////////////////////////////////

#ifndef _D3D11GRAPHICSBUFFER_HPP
#define _D3D11GRAPHICSBUFFER_HPP

#pragma once

#include <KlayGE/ElementFormat.hpp>
#include <KlayGE/GraphicsBuffer.hpp>
#include <KlayGE/D3D11/D3D11Typedefs.hpp>

namespace KlayGE
{
	class D3D11GraphicsBuffer : public GraphicsBuffer
	{
	public:
		D3D11GraphicsBuffer(BufferUsage usage, uint32_t access_hint, uint32_t bind_flags,
			uint32_t size_in_byte, ElementFormat fmt);

		ID3D11Buffer* D3DBuffer() const
		{
			return d3d_buffer_.get();
		}

		ID3D11ShaderResourceViewPtr const & D3DShaderResourceView() const
		{
			return d3d_sr_view_;
		}
		ID3D11RenderTargetViewPtr CreateD3DRenderTargetView(ElementFormat pf, uint32_t first_elem, uint32_t num_elems) const;
		ID3D11UnorderedAccessViewPtr CreateD3DUnorderedAccessView(ElementFormat pf, uint32_t first_elem, uint32_t num_elems) const;

		void CopyToBuffer(GraphicsBuffer& target) override;
		void CopyToSubBuffer(GraphicsBuffer& target,
			uint32_t dst_offset, uint32_t src_offset, uint32_t size) override;

		void CreateHWResource(void const * init_data) override;
		void DeleteHWResource() override;
		bool HWResourceReady() const override;

		void UpdateSubresource(uint32_t offset, uint32_t size, void const * data) override;

	protected:
		void GetD3DFlags(D3D11_USAGE& usage, UINT& cpu_access_flags, UINT& bind_flags, UINT& misc_flags);

	private:
		void* Map(BufferAccess ba);
		void Unmap();

	private:
		ID3D11Device* d3d_device_;
		ID3D11DeviceContext* d3d_imm_ctx_;

		ID3D11BufferPtr d3d_buffer_;
		ID3D11ShaderResourceViewPtr d3d_sr_view_;

		uint32_t bind_flags_;
		ElementFormat fmt_as_shader_res_;
	};
}

#endif			// _D3D11GRAPHICSBUFFER_HPP
