#include "stdafx.h"
#include "GameFramework.h"
//���� ���α׷��� �����̴�. 
//Direct3D ����̽� ����, �����ϸ� ȭ�� ����� ���� ���� ���� ó���� ����Ѵ�.
//c

//-----------------------------------------
//Device


void DeviceAndFactory::InitDeviceAndFactory()
{
	CreateDirect3DDevice();
}

void DeviceAndFactory::CreateDirect3DDevice()
{
	HRESULT hResult;

	UINT nDXGIFactoryFlags = 0;
#if defined(_DEBUG) //��ó���� ���ǹ� #if������ #endif �� ������ ��Ʈ, D3D12 ������� Ȱ��ȭ
	
	ComPtr<ID3D12Debug>pd3dDebugController = nullptr;
	hResult = D3D12GetDebugInterface(IID_PPV_ARGS(&pd3dDebugController));//(IID_PPV_ARGS(&pd3dDebugController)��밡��
	if (pd3dDebugController) {//�� �ؼ�: pd3dDebugController �����Ϳ� �ΰ� ������ ���� �ʱ�ȭ�ϴ�
		pd3dDebugController->EnableDebugLayer();
	}
	nDXGIFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;//�̵� ã�ƺ���
#endif

	hResult = ::CreateDXGIFactory2(nDXGIFactoryFlags, IID_PPV_ARGS(&m_cpdxgiFactory));//(IID_PPV_ARGS(&m_pdxgiFactory)

	ComPtr<IDXGIAdapter1> pd3dAdapter = nullptr;


	for (UINT i = 0; m_cpdxgiFactory->EnumAdapters1(i, &pd3dAdapter) != DXGI_ERROR_NOT_FOUND; ++i) {
		DXGI_ADAPTER_DESC1 dxgiAdapterDesc;
		pd3dAdapter->GetDesc1(&dxgiAdapterDesc);

		if (dxgiAdapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
			continue;

		if (SUCCEEDED(D3D12CreateDevice(pd3dAdapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&m_cpd3dDevice))))
			break;
	}

	if (!pd3dAdapter) {
		m_cpdxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&pd3dAdapter));
		D3D12CreateDevice(pd3dAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_cpd3dDevice));
	}

	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS d3dMsaaQualityLevels;
	d3dMsaaQualityLevels.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	d3dMsaaQualityLevels.SampleCount = 4; //Msaa4x ���� ���ø�
	d3dMsaaQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;//Ư���� ��� x ��Ƽ ������ ���� SHARED����Ҽ���
	d3dMsaaQualityLevels.NumQualityLevels = 0;
	m_cpd3dDevice->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &d3dMsaaQualityLevels, sizeof(D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS));
	m_nMsaa4xQualityLevels = d3dMsaaQualityLevels.NumQualityLevels;
	//����̽��� �����ϴ� ���� ������ ǰ�� ������ Ȯ���Ѵ�. 
	m_bMsaa4xEnable = (m_nMsaa4xQualityLevels > 1) ? true : false;
	//���� ������ ǰ�� ������ 1���� ũ�� ���� ���ø��� Ȱ��ȭ�Ѵ�. 

	hResult = m_cpd3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS (&m_cpd3dFence));

	//�潺�� �����ϰ� �潺 ���� 0���� �����Ѵ�

	m_hFenceEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);


	
}
//-----------------------------------------


//---------------------------
void SwapChainAndRtvDsvHeap::InitSwapChainAndRtvDsvHeap(ComPtr<IDXGIFactory4>& _Factory,  ComPtr<ID3D12Device>& _Device,  ComPtr< ID3D12CommandQueue>& _CmdQueue, bool MsaaEnable, uint16 MsaaQualityLevels)
{
	m_nRtvDescriptorIncrementSize = 0;
	m_nDsvDescriptorIncrementSize = 0;
	m_nSwapChainBufferIndex = 0;

	m_nWndClientWidth = FRAME_BUFFER_WIDTH;
	m_nWndClientHeight = FRAME_BUFFER_HEIGHT;

	CreateRtvAndDsvDescriptorHeaps(_Device);
	CreateSwapChain(_Factory,_Device,_CmdQueue, MsaaEnable, MsaaQualityLevels);
	CreateDepthStencilView(_Device, MsaaEnable,MsaaQualityLevels);


	
}

void SwapChainAndRtvDsvHeap::ChangeSwapChainState( ComPtr<ID3D12Device>& _Device)
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


void SwapChainAndRtvDsvHeap::CreateRenderTargetViews( ComPtr<ID3D12Device>& _Device)
{
	D3D12_CPU_DESCRIPTOR_HANDLE d3dRtvCPUDescriptorHandle =
		m_pd3dRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();

	for (UINT i = 0; i < SWAP_CHAIN_BUFFER_COUNT; i++) {
		m_cpdxgiSwapChain->GetBuffer(i,IID_PPV_ARGS(& m_ppd3dRenderTargetBuffers[i]));
		_Device->CreateRenderTargetView(m_ppd3dRenderTargetBuffers[i].Get(), NULL, d3dRtvCPUDescriptorHandle);
		d3dRtvCPUDescriptorHandle.ptr += m_nRtvDescriptorIncrementSize;

	}
	
}

void SwapChainAndRtvDsvHeap::CreateDepthStencilView( ComPtr<ID3D12Device>& _Device, bool MssaaEnable, uint16 MsaaQualityLevels)
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
	d3dResourceDesc.SampleDesc.Quality = (MssaaEnable) ? (MsaaQualityLevels - 1)
		: 0;
	d3dResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	d3dResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_HEAP_PROPERTIES d3dHeapProperties;
	::ZeroMemory(&d3dHeapProperties, sizeof(D3D12_HEAP_PROPERTIES));
	d3dHeapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;//CPU�� ������ �� ���� GPU�� �а��� ������ Ÿ��
	d3dHeapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	d3dHeapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	d3dHeapProperties.CreationNodeMask = 1;
	d3dHeapProperties.VisibleNodeMask = 1;
	D3D12_CLEAR_VALUE d3dClearValue;
	d3dClearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	d3dClearValue.DepthStencil.Depth = 1.0f;
	d3dClearValue.DepthStencil.Stencil = 0;
	_Device->CreateCommittedResource(&d3dHeapProperties, D3D12_HEAP_FLAG_NONE,
		&d3dResourceDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &d3dClearValue,
		IID_PPV_ARGS (& m_pd3dDepthStencilBuffer));
	//����-���ٽ� ���۸� �����Ѵ�. 
	D3D12_CPU_DESCRIPTOR_HANDLE d3dDsvCPUDescriptorHandle =
		m_pd3dDsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	_Device->CreateDepthStencilView(m_pd3dDepthStencilBuffer.Get(), NULL,
		d3dDsvCPUDescriptorHandle);
	//����-���ٽ� ���� �並 �����Ѵ�.

	
}

//->���� ������� ���� �ۼ�
void SwapChainAndRtvDsvHeap::CreateRtvAndDsvDescriptorHeaps( ComPtr<ID3D12Device>& _Device) {

	D3D12_DESCRIPTOR_HEAP_DESC d3dDescriptorHeapDesc;
	::ZeroMemory(&d3dDescriptorHeapDesc, sizeof(D3D12_DESCRIPTOR_HEAP_DESC));
	d3dDescriptorHeapDesc.NumDescriptors = SWAP_CHAIN_BUFFER_COUNT;
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	d3dDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	d3dDescriptorHeapDesc.NodeMask = 0;
	HRESULT hResult = _Device->CreateDescriptorHeap(&d3dDescriptorHeapDesc, IID_PPV_ARGS(& m_pd3dRtvDescriptorHeap));
	//���� Ÿ�� ������ ��(������ ����= ����ü�� ���� ����) �� ����
	m_nRtvDescriptorIncrementSize = _Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	//���� Ÿ�� ������ ���� ������ ũ�⸦ �����Ѵ�.

	d3dDescriptorHeapDesc.NumDescriptors = 1;
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	hResult = _Device->CreateDescriptorHeap(&d3dDescriptorHeapDesc, IID_PPV_ARGS(&m_pd3dDsvDescriptorHeap));


}


void SwapChainAndRtvDsvHeap::CreateSwapChain( ComPtr<IDXGIFactory4>& _Factory,  ComPtr<ID3D12Device>& _Device,  ComPtr< ID3D12CommandQueue>& _CmdQueue, bool MssaaEnable, uint16 MsaaQualityLevels)
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
	//��üȭ�� ��忡�� ����ȭ���� �ػ󵵸� ����ü��(�ĸ����)�� ũ�⿡ �°� �����Ѵ�. 
	dxgiSwapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	// CreateSwapChain �Լ��� ȣ���Ͽ� IDXGISwapChain�� ����ϴ�.
	ComPtr<IDXGISwapChain> pdxgiSwapChain;
	HRESULT hResult = _Factory->CreateSwapChain(_CmdQueue.Get(),
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
	m_cpdxgiSwapChain = pdxgiSwapChain3;

	m_nSwapChainBufferIndex = m_cpdxgiSwapChain->GetCurrentBackBufferIndex();
	hResult = _Factory->MakeWindowAssociation(m_hWnd, DXGI_MWA_NO_ALT_ENTER);
#ifndef _WITH_SWAPCHAIN_FULLSCREEN_STATE
	CreateRenderTargetViews(_Device);
#endif
}

//---------------------------


void CommandQueue::CreateCommandQueueAndList(ComPtr<ID3D12Device>& _Device) {

	D3D12_COMMAND_QUEUE_DESC d3dCommandQueueDesc;
	::ZeroMemory(&d3dCommandQueueDesc, sizeof(D3D12_COMMAND_QUEUE_DESC));
	d3dCommandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;//�⺻ ��� ť�� ����
	d3dCommandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;//�츮�� GPU�� ���� ���� �� �� �ִ� CommandList�� ���� CommandQueue�� ����� ���� GPU�� ���� ������ �� �ִ� ��� ���� ����Ʈ
	HRESULT hResult = _Device->CreateCommandQueue(&d3dCommandQueueDesc, IID_PPV_ARGS (&m_pd3dCommandQueue));
	//����(direct) ��� ť�� ����

	hResult = _Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS (&m_pd3dCommandAllocator));
	//����(direct) ��� �Ҵ��ڸ� �����Ѵ�.

	hResult = _Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_pd3dCommandAllocator.Get(), NULL, IID_PPV_ARGS (&m_pd3dCommandList));
	//����(dircect) ��� ����Ʈ�� �����Ѵ�.                                        //������ ��� �Ҵ���  //�ʱ� ���������� ���� ��ü

	hResult = m_pd3dCommandList->Close();
	//��ɸ���Ʈ�� �ʱ���°� ���������̹Ƿ� ���� ���·� �����.

}
//-----------------------------


CGameFramework::CGameFramework()
{

	
}

CGameFramework::~CGameFramework()
{
	

}

//���� �Լ��� ���� ���α׷��� ����Ǿ� �� �����찡 �����Ǹ� ȣ��ȴٴ� �Ϳ� �����϶�. 
bool CGameFramework::OnCreate(HINSTANCE hInstance, HWND hMainWnd)
{
	m_spDevice = make_shared<DeviceAndFactory>();
	m_spSwapChainAndRtvDsvHeap = make_shared<SwapChainAndRtvDsvHeap>();
	m_spCommandQueue = make_shared<CommandQueue>();

	for (int i = 0; i < SWAP_CHAIN_BUFFER_COUNT; i++)m_spDevice->m_nFenceValues[i] = 0;
	m_pScene = NULL;

	_tcscpy_s(m_pszFrameRate, _T("LapProject ("));

	m_spSwapChainAndRtvDsvHeap->m_hInstance = hInstance;
	m_spSwapChainAndRtvDsvHeap->m_hWnd = hMainWnd;
	m_spDevice->InitDeviceAndFactory();
	m_spCommandQueue->CreateCommandQueueAndList(m_spDevice->GetDevice());
	m_spSwapChainAndRtvDsvHeap->InitSwapChainAndRtvDsvHeap(m_spDevice->GetFactory(),m_spDevice->GetDevice(), m_spCommandQueue->GetCmdQueue(), m_spDevice->GetMsaa4xEnable(), m_spDevice->GetMsaa4xQualityLevels());
	
	BuildObjects();
	return(true);
}

void CGameFramework::OnDestroy()
{
	WaitForGpuComplete();
	//GPU�� ��� ��� ����Ʈ�� ������ �� ���� ��ٸ���. ReleaseObjects();
	//���� ��ü(���� ���� ��ü)�� �Ҹ��Ѵ�. ::CloseHandle(m_hFenceEvent);

	m_spSwapChainAndRtvDsvHeap->GetSwapChain()->SetFullscreenState(FALSE, NULL);

#if defined(_DEBUG)
	ComPtr<IDXGIDebug1> pdxgiDebug = nullptr;

	DXGIGetDebugInterface1(0, IID_PPV_ARGS(&pdxgiDebug));
	HRESULT hResult = pdxgiDebug->ReportLiveObjects(DXGI_DEBUG_ALL,
		DXGI_DEBUG_RLO_DETAIL);
	
#endif
}









//���߿� hResult-> FAILED_CHECK_RETURN ���� ���н� ��ȯ�Լ� �ۼ�.
void CGameFramework::WaitForGpuComplete() {

	UINT64 nFenceValue = ++m_spDevice->m_nFenceValues[m_spSwapChainAndRtvDsvHeap->GetSwapChainIndex()];
	HRESULT hResult = m_spCommandQueue->GetCmdQueue()->Signal(m_spDevice->GetFence().Get(), nFenceValue);
	if (m_spDevice->GetFence()->GetCompletedValue() < nFenceValue) {
		hResult = m_spDevice->GetFence()->SetEventOnCompletion(nFenceValue, m_spDevice->m_hFenceEvent);
		::WaitForSingleObject(m_spDevice->m_hFenceEvent, INFINITE);
	}

}

void CGameFramework::MoveToNextFrame()
{
	m_spSwapChainAndRtvDsvHeap->SetSwapChainIndex(m_spSwapChainAndRtvDsvHeap->GetSwapChain()->GetCurrentBackBufferIndex());

	UINT64 nFenceValue = ++m_spDevice->m_nFenceValues[m_spSwapChainAndRtvDsvHeap->GetSwapChainIndex()];
	HRESULT hResult = m_spCommandQueue->GetCmdQueue()->Signal(m_spDevice->GetFence().Get(), nFenceValue);
	if (m_spDevice->GetFence()->GetCompletedValue() < nFenceValue) {
		hResult = m_spDevice->GetFence()->SetEventOnCompletion(nFenceValue, m_spDevice->m_hFenceEvent);
		::WaitForSingleObject(m_spDevice->m_hFenceEvent, INFINITE);
	}
}

void CGameFramework::FrameAdvance()
{
	m_GameTimer.Tick(0.0f);

	ProcessInput();
	AnimateObjects();
	HRESULT hResult = m_spCommandQueue->GetCmdAllocator()->Reset();
	hResult = m_spCommandQueue->GetCmdList()->Reset(m_spCommandQueue->GetCmdAllocator().Get(), NULL);


	//��� �Ҵ��ڿ� ��� ����Ʈ�� �����Ѵ�.
	D3D12_RESOURCE_BARRIER d3dResourceBarrier;
	::ZeroMemory(&d3dResourceBarrier, sizeof(D3D12_RESOURCE_BARRIER));
	d3dResourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	d3dResourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	d3dResourceBarrier.Transition.pResource = m_spSwapChainAndRtvDsvHeap->GetRenderTarget(m_spSwapChainAndRtvDsvHeap->GetSwapChain()->GetCurrentBackBufferIndex()).Get();
	d3dResourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;//gPU���� read
	d3dResourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;//gpu���� write
	d3dResourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	m_spCommandQueue->GetCmdList()->ResourceBarrier(1, &d3dResourceBarrier);
	/*���� ���� Ÿ�ٿ� ���� ������Ʈ�� �����⸦ ��ٸ���. ������Ʈ�� ������ ���� Ÿ�� ������ ���´� ������Ʈ ����
	(D3D12_RESOURCE_STATE_PRESENT)���� ���� Ÿ�� ����(D3D12_RESOURCE_STATE_RENDER_TARGET)�� ��
	�� ���̴�.*/

	D3D12_CPU_DESCRIPTOR_HANDLE d3dRtvCPUDescriptorHandle =
		m_spSwapChainAndRtvDsvHeap->GetRTVHeap()->GetCPUDescriptorHandleForHeapStart();
	d3dRtvCPUDescriptorHandle.ptr += (m_spSwapChainAndRtvDsvHeap->GetSwapChain()->GetCurrentBackBufferIndex() *
		m_spSwapChainAndRtvDsvHeap->GetRTVIncrementSize());
	//������ ���� Ÿ�ٿ� �ش��ϴ� �������� CPU �ּ�(�ڵ�)�� ����Ѵ�. 
	float pfClearColor[4] = { 0.0f, 0.125f, 0.3f, 1.0f };
	m_spCommandQueue->GetCmdList()->ClearRenderTargetView(d3dRtvCPUDescriptorHandle,
		pfClearColor/*Colors::Azure*/, 0, NULL);
	//���ϴ� �������� ���� Ÿ��(��)�� �����. 
	D3D12_CPU_DESCRIPTOR_HANDLE d3dDsvCPUDescriptorHandle =
		m_spSwapChainAndRtvDsvHeap->GetDSVHeap()->GetCPUDescriptorHandleForHeapStart();
	//����-���ٽ� �������� CPU �ּҸ� ����Ѵ�.
	m_spCommandQueue->GetCmdList()->ClearDepthStencilView(d3dDsvCPUDescriptorHandle,
		D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, NULL);
	//���ϴ� ������ ����-���ٽ�(��)�� �����.
	m_spCommandQueue->GetCmdList()->OMSetRenderTargets(1, &d3dRtvCPUDescriptorHandle, TRUE,
		&d3dDsvCPUDescriptorHandle);
	if (m_pScene)m_pScene->Render(m_spCommandQueue->GetCmdList().Get(),m_pCamera);
	// 
	// 
	//���� Ÿ�� ��(������)�� ����-���ٽ� ��(������)�� ���-���� �ܰ�(OM)�� �����Ѵ�. //������ �ڵ�� ���⿡ �߰��� ���̴�.
	d3dResourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	d3dResourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	d3dResourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	m_spCommandQueue->GetCmdList()->ResourceBarrier(1, &d3dResourceBarrier);
	/*���� ���� Ÿ�ٿ� ���� �������� �����⸦ ��ٸ���. GPU�� ���� Ÿ��(����)�� �� �̻� ������� ������ ���� Ÿ��
	�� ���´� ������Ʈ ����(D3D12_RESOURCE_STATE_PRESENT)�� �ٲ� ���̴�.*/


	hResult = m_spCommandQueue->GetCmdList()->Close();
	//��� ����Ʈ�� ���� ���·� �����. 
	ComPtr<ID3D12CommandList>ppd3dCommandLists[]={ m_spCommandQueue->GetCmdList().Get() };
	
	m_spCommandQueue->GetCmdQueue()->ExecuteCommandLists(1, ppd3dCommandLists->GetAddressOf());
	//��� ����Ʈ�� ��� ť�� �߰��Ͽ� �����Ѵ�.
	WaitForGpuComplete();
	m_spSwapChainAndRtvDsvHeap->GetSwapChain()->Present(0, 0);
	MoveToNextFrame();
	m_GameTimer.GetFrameRate(m_pszFrameRate + 12, 37);
	::SetWindowText(m_spSwapChainAndRtvDsvHeap->m_hWnd, m_pszFrameRate);
}



void CGameFramework::BuildObjects()
{
	m_spCommandQueue->GetCmdList()->Reset(m_spCommandQueue->GetCmdAllocator().Get(), NULL);

	//ī�޶� ��ü�� �����Ͽ� ����Ʈ, ���� �簢��, ���� ��ȯ ���, ī�޶� ��ȯ ����� �����ϰ� �����Ѵ�. 
	m_pCamera = new CCamera();
	m_pCamera->SetViewport(0, 0, m_spSwapChainAndRtvDsvHeap->m_nWndClientWidth, m_spSwapChainAndRtvDsvHeap->m_nWndClientHeight, 0.0f, 1.0f);
	m_pCamera->SetScissorRect(0, 0, m_spSwapChainAndRtvDsvHeap->m_nWndClientWidth, m_spSwapChainAndRtvDsvHeap->m_nWndClientHeight);
	m_pCamera->GenerateProjectionMatrix(1.0f, 500.0f, float(m_spSwapChainAndRtvDsvHeap->m_nWndClientWidth) /
		float(m_spSwapChainAndRtvDsvHeap->m_nWndClientHeight), 90.0f);
	m_pCamera->GenerateViewMatrix(XMFLOAT3(0.0f, 15.0f, -25.0f), XMFLOAT3(0.0f, 0.0f,
		0.0f), XMFLOAT3(0.0f, 1.0f, 0.0f));

	//�� ��ü�� �����ϰ� ���� ���Ե� ���� ��ü���� �����Ѵ�. 
	m_pScene = new CScene();
	m_pScene->BuildObjects(m_spDevice->GetDevice().Get(), m_spCommandQueue->GetCmdList().Get());
	//�� ��ü�� �����ϱ� ���Ͽ� �ʿ��� �׷��� ��� ����Ʈ���� ��� ť�� �߰��Ѵ�.
	m_spCommandQueue->GetCmdList()->Close();
	ComPtr<ID3D12CommandList>ppd3dCommandLists[] = { m_spCommandQueue->GetCmdList().Get() };
	m_spCommandQueue->GetCmdQueue()->ExecuteCommandLists(1, ppd3dCommandLists->GetAddressOf());
	//�׷��� ��� ����Ʈ���� ��� ����� ������ ��ٸ���.
	WaitForGpuComplete();
	//�׷��� ���ҽ����� �����ϴ� ������ ������ ���ε� ���۵��� �Ҹ��Ų��. 
	if (m_pScene) m_pScene->ReleaseUploadBuffers();
	m_GameTimer.Reset();
}
void CGameFramework::ReleaseObjects()
{
	if (m_pScene)m_pScene->ReleaseObjects();
	if (m_pScene)delete m_pScene;
}



void CGameFramework::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam,
	LPARAM lParam)
{
	switch (nMessageID)
	{
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
		break;
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
		break;
	case WM_MOUSEMOVE:
		break;
	default:
		break;
	}
}
void CGameFramework::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM
	wParam, LPARAM lParam)
{
	switch (nMessageID)
	{
	case WM_KEYUP:
		switch (wParam)
		{
		case VK_ESCAPE:
			::PostQuitMessage(0);
			break;
		case VK_RETURN:
			break;
			//��F9�� Ű�� �������� ������ ���� ��üȭ�� ����� ��ȯ�� ó���Ѵ�. 
		case VK_F9:
			WaitForGpuComplete();
			m_spSwapChainAndRtvDsvHeap->ChangeSwapChainState(m_spDevice->GetDevice());
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}
}
LRESULT CALLBACK CGameFramework::OnProcessingWindowMessage(HWND hWnd, UINT nMessageID,
	WPARAM wParam, LPARAM lParam)
{
	switch (nMessageID)
	{
	case WM_SIZE:
	{
		m_spSwapChainAndRtvDsvHeap->m_nWndClientWidth = LOWORD(lParam);
		m_spSwapChainAndRtvDsvHeap->m_nWndClientHeight = HIWORD(lParam);
		break;
	}
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
	case WM_MOUSEMOVE:
		OnProcessingMouseMessage(hWnd, nMessageID, wParam, lParam);
		break;
	case WM_KEYDOWN:
	case WM_KEYUP:
		OnProcessingKeyboardMessage(hWnd, nMessageID, wParam, lParam);
		break;
	}
	return(0);
}
void CGameFramework::ProcessInput()
{
}
void CGameFramework::AnimateObjects()
{
	if (m_pScene)m_pScene->AnimateObjects(m_GameTimer.GetTimeElapsed());
}

