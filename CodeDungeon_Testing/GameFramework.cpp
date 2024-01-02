#include "stdafx.h"
#include "GameFramework.h"
//게임 프로그램의 뼈대이다. 
//Direct3D 디바이스 생성, 관리하며 화면 출력을 위한 여러 가지 처리를 담당한다.
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
#if defined(_DEBUG) //전처리기 조건문 #if있으면 #endif 는 무조건 세트, D3D12 디버그층 활성화
	
	ComPtr<ID3D12Debug>pd3dDebugController = nullptr;
	hResult = D3D12GetDebugInterface(IID_PPV_ARGS(&pd3dDebugController));//(IID_PPV_ARGS(&pd3dDebugController)사용가능
	if (pd3dDebugController) {//내 해석: pd3dDebugController 포인터에 널값 없으면 무언가 초기화하는
		pd3dDebugController->EnableDebugLayer();
	}
	nDXGIFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;//이따 찾아보기
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
	d3dMsaaQualityLevels.SampleCount = 4; //Msaa4x 다중 샘플링
	d3dMsaaQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;//특별한 기능 x 멀티 스레드 사용시 SHARED사용할수도
	d3dMsaaQualityLevels.NumQualityLevels = 0;
	m_cpd3dDevice->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &d3dMsaaQualityLevels, sizeof(D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS));
	m_nMsaa4xQualityLevels = d3dMsaaQualityLevels.NumQualityLevels;
	//디바이스가 지원하는 다중 샘플의 품질 수준을 확인한다. 
	m_bMsaa4xEnable = (m_nMsaa4xQualityLevels > 1) ? true : false;
	//다중 샘플의 품질 수준이 1보다 크면 다중 샘플링을 활성화한다. 

	hResult = m_cpd3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS (&m_cpd3dFence));

	//펜스를 생성하고 펜스 값을 0으로 설정한다

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
	d3dHeapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;//CPU는 접근할 수 없고 GPU는 읽고쓰기 가능한 타입
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
	//깊이-스텐실 버퍼를 생성한다. 
	D3D12_CPU_DESCRIPTOR_HANDLE d3dDsvCPUDescriptorHandle =
		m_pd3dDsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	_Device->CreateDepthStencilView(m_pd3dDepthStencilBuffer.Get(), NULL,
		d3dDsvCPUDescriptorHandle);
	//깊이-스텐실 버퍼 뷰를 생성한다.

	
}

//->여기 개념부터 보고 작성
void SwapChainAndRtvDsvHeap::CreateRtvAndDsvDescriptorHeaps( ComPtr<ID3D12Device>& _Device) {

	D3D12_DESCRIPTOR_HEAP_DESC d3dDescriptorHeapDesc;
	::ZeroMemory(&d3dDescriptorHeapDesc, sizeof(D3D12_DESCRIPTOR_HEAP_DESC));
	d3dDescriptorHeapDesc.NumDescriptors = SWAP_CHAIN_BUFFER_COUNT;
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	d3dDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	d3dDescriptorHeapDesc.NodeMask = 0;
	HRESULT hResult = _Device->CreateDescriptorHeap(&d3dDescriptorHeapDesc, IID_PPV_ARGS(& m_pd3dRtvDescriptorHeap));
	//랜더 타겟 서술자 힙(서술자 개수= 스왑체인 버퍼 개수) 을 생성
	m_nRtvDescriptorIncrementSize = _Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	//렌더 타겟 서술자 힙의 원소의 크기를 저장한다.

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
	//전체화면 모드에서 바탕화면의 해상도를 스왑체인(후면버퍼)의 크기에 맞게 변경한다. 
	dxgiSwapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	// CreateSwapChain 함수를 호출하여 IDXGISwapChain을 만듭니다.
	ComPtr<IDXGISwapChain> pdxgiSwapChain;
	HRESULT hResult = _Factory->CreateSwapChain(_CmdQueue.Get(),
		&dxgiSwapChainDesc, pdxgiSwapChain.GetAddressOf());
	if (FAILED(hResult)) {
		// 오류 처리
		return;
	}

	// IDXGISwapChain3로 변환
	ComPtr<IDXGISwapChain3> pdxgiSwapChain3;
	hResult = pdxgiSwapChain.As(&pdxgiSwapChain3);
	if (FAILED(hResult)) {
		// 오류 처리
		return;
	}

	// m_pdxgiSwapChain에 IDXGISwapChain3 할당
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
	d3dCommandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;//기본 명령 큐로 정함
	d3dCommandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;//우리는 GPU가 직접 실행 할 수 있는 CommandList를 담은 CommandQueue를 만들기 위해 GPU가 직접 실행할 수 있는 명령 버퍼 리스트
	HRESULT hResult = _Device->CreateCommandQueue(&d3dCommandQueueDesc, IID_PPV_ARGS (&m_pd3dCommandQueue));
	//직접(direct) 명령 큐를 생성

	hResult = _Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS (&m_pd3dCommandAllocator));
	//직접(direct) 명령 할당자를 생성한다.

	hResult = _Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_pd3dCommandAllocator.Get(), NULL, IID_PPV_ARGS (&m_pd3dCommandList));
	//직접(dircect) 명령 리스트를 생성한다.                                        //연관된 명령 할당자  //초기 파이프라인 상태 객체

	hResult = m_pd3dCommandList->Close();
	//명령리스트는 초기상태가 열린상태이므로 닫힌 상태로 만든다.

}
//-----------------------------


CGameFramework::CGameFramework()
{

	
}

CGameFramework::~CGameFramework()
{
	

}

//다음 함수는 응용 프로그램이 실행되어 주 윈도우가 생성되면 호출된다는 것에 유의하라. 
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
	//GPU가 모든 명령 리스트를 실행할 때 까지 기다린다. ReleaseObjects();
	//게임 객체(게임 월드 객체)를 소멸한다. ::CloseHandle(m_hFenceEvent);

	m_spSwapChainAndRtvDsvHeap->GetSwapChain()->SetFullscreenState(FALSE, NULL);

#if defined(_DEBUG)
	ComPtr<IDXGIDebug1> pdxgiDebug = nullptr;

	DXGIGetDebugInterface1(0, IID_PPV_ARGS(&pdxgiDebug));
	HRESULT hResult = pdxgiDebug->ReportLiveObjects(DXGI_DEBUG_ALL,
		DXGI_DEBUG_RLO_DETAIL);
	
#endif
}









//나중에 hResult-> FAILED_CHECK_RETURN 같이 실패시 반환함수 작성.
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


	//명령 할당자와 명령 리스트를 리셋한다.
	D3D12_RESOURCE_BARRIER d3dResourceBarrier;
	::ZeroMemory(&d3dResourceBarrier, sizeof(D3D12_RESOURCE_BARRIER));
	d3dResourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	d3dResourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	d3dResourceBarrier.Transition.pResource = m_spSwapChainAndRtvDsvHeap->GetRenderTarget(m_spSwapChainAndRtvDsvHeap->GetSwapChain()->GetCurrentBackBufferIndex()).Get();
	d3dResourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;//gPU입장 read
	d3dResourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;//gpu입장 write
	d3dResourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	m_spCommandQueue->GetCmdList()->ResourceBarrier(1, &d3dResourceBarrier);
	/*현재 렌더 타겟에 대한 프리젠트가 끝나기를 기다린다. 프리젠트가 끝나면 렌더 타겟 버퍼의 상태는 프리젠트 상태
	(D3D12_RESOURCE_STATE_PRESENT)에서 렌더 타겟 상태(D3D12_RESOURCE_STATE_RENDER_TARGET)로 바
	뀔 것이다.*/

	D3D12_CPU_DESCRIPTOR_HANDLE d3dRtvCPUDescriptorHandle =
		m_spSwapChainAndRtvDsvHeap->GetRTVHeap()->GetCPUDescriptorHandleForHeapStart();
	d3dRtvCPUDescriptorHandle.ptr += (m_spSwapChainAndRtvDsvHeap->GetSwapChain()->GetCurrentBackBufferIndex() *
		m_spSwapChainAndRtvDsvHeap->GetRTVIncrementSize());
	//현재의 렌더 타겟에 해당하는 서술자의 CPU 주소(핸들)를 계산한다. 
	float pfClearColor[4] = { 0.0f, 0.125f, 0.3f, 1.0f };
	m_spCommandQueue->GetCmdList()->ClearRenderTargetView(d3dRtvCPUDescriptorHandle,
		pfClearColor/*Colors::Azure*/, 0, NULL);
	//원하는 색상으로 렌더 타겟(뷰)을 지운다. 
	D3D12_CPU_DESCRIPTOR_HANDLE d3dDsvCPUDescriptorHandle =
		m_spSwapChainAndRtvDsvHeap->GetDSVHeap()->GetCPUDescriptorHandleForHeapStart();
	//깊이-스텐실 서술자의 CPU 주소를 계산한다.
	m_spCommandQueue->GetCmdList()->ClearDepthStencilView(d3dDsvCPUDescriptorHandle,
		D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, NULL);
	//원하는 값으로 깊이-스텐실(뷰)을 지운다.
	m_spCommandQueue->GetCmdList()->OMSetRenderTargets(1, &d3dRtvCPUDescriptorHandle, TRUE,
		&d3dDsvCPUDescriptorHandle);
	if (m_pScene)m_pScene->Render(m_spCommandQueue->GetCmdList().Get(),m_pCamera);
	// 
	// 
	//렌더 타겟 뷰(서술자)와 깊이-스텐실 뷰(서술자)를 출력-병합 단계(OM)에 연결한다. //렌더링 코드는 여기에 추가될 것이다.
	d3dResourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	d3dResourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	d3dResourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	m_spCommandQueue->GetCmdList()->ResourceBarrier(1, &d3dResourceBarrier);
	/*현재 렌더 타겟에 대한 렌더링이 끝나기를 기다린다. GPU가 렌더 타겟(버퍼)을 더 이상 사용하지 않으면 렌더 타겟
	의 상태는 프리젠트 상태(D3D12_RESOURCE_STATE_PRESENT)로 바뀔 것이다.*/


	hResult = m_spCommandQueue->GetCmdList()->Close();
	//명령 리스트를 닫힌 상태로 만든다. 
	ComPtr<ID3D12CommandList>ppd3dCommandLists[]={ m_spCommandQueue->GetCmdList().Get() };
	
	m_spCommandQueue->GetCmdQueue()->ExecuteCommandLists(1, ppd3dCommandLists->GetAddressOf());
	//명령 리스트를 명령 큐에 추가하여 실행한다.
	WaitForGpuComplete();
	m_spSwapChainAndRtvDsvHeap->GetSwapChain()->Present(0, 0);
	MoveToNextFrame();
	m_GameTimer.GetFrameRate(m_pszFrameRate + 12, 37);
	::SetWindowText(m_spSwapChainAndRtvDsvHeap->m_hWnd, m_pszFrameRate);
}



void CGameFramework::BuildObjects()
{
	m_spCommandQueue->GetCmdList()->Reset(m_spCommandQueue->GetCmdAllocator().Get(), NULL);

	//카메라 객체를 생성하여 뷰포트, 씨저 사각형, 투영 변환 행렬, 카메라 변환 행렬을 생성하고 설정한다. 
	m_pCamera = new CCamera();
	m_pCamera->SetViewport(0, 0, m_spSwapChainAndRtvDsvHeap->m_nWndClientWidth, m_spSwapChainAndRtvDsvHeap->m_nWndClientHeight, 0.0f, 1.0f);
	m_pCamera->SetScissorRect(0, 0, m_spSwapChainAndRtvDsvHeap->m_nWndClientWidth, m_spSwapChainAndRtvDsvHeap->m_nWndClientHeight);
	m_pCamera->GenerateProjectionMatrix(1.0f, 500.0f, float(m_spSwapChainAndRtvDsvHeap->m_nWndClientWidth) /
		float(m_spSwapChainAndRtvDsvHeap->m_nWndClientHeight), 90.0f);
	m_pCamera->GenerateViewMatrix(XMFLOAT3(0.0f, 15.0f, -25.0f), XMFLOAT3(0.0f, 0.0f,
		0.0f), XMFLOAT3(0.0f, 1.0f, 0.0f));

	//씬 객체를 생성하고 씬에 포함될 게임 객체들을 생성한다. 
	m_pScene = new CScene();
	m_pScene->BuildObjects(m_spDevice->GetDevice().Get(), m_spCommandQueue->GetCmdList().Get());
	//씬 객체를 생성하기 위하여 필요한 그래픽 명령 리스트들을 명령 큐에 추가한다.
	m_spCommandQueue->GetCmdList()->Close();
	ComPtr<ID3D12CommandList>ppd3dCommandLists[] = { m_spCommandQueue->GetCmdList().Get() };
	m_spCommandQueue->GetCmdQueue()->ExecuteCommandLists(1, ppd3dCommandLists->GetAddressOf());
	//그래픽 명령 리스트들이 모두 실행될 때까지 기다린다.
	WaitForGpuComplete();
	//그래픽 리소스들을 생성하는 과정에 생성된 업로드 버퍼들을 소멸시킨다. 
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
			//“F9” 키가 눌려지면 윈도우 모드와 전체화면 모드의 전환을 처리한다. 
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

