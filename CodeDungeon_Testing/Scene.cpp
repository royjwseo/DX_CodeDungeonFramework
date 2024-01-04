#include "stdafx.h"
#include "Scene.h"
#include "Engine.h"


CScene::CScene()
{
}

CScene::~CScene()
{
}

bool CScene::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	return false;
}

bool CScene::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	return false;
}






void CScene::BuildObjects(const ComPtr<ID3D12Device>& _Device, const ComPtr<ID3D12GraphicsCommandList>& _CommandList,const ComPtr<ID3D12RootSignature>& _RootSignature)
{
	
	//����x����x���̰� 12x12x12�� ������ü �޽��� �����Ѵ�. 
	CCubeMeshDiffused* pCubeMesh = new CCubeMeshDiffused(_Device, _CommandList,
		12.0f, 12.0f, 12.0f);
	m_nObjects = 1;
	m_ppObjects = new CGameObject * [m_nObjects];
	CRotatingObject* pRotatingObject = new CRotatingObject();
	pRotatingObject->SetMesh(pCubeMesh);
	CDiffusedShader* pShader = new CDiffusedShader();
	pShader->CreateShader(L"Shaders.hlsl","VSDiffused","PSDiffused",_Device, _RootSignature );
	pShader->CreateShaderVariables(_Device, _CommandList);
	pRotatingObject->SetShader(pShader);
	m_ppObjects[0] = pRotatingObject;
}

void CScene::ReleaseObjects()
{
	
	if (m_ppObjects)
	{
		for (int j = 0; j < m_nObjects; j++) if (m_ppObjects[j]) delete m_ppObjects[j];
		delete[] m_ppObjects;
	}
}

void CScene::ReleaseUploadBuffers()
{
	if (m_ppObjects)
	{
		for (int j = 0; j < m_nObjects; j++) if (m_ppObjects[j])
			m_ppObjects[j]->ReleaseUploadBuffers();
	}
}






void CScene::AnimateObjects(float fTimeElapsed)
{
	for (int j = 0; j < m_nObjects; j++)
	{
		m_ppObjects[j]->Animate(fTimeElapsed);
	}
}


void CScene::Render(const ComPtr<ID3D12GraphicsCommandList>& _CommandList, CCamera* pCamera)
{
	pCamera->SetViewportsAndScissorRects(_CommandList);
	if (pCamera) pCamera->UpdateShaderVariables(_CommandList);
	//���� �������ϴ� ���� ���� �����ϴ� ���� ��ü(���̴��� �����ϴ� ��ü)���� �������ϴ� ���̴�.
	for (int j = 0; j < m_nObjects; j++)
	{
		if (m_ppObjects[j]) m_ppObjects[j]->Render(_CommandList, pCamera);
	}
}

