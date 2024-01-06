#pragma once
#include "stdafx.h"
#include "Timer.h"
#include "Scene.h"
#include "DeviceAndFactory.h"
#include "SwapChainAndRtvDsvHeap.h"
#include "CommandQueue.h"
#include "RootSignature.h"
class Engine
{
	
public:
	bool OnCreate(HINSTANCE hInstance, HWND hMainWnd);
	void OnDestroy();
public:
	shared_ptr<DeviceAndFactory> GetDeviceAndFactory() { return m_spDevice; }
	shared_ptr<SwapChainAndRtvDsvHeap> GetSwapChainAndRtvDsvHeap() { return m_spSwapChainAndRtvDsvHeap; }
	shared_ptr<CommandQueue> GetCommandQueue() { return m_spCommandQueue; }
	shared_ptr<RootSignature> GetRootSignature() { return m_spRootSignature; }
public:
	void RenderBegin();
	void RenderEnd();
	void Render();
public:
	void ProcessInput();   
	void AnimateObjects(); 
	void BuildObjects();
	void ReleaseObjects();

private:
	CGameTimer m_GameTimer;
	_TCHAR m_pszFrameRate[50];
	CCamera* m_pCamera = NULL;
private:
	CScene* m_pScene;
	ComPtr<ID3D12PipelineState> m_pd3dPipelineState;
	

private:
	shared_ptr<DeviceAndFactory> m_spDevice= make_shared<DeviceAndFactory>();
	shared_ptr<SwapChainAndRtvDsvHeap> m_spSwapChainAndRtvDsvHeap= make_shared<SwapChainAndRtvDsvHeap>();
	shared_ptr<CommandQueue> m_spCommandQueue = make_shared<CommandQueue>();
	shared_ptr<RootSignature> m_spRootSignature = make_shared<RootSignature>();
public:
	//INPUT ฐüทร
	void OnProcessingMouseMessage(HWND hwnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);

	void OnProcessingKeyboardMessage(HWND hwnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	
	LRESULT CALLBACK OnProcessingWindowMessage(HWND hwnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
};

