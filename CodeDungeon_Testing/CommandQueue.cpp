#include "stdafx.h"
#include "CommandQueue.h"


void CommandQueue::CreateCommandQueueAndList(ComPtr<ID3D12Device>& _Device) {

	D3D12_COMMAND_QUEUE_DESC d3dCommandQueueDesc{};
	::ZeroMemory(&d3dCommandQueueDesc, sizeof(D3D12_COMMAND_QUEUE_DESC));
	d3dCommandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;//�⺻ ��� ť�� ����
	d3dCommandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;//�츮�� GPU�� ���� ���� �� �� �ִ� CommandList�� ���� CommandQueue�� ����� ���� GPU�� ���� ������ �� �ִ� ��� ���� ����Ʈ
	DX::ThrowIfFailed(_Device->CreateCommandQueue(&d3dCommandQueueDesc, IID_PPV_ARGS(&m_pd3dCommandQueue)));
	//����(direct) ��� ť�� ����

	DX::ThrowIfFailed(_Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_pd3dCommandAllocator)));
	//����(direct) ��� �Ҵ��ڸ� �����Ѵ�.

	DX::ThrowIfFailed(_Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_pd3dCommandAllocator.Get(), NULL, IID_PPV_ARGS(&m_pd3dCommandList)));
	//����(dircect) ��� ����Ʈ�� �����Ѵ�.                                        //������ ��� �Ҵ���  //�ʱ� ���������� ���� ��ü

	DX::ThrowIfFailed(m_pd3dCommandList->Close());
	//��ɸ���Ʈ�� �ʱ���°� ���������̹Ƿ� ���� ���·� �����.

}