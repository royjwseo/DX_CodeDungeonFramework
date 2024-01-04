#include "stdafx.h"
#include "GameFramework.h"
#include "DeviceAndFactory.h"
#include "SwapChainAndRtvDsvHeap.h"
#include "CommandQueue.h"


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
	DX::ThrowIfFailed(m_spCommandQueue->GetCmdQueue()->Signal(m_spDevice->GetFence().Get(), nFenceValue));
	if (m_spDevice->GetFence()->GetCompletedValue() < nFenceValue) {
		DX::ThrowIfFailed(m_spDevice->GetFence()->SetEventOnCompletion(nFenceValue, m_spDevice->m_hFenceEvent));
		WaitForSingleObjectEx(m_spDevice->m_hFenceEvent, INFINITE,FALSE);
	}

}

void CGameFramework::MoveToNextFrame()
{
	m_spSwapChainAndRtvDsvHeap->SetSwapChainIndex(m_spSwapChainAndRtvDsvHeap->GetSwapChain()->GetCurrentBackBufferIndex());

	UINT64 nFenceValue = ++m_spDevice->m_nFenceValues[m_spSwapChainAndRtvDsvHeap->GetSwapChainIndex()];
	DX::ThrowIfFailed(m_spCommandQueue->GetCmdQueue()->Signal(m_spDevice->GetFence().Get(), nFenceValue));
	if (m_spDevice->GetFence()->GetCompletedValue() < nFenceValue) {
		DX::ThrowIfFailed(m_spDevice->GetFence()->SetEventOnCompletion(nFenceValue, m_spDevice->m_hFenceEvent));
		::WaitForSingleObjectEx(m_spDevice->m_hFenceEvent, INFINITE,FALSE);
	}
}

void CGameFramework::FrameAdvance()
{
	m_GameTimer.Tick(0.0f);

	ProcessInput();
	AnimateObjects();
	DX::ThrowIfFailed(m_spCommandQueue->GetCmdAllocator()->Reset());
	DX::ThrowIfFailed(m_spCommandQueue->GetCmdList()->Reset(m_spCommandQueue->GetCmdAllocator().Get(), NULL));


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


	DX::ThrowIfFailed(m_spCommandQueue->GetCmdList()->Close());
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

