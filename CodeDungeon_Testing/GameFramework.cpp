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


	DX::ThrowIfFailed(m_spCommandQueue->GetCmdList()->Close());
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

