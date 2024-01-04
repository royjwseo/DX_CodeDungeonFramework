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
	void BuildObjects(const ComPtr<ID3D12Device>& _Device, const ComPtr<ID3D12GraphicsCommandList>& _CommandList,const ComPtr<ID3D12RootSignature>& _RootSignature);
	void ReleaseObjects();
	bool ProcessInput(UCHAR* pKeysBuffer);
	void AnimateObjects(float fTimeElapsed);
	void Render(const ComPtr<ID3D12GraphicsCommandList>& _CommandList, CCamera* pCamera);
	void ReleaseUploadBuffers();
	//�׷��� ��Ʈ �ñ׳��ĸ� �����Ѵ�. 
	
protected:
	//���� ���̴����� �����̴�. ���̴����� ���� ��ü���� �����̴�.
	CGameObject** m_ppObjects = NULL;
	int m_nObjects = 0;

};