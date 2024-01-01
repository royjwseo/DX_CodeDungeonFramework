#pragma once
#include "Timer.h"
#include "Scene.h"


class CGameFramework
{

private:
	CGameTimer m_GameTimer;
	_TCHAR m_pszFrameRate[50];
private:
	HINSTANCE m_hInstance;
	HWND m_hWnd;

	int m_nWndClientWidth; //swap chain �ʺ� �� �������
	int m_nWndClientHeight;
	//-----------------------------------------------------------------------------------------------------
	
	//IDXGIFactory4* m_pdxgiFactory;
	ComPtr< IDXGIFactory4> m_pdxgiFactory;
	
	ComPtr<IDXGISwapChain3> m_pdxgiSwapChain;
	//IDXGISwapChain3* m_pdxgiSwapChain; 
	
	ComPtr< ID3D12Device> m_pd3dDevice;

	// ComPtr�� m_p3dDevice -> .Get�� m_pd3dDevice�� �ǹ��ϰ� .GetAddressof�� &m_pd3dDevice�� �ǹ���.
	//ID3D12Device* m_pd3dDevice;
	
	static const UINT m_nSwapChainBuffers = 2;
	//���� ü���� �ĸ� ������ ����
	UINT m_nSwapChainBufferIndex;


	bool m_bMsaa4xEnable = false;
	UINT m_nMsaa4xQualityLevels = 0;
	//MSAA ���� ���ø��� Ȱ��ȭ�ϰ� ���� ���ø� ������ �����Ѵ�.

 //-----------------------------------------------------------------------------------------------------
 
	ID3D12Resource* m_ppd3dRenderTargetBuffers[m_nSwapChainBuffers];
	ID3D12DescriptorHeap* m_pd3dRtvDescriptorHeap;
	UINT m_nRtvDescriptorIncrementSize;

	ID3D12Resource* m_pd3dDepthStencilBuffer;
	ID3D12DescriptorHeap* m_pd3dDsvDescriptorHeap;
	UINT m_nDsvDescriptorIncrementSize;

	ID3D12CommandQueue* m_pd3dCommandQueue;
	ID3D12CommandAllocator* m_pd3dCommandAllocator; //�� �����ͷ� CommandList�� ��� �ִ� ��ɾ� �޸𸮸� ������ �� ���� �� �� �ִ�.
	//CreateCommandAllocator()�Լ��� ���� CommandAllocator�� �����.
	ID3D12GraphicsCommandList* m_pd3dCommandList; //�������� ���� �׷��� ��ɵ��� ����Ʈ�̴�.

	ID3D12PipelineState* m_pd3dPipelineState;
	//�׷��Ƚ� ���������� ���� ��ü�� ���� �������̽� �������̴�.
 //----------------------------------------------------------------------------------------------------------

	ID3D12Fence* m_pd3dFence;
	UINT64 m_nFenceValues[m_nSwapChainBuffers]; //(���� ������ 0 ~ 2^64 -1) ������ ��ȣ�̴�. �� �����Ӹ��� ���� �þ��.
	HANDLE m_hFenceEvent; // CreateEventEx(NULL, bool Event_name, bool Flags, EVENT_ALL_ACCESS)�� �Լ��� HANDLE�� ��ȯ

	CScene* m_pScene;

	public:
		CCamera* m_pCamera = NULL;
public:
	CGameFramework();
	~CGameFramework();

	void MoveToNextFrame();


	bool OnCreate(HINSTANCE hInstance, HWND hMainWnd);
	//�����ӿ�ũ�� �ʱ�ȭ�ϴ� �Լ��̴�.(�� �����찡 �����Ǹ� ȣ��ȴ�.)
	void OnDestroy();

	void ChangeSwapChainState();
	void CreateSwapChain(); //���� ü�� ���� �Լ�
	void CreateRtvAndDsvDescriptorHeaps(); //������ �� ���� �Լ�
	void CreateDirect3DDevice(); //����̽� ���� �Լ�
	void CreateCommandQueueAndList(); //��� ť/ �Ҵ��� / ����Ʈ ���� �Լ�

	void CreateRenderTargetViews(); //���� Ÿ�� �� ���� �Լ�
	void CreateDepthStencilView(); //����-���ٽ� �� ���� �Լ�

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
