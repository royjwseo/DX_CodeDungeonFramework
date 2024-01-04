#pragma once

class SwapChainAndRtvDsvHeap;

class CommandQueue {
public:
	void CreateCommandQueueAndList(const ComPtr<ID3D12Device>& _Device, shared_ptr<SwapChainAndRtvDsvHeap> _SwapChain); //��� ť/ �Ҵ��� / ����Ʈ ���� �Լ�
	void RenderBegin();
	void RenderEnd();
public:
	void WaitForGpuComplete();
	void MoveToNextFrame();
public:
	ComPtr<ID3D12CommandQueue>GetCmdQueue() { return m_pd3dCommandQueue; }
	ComPtr<ID3D12CommandAllocator>GetCmdAllocator() { return m_pd3dCommandAllocator; }
	ComPtr<ID3D12GraphicsCommandList>GetCmdList() { return m_pd3dCommandList; }
	ComPtr< ID3D12Fence> GetFence() { return m_cpd3dFence; }
private:
	ComPtr<ID3D12CommandQueue> m_pd3dCommandQueue;
	ComPtr<ID3D12CommandAllocator> m_pd3dCommandAllocator;
	ComPtr<ID3D12GraphicsCommandList> m_pd3dCommandList;

	shared_ptr<SwapChainAndRtvDsvHeap> m_spSwapChainAndRtvDsvHeap;

	ComPtr<ID3D12Fence> m_cpd3dFence;

public:
	UINT64 m_nFenceValues[SWAP_CHAIN_BUFFER_COUNT]; 
	HANDLE m_hFenceEvent; 

};

