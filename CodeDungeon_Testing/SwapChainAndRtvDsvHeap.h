#pragma once
class SwapChainAndRtvDsvHeap {
public:
	void InitSwapChainAndRtvDsvHeap(const ComPtr<IDXGIFactory4>& _Factory, const ComPtr<ID3D12Device>& _Device, const ComPtr< ID3D12CommandQueue>& _CmdQueue, bool MsaaEnable, uint16 MsaaQualityLevels);
	void ChangeSwapChainState(const ComPtr<ID3D12Device>& _Device);
	void CreateSwapChain(const ComPtr<IDXGIFactory4>& _Factory, const ComPtr<ID3D12Device>& _Device, const ComPtr< ID3D12CommandQueue>& _CmdQueue, bool MssaaEnable, uint16 MsaaQualityLevels); //스왑 체인 생성 함수
	void CreateRenderTargetViews(const ComPtr<ID3D12Device>& _Device); //렌더 타겟 뷰 생성 함수
	void CreateDepthStencilView(const ComPtr<ID3D12Device>& _Device, bool MsaaEnable, uint16 MsaaQualityLevels); //깊이-스텐실 뷰 생성 함수
	void CreateRtvAndDsvDescriptorHeaps(const ComPtr<ID3D12Device>& _Device); //서술자 힙 생성 함수
public:
	ComPtr<IDXGISwapChain3>GetSwapChain() { return m_cpdxgiSwapChain; }
	ComPtr<ID3D12Resource>GetRenderTarget(int index) { return m_ppd3dRenderTargetBuffers[index]; }
	ComPtr<ID3D12DescriptorHeap>GetRTVHeap() { return m_pd3dRtvDescriptorHeap; }
	ComPtr<ID3D12Resource>GetDepthStencilBuffer() { return m_pd3dDepthStencilBuffer; }
	ComPtr<ID3D12DescriptorHeap>GetDSVHeap() { return m_pd3dDsvDescriptorHeap; }
	uint16 GetRTVIncrementSize() { return m_nRtvDescriptorIncrementSize; }
	uint16 GetDSVIncrementSize() { return m_nDsvDescriptorIncrementSize; }

	uint16 GetSwapChainIndex() { return m_nSwapChainBufferIndex; }
	void SetSwapChainIndex(int16 index) { m_nSwapChainBufferIndex = index; }
public:
	HINSTANCE GetInstance() { return m_hInstance; }
	HWND GetHandle() { return m_hWnd; }
	int GetClientWidth() { return m_nWndClientWidth; }
	int GetClientHeight() { return m_nWndClientHeight; }
private:
	uint16 m_nSwapChainBufferIndex;
	ComPtr<IDXGISwapChain3> m_cpdxgiSwapChain;

	ComPtr<ID3D12Resource> m_ppd3dRenderTargetBuffers[SWAP_CHAIN_BUFFER_COUNT];
	ComPtr<ID3D12DescriptorHeap> m_pd3dRtvDescriptorHeap;
	uint16 m_nRtvDescriptorIncrementSize;

	ComPtr<ID3D12Resource> m_pd3dDepthStencilBuffer;
	ComPtr<ID3D12DescriptorHeap> m_pd3dDsvDescriptorHeap;
	uint16 m_nDsvDescriptorIncrementSize;
public:
	HINSTANCE m_hInstance;
	HWND m_hWnd;
	int m_nWndClientWidth; //swap chain 너비 등 저장목적
	int m_nWndClientHeight;
};
