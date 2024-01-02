#pragma once
class SwapChain
{
public:
	ComPtr<IDXGISwapChain3> GetSwapChain() { return m_pdxgiSwapChain; }
	ComPtr<ID3D12Resource> GetRenderTarget(int index) { return  m_ppd3dRenderTargetBuffers[index]; }

	ComPtr<ID3D12Resource> GetBackRTVBuffer() { return  m_ppd3dRenderTargetBuffers[m_nSwapChainBuffers]; }
private:
	void CreateSwapChain();
	void CreateRTV();

private:
	ComPtr<IDXGISwapChain3> m_pdxgiSwapChain;

	static const UINT m_nSwapChainBuffers = 2;

	ComPtr<ID3D12Resource> m_ppd3dRenderTargetBuffers[m_nSwapChainBuffers];

	ComPtr<ID3D12DescriptorHeap> m_pd3dRtvDescriptorHeap;

	UINT m_nRtvDescriptorIncrementSize;

	UINT m_nSwapChainBufferIndex;

};

