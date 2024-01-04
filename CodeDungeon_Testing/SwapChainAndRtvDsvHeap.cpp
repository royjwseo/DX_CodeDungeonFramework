#include "stdafx.h"
#include "SwapChainAndRtvDsvHeap.h"

//---------------------------
void SwapChainAndRtvDsvHeap::InitSwapChainAndRtvDsvHeap(ComPtr<IDXGIFactory4>& _Factory, ComPtr<ID3D12Device>& _Device, ComPtr< ID3D12CommandQueue>& _CmdQueue, bool MsaaEnable, uint16 MsaaQualityLevels)
{
	m_nRtvDescriptorIncrementSize = 0;
	m_nDsvDescriptorIncrementSize = 0;
	m_nSwapChainBufferIndex = 0;

	m_nWndClientWidth = FRAME_BUFFER_WIDTH;
	m_nWndClientHeight = FRAME_BUFFER_HEIGHT;

	CreateRtvAndDsvDescriptorHeaps(_Device);
	CreateSwapChain(_Factory, _Device, _CmdQueue, MsaaEnable, MsaaQualityLevels);

	CreateDepthStencilView(_Device, MsaaEnable, MsaaQualityLevels);



}

void SwapChainAndRtvDsvHeap::ChangeSwapChainState(ComPtr<ID3D12Device>& _Device)
{

	BOOL bFullScreenState = FALSE;
	m_cpdxgiSwapChain->GetFullscreenState(&bFullScreenState, NULL);
	m_cpdxgiSwapChain->SetFullscreenState(!bFullScreenState, NULL);
	DXGI_MODE_DESC dxgiTargetParameters;
	dxgiTargetParameters.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	dxgiTargetParameters.Width = m_nWndClientWidth;
	dxgiTargetParameters.Height = m_nWndClientHeight;
	dxgiTargetParameters.RefreshRate.Numerator = 60;
	dxgiTargetParameters.RefreshRate.Denominator = 1;
	dxgiTargetParameters.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	dxgiTargetParameters.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	m_cpdxgiSwapChain->ResizeTarget(&dxgiTargetParameters);
	for (int i = 0; i < SWAP_CHAIN_BUFFER_COUNT; i++) if (m_ppd3dRenderTargetBuffers[i])
		m_ppd3dRenderTargetBuffers[i]->Release();
	DXGI_SWAP_CHAIN_DESC dxgiSwapChainDesc;
	m_cpdxgiSwapChain->GetDesc(&dxgiSwapChainDesc);
	m_cpdxgiSwapChain->ResizeBuffers(SWAP_CHAIN_BUFFER_COUNT, m_nWndClientWidth,
		m_nWndClientHeight, dxgiSwapChainDesc.BufferDesc.Format, dxgiSwapChainDesc.Flags);
	m_nSwapChainBufferIndex = m_cpdxgiSwapChain->GetCurrentBackBufferIndex();
	CreateRenderTargetViews(_Device);

}


void SwapChainAndRtvDsvHeap::CreateRenderTargetViews(ComPtr<ID3D12Device>& _Device)
{
	D3D12_CPU_DESCRIPTOR_HANDLE d3dRtvCPUDescriptorHandle =
		m_pd3dRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();

	for (UINT i = 0; i < SWAP_CHAIN_BUFFER_COUNT; i++) {
		DX::ThrowIfFailed(m_cpdxgiSwapChain->GetBuffer(i, IID_PPV_ARGS(&m_ppd3dRenderTargetBuffers[i])));
		_Device->CreateRenderTargetView(m_ppd3dRenderTargetBuffers[i].Get(), NULL, d3dRtvCPUDescriptorHandle);
		d3dRtvCPUDescriptorHandle.ptr += m_nRtvDescriptorIncrementSize;

	}

}

void SwapChainAndRtvDsvHeap::CreateDepthStencilView(ComPtr<ID3D12Device>& _Device, bool MssaaEnable, uint16 MsaaQualityLevels)
{
	D3D12_RESOURCE_DESC d3dResourceDesc;
	d3dResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	d3dResourceDesc.Alignment = 0;
	d3dResourceDesc.Width = m_nWndClientWidth;
	d3dResourceDesc.Height = m_nWndClientHeight;
	d3dResourceDesc.DepthOrArraySize = 1;
	d3dResourceDesc.MipLevels = 1;
	d3dResourceDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	d3dResourceDesc.SampleDesc.Count = (MssaaEnable) ? 4 : 1;
	d3dResourceDesc.SampleDesc.Quality = (MssaaEnable) ? (MsaaQualityLevels - 1) : 0;
	d3dResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	d3dResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_HEAP_PROPERTIES d3dHeapProperties;
	::ZeroMemory(&d3dHeapProperties, sizeof(D3D12_HEAP_PROPERTIES));
	d3dHeapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;//CPU는 접근할 수 없고 GPU는 읽고쓰기 가능한 타입
	d3dHeapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	d3dHeapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	d3dHeapProperties.CreationNodeMask = 1;
	d3dHeapProperties.VisibleNodeMask = 1;

	D3D12_CLEAR_VALUE d3dClearValue;
	d3dClearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	d3dClearValue.DepthStencil.Depth = 1.0f;
	d3dClearValue.DepthStencil.Stencil = 0;
	DX::ThrowIfFailed(_Device->CreateCommittedResource(&d3dHeapProperties, D3D12_HEAP_FLAG_NONE,
		&d3dResourceDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &d3dClearValue,
		IID_PPV_ARGS(&m_pd3dDepthStencilBuffer)));
	//깊이-스텐실 버퍼를 생성한다. 
	D3D12_CPU_DESCRIPTOR_HANDLE d3dDsvCPUDescriptorHandle =
		m_pd3dDsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();

	D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc{};
	depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilViewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Flags = D3D12_DSV_FLAG_NONE;
	_Device->CreateDepthStencilView(m_pd3dDepthStencilBuffer.Get(), &depthStencilViewDesc,
		d3dDsvCPUDescriptorHandle);
	//깊이-스텐실 버퍼 뷰를 생성한다.


}

//->여기 개념부터 보고 작성
void SwapChainAndRtvDsvHeap::CreateRtvAndDsvDescriptorHeaps(ComPtr<ID3D12Device>& _Device) {

	D3D12_DESCRIPTOR_HEAP_DESC d3dDescriptorHeapDesc;
	::ZeroMemory(&d3dDescriptorHeapDesc, sizeof(D3D12_DESCRIPTOR_HEAP_DESC));
	d3dDescriptorHeapDesc.NumDescriptors = SWAP_CHAIN_BUFFER_COUNT;
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	d3dDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	d3dDescriptorHeapDesc.NodeMask = 0;
	DX::ThrowIfFailed(_Device->CreateDescriptorHeap(&d3dDescriptorHeapDesc, IID_PPV_ARGS(&m_pd3dRtvDescriptorHeap)));
	//랜더 타겟 서술자 힙(서술자 개수= 스왑체인 버퍼 개수) 을 생성
	m_nRtvDescriptorIncrementSize = _Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	//렌더 타겟 서술자 힙의 원소의 크기를 저장한다.

	d3dDescriptorHeapDesc.NumDescriptors = 1;
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	DX::ThrowIfFailed(_Device->CreateDescriptorHeap(&d3dDescriptorHeapDesc, IID_PPV_ARGS(&m_pd3dDsvDescriptorHeap)));


}


void SwapChainAndRtvDsvHeap::CreateSwapChain(ComPtr<IDXGIFactory4>& _Factory, ComPtr<ID3D12Device>& _Device, ComPtr< ID3D12CommandQueue>& _CmdQueue, bool MssaaEnable, uint16 MsaaQualityLevels)
{
	DXGI_SWAP_CHAIN_DESC dxgiSwapChainDesc;
	::ZeroMemory(&dxgiSwapChainDesc, sizeof(dxgiSwapChainDesc));
	dxgiSwapChainDesc.BufferCount = SWAP_CHAIN_BUFFER_COUNT;
	dxgiSwapChainDesc.BufferDesc.Width = m_nWndClientWidth;
	dxgiSwapChainDesc.BufferDesc.Height = m_nWndClientHeight;
	dxgiSwapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	dxgiSwapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
	dxgiSwapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	dxgiSwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	dxgiSwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	dxgiSwapChainDesc.OutputWindow = m_hWnd;
	dxgiSwapChainDesc.SampleDesc.Count = (MssaaEnable) ? 4 : 1;
	dxgiSwapChainDesc.SampleDesc.Quality = (MssaaEnable) ? (MsaaQualityLevels - 1) : 0;
	dxgiSwapChainDesc.Windowed = TRUE;
	//전체화면 모드에서 바탕화면의 해상도를 스왑체인(후면버퍼)의 크기에 맞게 변경한다. 
	dxgiSwapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	// CreateSwapChain 함수를 호출하여 IDXGISwapChain을 만듭니다.
	ComPtr<IDXGISwapChain> pdxgiSwapChain;
	DX::ThrowIfFailed(_Factory->CreateSwapChain(_CmdQueue.Get(),
		&dxgiSwapChainDesc, pdxgiSwapChain.GetAddressOf()));


	// IDXGISwapChain3로 변환
	ComPtr<IDXGISwapChain3> pdxgiSwapChain3;
	DX::ThrowIfFailed(pdxgiSwapChain.As(&pdxgiSwapChain3));


	// m_pdxgiSwapChain에 IDXGISwapChain3 할당
	m_cpdxgiSwapChain = pdxgiSwapChain3;

	m_nSwapChainBufferIndex = m_cpdxgiSwapChain->GetCurrentBackBufferIndex();
	DX::ThrowIfFailed(_Factory->MakeWindowAssociation(m_hWnd, DXGI_MWA_NO_ALT_ENTER));
#ifndef _WITH_SWAPCHAIN_FULLSCREEN_STATE
	CreateRenderTargetViews(_Device);
#endif
}