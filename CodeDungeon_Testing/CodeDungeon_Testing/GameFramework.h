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

	int m_nWndClientWidth; //swap chain 너비 등 저장목적
	int m_nWndClientHeight;
	//-----------------------------------------------------------------------------------------------------
	
	//IDXGIFactory4* m_pdxgiFactory;
	ComPtr< IDXGIFactory4> m_pdxgiFactory;
	
	ComPtr<IDXGISwapChain3> m_pdxgiSwapChain;
	//IDXGISwapChain3* m_pdxgiSwapChain; 
	
	ComPtr< ID3D12Device> m_pd3dDevice;

	// ComPtr은 m_p3dDevice -> .Get은 m_pd3dDevice를 의미하고 .GetAddressof는 &m_pd3dDevice를 의미함.
	//ID3D12Device* m_pd3dDevice;
	
	static const UINT m_nSwapChainBuffers = 2;
	//스왑 체인의 후면 버퍼의 개수
	UINT m_nSwapChainBufferIndex;


	bool m_bMsaa4xEnable = false;
	UINT m_nMsaa4xQualityLevels = 0;
	//MSAA 다중 샘플링을 활성화하고 다중 샘플링 레벨을 설정한다.

 //-----------------------------------------------------------------------------------------------------
 
	ID3D12Resource* m_ppd3dRenderTargetBuffers[m_nSwapChainBuffers];
	ID3D12DescriptorHeap* m_pd3dRtvDescriptorHeap;
	UINT m_nRtvDescriptorIncrementSize;

	ID3D12Resource* m_pd3dDepthStencilBuffer;
	ID3D12DescriptorHeap* m_pd3dDsvDescriptorHeap;
	UINT m_nDsvDescriptorIncrementSize;

	ID3D12CommandQueue* m_pd3dCommandQueue;
	ID3D12CommandAllocator* m_pd3dCommandAllocator; //이 포인터로 CommandList가 어디에 있는 명령어 메모리를 가져다 쓸 건지 알 수 있다.
	//CreateCommandAllocator()함수를 통해 CommandAllocator를 만든다.
	ID3D12GraphicsCommandList* m_pd3dCommandList; //렌더링을 위한 그래픽 명령들의 리스트이다.

	ID3D12PipelineState* m_pd3dPipelineState;
	//그래픽스 파이프라인 상태 객체에 대한 인터페이스 포인터이다.
 //----------------------------------------------------------------------------------------------------------

	ID3D12Fence* m_pd3dFence;
	UINT64 m_nFenceValues[m_nSwapChainBuffers]; //(값의 범위가 0 ~ 2^64 -1) 프레임 번호이다. 매 프레임마다 값이 늘어난다.
	HANDLE m_hFenceEvent; // CreateEventEx(NULL, bool Event_name, bool Flags, EVENT_ALL_ACCESS)의 함수가 HANDLE값 반환

	CScene* m_pScene;

	public:
		CCamera* m_pCamera = NULL;
public:
	CGameFramework();
	~CGameFramework();

	void MoveToNextFrame();


	bool OnCreate(HINSTANCE hInstance, HWND hMainWnd);
	//프레임워크를 초기화하는 함수이다.(주 윈도우가 생성되면 호출된다.)
	void OnDestroy();

	void ChangeSwapChainState();
	void CreateSwapChain(); //스왑 체인 생성 함수
	void CreateRtvAndDsvDescriptorHeaps(); //서술자 힙 생성 함수
	void CreateDirect3DDevice(); //디바이스 생성 함수
	void CreateCommandQueueAndList(); //명령 큐/ 할당자 / 리스트 생성 함수

	void CreateRenderTargetViews(); //렌더 타겟 뷰 생성 함수
	void CreateDepthStencilView(); //깊이-스텐실 뷰 생성 함수

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
