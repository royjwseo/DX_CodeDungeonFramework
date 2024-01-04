#include "stdafx.h"
#include "CommandQueue.h"


void CommandQueue::CreateCommandQueueAndList(ComPtr<ID3D12Device>& _Device) {

	D3D12_COMMAND_QUEUE_DESC d3dCommandQueueDesc{};
	::ZeroMemory(&d3dCommandQueueDesc, sizeof(D3D12_COMMAND_QUEUE_DESC));
	d3dCommandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;//기본 명령 큐로 정함
	d3dCommandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;//우리는 GPU가 직접 실행 할 수 있는 CommandList를 담은 CommandQueue를 만들기 위해 GPU가 직접 실행할 수 있는 명령 버퍼 리스트
	DX::ThrowIfFailed(_Device->CreateCommandQueue(&d3dCommandQueueDesc, IID_PPV_ARGS(&m_pd3dCommandQueue)));
	//직접(direct) 명령 큐를 생성

	DX::ThrowIfFailed(_Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_pd3dCommandAllocator)));
	//직접(direct) 명령 할당자를 생성한다.

	DX::ThrowIfFailed(_Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_pd3dCommandAllocator.Get(), NULL, IID_PPV_ARGS(&m_pd3dCommandList)));
	//직접(dircect) 명령 리스트를 생성한다.                                        //연관된 명령 할당자  //초기 파이프라인 상태 객체

	DX::ThrowIfFailed(m_pd3dCommandList->Close());
	//명령리스트는 초기상태가 열린상태이므로 닫힌 상태로 만든다.

}