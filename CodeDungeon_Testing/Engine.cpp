#include "stdafx.h"
#include "Engine.h"


bool Engine::OnCreate(HINSTANCE hInstance, HWND hMainWnd)
{
	m_spDevice = make_shared<DeviceAndFactory>();
	m_spSwapChainAndRtvDsvHeap = make_shared<SwapChainAndRtvDsvHeap>();
	m_spCommandQueue = make_shared<CommandQueue>();
	m_spRootSignature = make_shared<RootSignature>();

	for (int i = 0; i < SWAP_CHAIN_BUFFER_COUNT; i++)m_spCommandQueue->m_nFenceValues[i] = 0;
	m_pScene = NULL;

	_tcscpy_s(m_pszFrameRate, _T("LapProject ("));

	m_spSwapChainAndRtvDsvHeap->m_hInstance = hInstance;
	m_spSwapChainAndRtvDsvHeap->m_hWnd = hMainWnd;
	m_spDevice->InitDeviceAndFactory();
	m_spCommandQueue->CreateCommandQueueAndList(m_spDevice->GetDevice(), m_spSwapChainAndRtvDsvHeap);
	m_spSwapChainAndRtvDsvHeap->InitSwapChainAndRtvDsvHeap(m_spDevice->GetFactory(), m_spDevice->GetDevice(), m_spCommandQueue->GetCmdQueue(), m_spDevice->GetMsaa4xEnable(), m_spDevice->GetMsaa4xQualityLevels());
	m_spRootSignature->InitRootSignature(m_spDevice->GetDevice());

	BuildObjects();
	return(true);
}

void Engine::OnDestroy()
{
	m_spCommandQueue->WaitForGpuComplete();

	m_spSwapChainAndRtvDsvHeap->GetSwapChain()->SetFullscreenState(FALSE, NULL);

#if defined(_DEBUG)
	ComPtr<IDXGIDebug1> pdxgiDebug = nullptr;

	DXGIGetDebugInterface1(0, IID_PPV_ARGS(&pdxgiDebug));
	HRESULT hResult = pdxgiDebug->ReportLiveObjects(DXGI_DEBUG_ALL,
		DXGI_DEBUG_RLO_DETAIL);

#endif
}


void Engine::BuildObjects()
{
	m_spCommandQueue->GetCmdList()->Reset(m_spCommandQueue->GetCmdAllocator().Get(), NULL);

	//카메라 객체를 생성하여 뷰포트, 씨저 사각형, 투영 변환 행렬, 카메라 변환 행렬을 생성하고 설정한다. 
	m_pCamera = new CCamera();
	m_pCamera->SetViewport(0, 0, m_spSwapChainAndRtvDsvHeap->m_nWndClientWidth, m_spSwapChainAndRtvDsvHeap->m_nWndClientHeight, 0.0f, 1.0f);
	m_pCamera->SetScissorRect(0, 0, m_spSwapChainAndRtvDsvHeap->m_nWndClientWidth, m_spSwapChainAndRtvDsvHeap->m_nWndClientHeight);
	m_pCamera->GenerateProjectionMatrix(1.0f, 500.0f, float(m_spSwapChainAndRtvDsvHeap->m_nWndClientWidth) / float(m_spSwapChainAndRtvDsvHeap->m_nWndClientHeight), 90.0f);
	m_pCamera->GenerateViewMatrix(XMFLOAT3(0.0f, 15.0f, -25.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(0.0f, 1.0f, 0.0f));

	m_pScene = new CScene();
	m_pScene->BuildObjects(m_spDevice->GetDevice().Get(), m_spCommandQueue->GetCmdList().Get(),m_spRootSignature->GetGraphicsRootSignature());

	Util::ExecuteCommandList(m_spCommandQueue->GetCmdList(), m_spCommandQueue->GetCmdQueue(), m_spCommandQueue->GetFence(), ++m_spCommandQueue->m_nFenceValues[m_spSwapChainAndRtvDsvHeap->GetSwapChainIndex()], m_spCommandQueue->m_hFenceEvent);

	if (m_pScene) m_pScene->ReleaseUploadBuffers();
	m_GameTimer.Reset();
}
void Engine::ReleaseObjects()
{
	if (m_pScene)m_pScene->ReleaseObjects();
	if (m_pScene)delete m_pScene;
}


void Engine::ProcessInput()
{
}
void Engine::AnimateObjects()
{
	if (m_pScene)m_pScene->AnimateObjects(m_GameTimer.GetTimeElapsed());
}


void Engine::RenderBegin()
{
	m_spCommandQueue->RenderBegin();
}

void Engine::RenderEnd()
{
	m_spCommandQueue->RenderEnd();
}

void Engine::Render() {
	m_GameTimer.Tick(0.0f);

	ProcessInput();
	AnimateObjects();

	RenderBegin();
	//GetCommandQueue()->GetCmdList()->SetGraphicsRootSignature(m_spRootSignature->GetGraphicsRootSignature().Get());
	if (m_pScene)m_pScene->Render(m_spCommandQueue->GetCmdList().Get(), m_pCamera);
	
	//TODO


	RenderEnd();
	m_GameTimer.GetFrameRate(m_pszFrameRate + 12, 37);
	::SetWindowText(m_spSwapChainAndRtvDsvHeap->m_hWnd, m_pszFrameRate);
}


void Engine::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam,
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

void Engine::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM
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
			m_spCommandQueue->WaitForGpuComplete();
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

LRESULT CALLBACK Engine::OnProcessingWindowMessage(HWND hWnd, UINT nMessageID,
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


