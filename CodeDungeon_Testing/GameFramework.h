#pragma once
#include "stdafx.h"
#include "Timer.h"
#include "Scene.h"


class DeviceAndFactory {
public:
	
	void InitDeviceAndFactory();
	void CreateDirect3DDevice(); //디바이스 생성 함수

public:
	ComPtr<IDXGIFactory4> GetFactory() { return m_cpdxgiFactory; }
	//ComPtr<IDXGISwapChain3> GetSwapChain() { return m_cpdxgiSwapChain; }
	ComPtr<ID3D12Device> GetDevice() { return m_cpd3dDevice; }
	ComPtr< ID3D12Fence> GetFence() { return m_cpd3dFence; }

	bool GetMsaa4xEnable() { return m_bMsaa4xEnable; }
	uint16 GetMsaa4xQualityLevels() { return m_nMsaa4xQualityLevels; }

private:
	ComPtr< IDXGIFactory4> m_cpdxgiFactory;

	bool m_bMsaa4xEnable = false;
	uint16 m_nMsaa4xQualityLevels = 0;

	ComPtr< ID3D12Device> m_cpd3dDevice;
	
	ComPtr<ID3D12Fence> m_cpd3dFence;
public:
	UINT64 m_nFenceValues[SWAP_CHAIN_BUFFER_COUNT]; //(값의 범위가 0 ~ 2^64 -1) 프레임 번호이다. 매 프레임마다 값이 늘어난다.
	HANDLE m_hFenceEvent; // CreateEventEx(NULL, bool Event_name, bool Flags, EVENT_ALL_ACCESS)의 함수가 HANDLE값 반환


};

class SwapChainAndRtvDsvHeap {
public:
	void InitSwapChainAndRtvDsvHeap( ComPtr<IDXGIFactory4>& _Factory, ComPtr<ID3D12Device>& _Device, ComPtr< ID3D12CommandQueue>& _CmdQueue, bool MsaaEnable, uint16 MsaaQualityLevels);
	void ChangeSwapChainState( ComPtr<ID3D12Device>& _Device);
	void CreateSwapChain( ComPtr<IDXGIFactory4>& _Factory,  ComPtr<ID3D12Device>& _Device,  ComPtr< ID3D12CommandQueue>& _CmdQueue,bool MssaaEnable,uint16 MsaaQualityLevels); //스왑 체인 생성 함수
	void CreateRenderTargetViews( ComPtr<ID3D12Device>& _Device); //렌더 타겟 뷰 생성 함수
	void CreateDepthStencilView( ComPtr<ID3D12Device>& _Device, bool MsaaEnable, uint16 MsaaQualityLevels); //깊이-스텐실 뷰 생성 함수
	void CreateRtvAndDsvDescriptorHeaps( ComPtr<ID3D12Device>& _Device); //서술자 힙 생성 함수
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


class CommandQueue {
public:
	void CreateCommandQueueAndList(ComPtr<ID3D12Device>& _Device); //명령 큐/ 할당자 / 리스트 생성 함수

public:
	ComPtr<ID3D12CommandQueue>GetCmdQueue() { return m_pd3dCommandQueue; }
	ComPtr<ID3D12CommandAllocator>GetCmdAllocator() { return m_pd3dCommandAllocator; }
	ComPtr<ID3D12GraphicsCommandList>GetCmdList() { return m_pd3dCommandList; }
private:
	ComPtr<ID3D12CommandQueue> m_pd3dCommandQueue;
	ComPtr<ID3D12CommandAllocator> m_pd3dCommandAllocator;
	ComPtr<ID3D12GraphicsCommandList> m_pd3dCommandList;
};

class CGameFramework
{

private:
	CGameTimer m_GameTimer;
	_TCHAR m_pszFrameRate[50];
private:


	shared_ptr<DeviceAndFactory> m_spDevice;
	shared_ptr<SwapChainAndRtvDsvHeap> m_spSwapChainAndRtvDsvHeap;
	shared_ptr<CommandQueue> m_spCommandQueue;
	
	ComPtr<ID3D12PipelineState> m_pd3dPipelineState;
	
	
	CScene* m_pScene;

public:
	CCamera* m_pCamera = NULL;

	shared_ptr<DeviceAndFactory> GetDeviceAndFactory() { return m_spDevice; }
	shared_ptr<SwapChainAndRtvDsvHeap> GetSwapChainAndRtvDsvHeap() { return m_spSwapChainAndRtvDsvHeap; }
	shared_ptr<CommandQueue> GetCommandQueue() { return m_spCommandQueue; }
public:
	CGameFramework();
	~CGameFramework();

	void MoveToNextFrame();


	bool OnCreate(HINSTANCE hInstance, HWND hMainWnd);
	//프레임워크를 초기화하는 함수이다.(주 윈도우가 생성되면 호출된다.)
	void OnDestroy();

	void BuildObjects();
	void ReleaseObjects();
	//렌더링할 메쉬와 게임 객체를 생성하고 소멸하는 함수

	void ProcessInput(); //사용자 입력 구성 함수
	void AnimateObjects(); //애니메이션 구성 함수
	void FrameAdvance(); // 렌더링 구성 함수

	void WaitForGpuComplete(); //CPU와 GPU를 동기화하는 함수

	void OnProcessingMouseMessage(HWND hwnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	//윈도우 메세지 마우스 입력 처리 함수
	void OnProcessingKeyboardMessage(HWND hwnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	//윈도우 메세지 키보드 입력 처리 함수
	LRESULT CALLBACK OnProcessingWindowMessage(HWND hwnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
};


