#pragma once
#include "stdafx.h"
#include "Timer.h"
#include "Scene.h"


class DeviceAndFactory;
class SwapChainAndRtvDsvHeap;
class CommandQueue;



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


