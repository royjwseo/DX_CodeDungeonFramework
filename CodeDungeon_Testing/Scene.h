#pragma once
#include "Timer.h"
#include "Shader.h"



class CScene
{
public:
	CScene();
	~CScene();
	//씬에서 마우스와 키보드 메시지를 처리한다. 
	bool OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM
		lParam);
	bool OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM
		lParam);
	void BuildObjects(const ComPtr<ID3D12Device>& _Device, const ComPtr<ID3D12GraphicsCommandList>& _CommandList);
	void ReleaseObjects();
	bool ProcessInput(UCHAR* pKeysBuffer);
	void AnimateObjects(float fTimeElapsed);
	void Render(const ComPtr<ID3D12GraphicsCommandList>& _CommandList, CCamera* pCamera);
	void ReleaseUploadBuffers();
	//그래픽 루트 시그너쳐를 생성한다. 
	ComPtr<ID3D12RootSignature> CreateGraphicsRootSignature(const ComPtr<ID3D12Device>& _Device);
	ComPtr<ID3D12RootSignature> GetGraphicsRootSignature();
protected:
	//씬은 셰이더들의 집합이다. 셰이더들은 게임 객체들의 집합이다.
	CGameObject** m_ppObjects = NULL;
	int m_nObjects = 0;
	ComPtr<ID3D12RootSignature> m_pd3dGraphicsRootSignature = NULL;
};