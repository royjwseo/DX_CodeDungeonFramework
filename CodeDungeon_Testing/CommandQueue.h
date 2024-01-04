#pragma once
class CommandQueue {
public:
	void CreateCommandQueueAndList(ComPtr<ID3D12Device>& _Device); //명령 큐/ 할당자 / 리스트 생성 함수

public:
	ComPtr<ID3D12CommandQueue>GetCmdQueue() { return m_pd3dCommandQueue; }
	ComPtr<ID3D12CommandAllocator>GetCmdAllocator() { return m_pd3dCommandAllocator; }
	ComPtr<ID3D12GraphicsCommandList>GetCmdList() { return m_pd3dCommandList; }
private:
	ComPtr<ID3D12CommandQueue> m_pd3dCommandQueue;
	ComPtr<ID3D12CommandAllocator> m_pd3dCommandAllocator;
	ComPtr<ID3D12GraphicsCommandList> m_pd3dCommandList;
};

