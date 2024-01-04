#pragma once
#include "Timer.h"
#include "Shader.h"



class CScene
{
public:
	CScene();
	~CScene();
	//������ ���콺�� Ű���� �޽����� ó���Ѵ�. 
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
	//�׷��� ��Ʈ �ñ׳��ĸ� �����Ѵ�. 
	ComPtr<ID3D12RootSignature> CreateGraphicsRootSignature(const ComPtr<ID3D12Device>& _Device);
	ComPtr<ID3D12RootSignature> GetGraphicsRootSignature();
protected:
	//���� ���̴����� �����̴�. ���̴����� ���� ��ü���� �����̴�.
	CGameObject** m_ppObjects = NULL;
	int m_nObjects = 0;
	ComPtr<ID3D12RootSignature> m_pd3dGraphicsRootSignature = NULL;
};