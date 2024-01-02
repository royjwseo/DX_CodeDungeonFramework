#pragma once
#include "stdafx.h"
#include "Timer.h"
#include "Scene.h"


class DeviceAndFactory {
public:
	
	void InitDeviceAndFactory();
	void CreateDirect3DDevice(); //����̽� ���� �Լ�

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
	UINT64 m_nFenceValues[SWAP_CHAIN_BUFFER_COUNT]; //(���� ������ 0 ~ 2^64 -1) ������ ��ȣ�̴�. �� �����Ӹ��� ���� �þ��.
	HANDLE m_hFenceEvent; // CreateEventEx(NULL, bool Event_name, bool Flags, EVENT_ALL_ACCESS)�� �Լ��� HANDLE�� ��ȯ


};

class SwapChainAndRtvDsvHeap {
public:
	void InitSwapChainAndRtvDsvHeap( ComPtr<IDXGIFactory4>& _Factory, ComPtr<ID3D12Device>& _Device, ComPtr< ID3D12CommandQueue>& _CmdQueue, bool MsaaEnable, uint16 MsaaQualityLevels);
	void ChangeSwapChainState( ComPtr<ID3D12Device>& _Device);
	void CreateSwapChain( ComPtr<IDXGIFactory4>& _Factory,  ComPtr<ID3D12Device>& _Device,  ComPtr< ID3D12CommandQueue>& _CmdQueue,bool MssaaEnable,uint16 MsaaQualityLevels); //���� ü�� ���� �Լ�
	void CreateRenderTargetViews( ComPtr<ID3D12Device>& _Device); //���� Ÿ�� �� ���� �Լ�
	void CreateDepthStencilView( ComPtr<ID3D12Device>& _Device, bool MsaaEnable, uint16 MsaaQualityLevels); //����-���ٽ� �� ���� �Լ�
	void CreateRtvAndDsvDescriptorHeaps( ComPtr<ID3D12Device>& _Device); //������ �� ���� �Լ�
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

	int m_nWndClientWidth; //swap chain �ʺ� �� �������
	int m_nWndClientHeight;
};


class CommandQueue {
public:
	void CreateCommandQueueAndList(ComPtr<ID3D12Device>& _Device); //��� ť/ �Ҵ��� / ����Ʈ ���� �Լ�

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
	//�����ӿ�ũ�� �ʱ�ȭ�ϴ� �Լ��̴�.(�� �����찡 �����Ǹ� ȣ��ȴ�.)
	void OnDestroy();

	void BuildObjects();
	void ReleaseObjects();
	//�������� �޽��� ���� ��ü�� �����ϰ� �Ҹ��ϴ� �Լ�

	void ProcessInput(); //����� �Է� ���� �Լ�
	void AnimateObjects(); //�ִϸ��̼� ���� �Լ�
	void FrameAdvance(); // ������ ���� �Լ�

	void WaitForGpuComplete(); //CPU�� GPU�� ����ȭ�ϴ� �Լ�

	void OnProcessingMouseMessage(HWND hwnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	//������ �޼��� ���콺 �Է� ó�� �Լ�
	void OnProcessingKeyboardMessage(HWND hwnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	//������ �޼��� Ű���� �Է� ó�� �Լ�
	LRESULT CALLBACK OnProcessingWindowMessage(HWND hwnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
};


