#include "stdafx.h"
#include "SwapChain.h"


void SwapChain::CreateSwapChain(const WindowInfo& info, ComPtr<IDXGIFactory> cpFactory, ComPtr<ID3D12CommandQueue> cpCmdQueue)
{
	DXGI_SWAP_CHAIN_DESC dxgiSwapChainDesc;
	::ZeroMemory(&dxgiSwapChainDesc, sizeof(dxgiSwapChainDesc));
	dxgiSwapChainDesc.BufferCount = m_nSwapChainBuffers;
	dxgiSwapChainDesc.BufferDesc.Width = static_cast<uint32>(info.Width);;
	dxgiSwapChainDesc.BufferDesc.Height = static_cast<uint32>(info.Height);;
	dxgiSwapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	dxgiSwapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
	dxgiSwapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	dxgiSwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	dxgiSwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	dxgiSwapChainDesc.OutputWindow = info.Hwnd;
	dxgiSwapChainDesc.SampleDesc.Count = (info.Msaa4xEnable) ? 4 : 1;
	dxgiSwapChainDesc.SampleDesc.Quality = (info.Msaa4xEnable) ? (info.Msaa4xQualityLevels - 1) : 0;
	dxgiSwapChainDesc.Windowed = info.Windowed;
	//��üȭ�� ��忡�� ����ȭ���� �ػ󵵸� ����ü��(�ĸ����)�� ũ�⿡ �°� �����Ѵ�. 
	dxgiSwapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	// CreateSwapChain �Լ��� ȣ���Ͽ� IDXGISwapChain�� ����ϴ�.
	ComPtr<IDXGISwapChain> pdxgiSwapChain;
	HRESULT hResult = cpFactory->CreateSwapChain(cpCmdQueue.Get(),
		&dxgiSwapChainDesc, pdxgiSwapChain.GetAddressOf());
	if (FAILED(hResult)) {
		// ���� ó��
		return;
	}

	// IDXGISwapChain3�� ��ȯ
	ComPtr<IDXGISwapChain3> pdxgiSwapChain3;
	hResult = pdxgiSwapChain.As(&pdxgiSwapChain3);
	if (FAILED(hResult)) {
		// ���� ó��
		return;
	}

	// m_pdxgiSwapChain�� IDXGISwapChain3 �Ҵ�
	m_pdxgiSwapChain = pdxgiSwapChain3;

	m_nSwapChainBufferIndex = m_pdxgiSwapChain->GetCurrentBackBufferIndex();
	hResult = cpFactory->MakeWindowAssociation(info.Hwnd, DXGI_MWA_NO_ALT_ENTER);
#/*ifndef _WITH_SWAPCHAIN_FULLSCREEN_STATE
	CreateRenderTargetViews();
#endif*/
}

void SwapChain::CreateRTV(ComPtr<ID3D12Device> cpDevice)
{
	D3D12_CPU_DESCRIPTOR_HANDLE d3dRtvCPUDescriptorHandle =
		m_pd3dRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();

	for (UINT i = 0; i < m_nSwapChainBuffers; i++) {
		m_pdxgiSwapChain->GetBuffer(i, __uuidof(ID3D12Resource), (void**)&m_ppd3dRenderTargetBuffers[i]);
		cpDevice->CreateRenderTargetView(m_ppd3dRenderTargetBuffers[i].Get(), NULL, d3dRtvCPUDescriptorHandle);
		d3dRtvCPUDescriptorHandle.ptr += m_nRtvDescriptorIncrementSize;

	}

}

void SwapChain::CreateRTHeap(ComPtr<ID3D12Device> cpDevice)
{
	D3D12_DESCRIPTOR_HEAP_DESC d3dDescriptorHeapDesc;
	::ZeroMemory(&d3dDescriptorHeapDesc, sizeof(D3D12_DESCRIPTOR_HEAP_DESC));
	d3dDescriptorHeapDesc.NumDescriptors = m_nSwapChainBuffers;
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	d3dDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	d3dDescriptorHeapDesc.NodeMask = 0;
	HRESULT hResult = cpDevice->CreateDescriptorHeap(&d3dDescriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), (void**)&m_pd3dRtvDescriptorHeap);
	
	m_nRtvDescriptorIncrementSize = cpDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

}

