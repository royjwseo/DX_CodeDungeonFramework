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


