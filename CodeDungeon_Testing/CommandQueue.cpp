#include "stdafx.h"
#include "CommandQueue.h"
#include "SwapChainAndRtvDsvHeap.h"
#include "Engine.h"


void CommandQueue::CreateCommandQueueAndList(const ComPtr<ID3D12Device>& _Device,shared_ptr<SwapChainAndRtvDsvHeap> _SwapChain) {

	m_spSwapChainAndRtvDsvHeap = _SwapChain;

	D3D12_COMMAND_QUEUE_DESC d3dCommandQueueDesc{};
	d3dCommandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	d3dCommandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	DX::ThrowIfFailed(_Device->CreateCommandQueue(&d3dCommandQueueDesc, IID_PPV_ARGS(&m_pd3dCommandQueue)));
	
	DX::ThrowIfFailed(_Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_pd3dCommandAllocator)));
	
	DX::ThrowIfFailed(_Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_pd3dCommandAllocator.Get(), NULL, IID_PPV_ARGS(&m_pd3dCommandList)));
	
	DX::ThrowIfFailed(m_pd3dCommandList->Close());

	DX::ThrowIfFailed(_Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_cpd3dFence)));

	m_hFenceEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
}

void CommandQueue::RenderBegin() {//여기서 DescriptorHeap Set해도됨.
	DX::ThrowIfFailed(m_pd3dCommandAllocator->Reset());
	DX::ThrowIfFailed(m_pd3dCommandList->Reset(m_pd3dCommandAllocator.Get(), NULL));

	Util::SynchronizeResourceTransition(m_pd3dCommandList, m_spSwapChainAndRtvDsvHeap->GetRenderTarget(m_spSwapChainAndRtvDsvHeap->GetSwapChain()->GetCurrentBackBufferIndex()).Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);


	D3D12_CPU_DESCRIPTOR_HANDLE d3dRtvCPUDescriptorHandle = m_spSwapChainAndRtvDsvHeap->GetRTVHeap()->GetCPUDescriptorHandleForHeapStart();
	d3dRtvCPUDescriptorHandle.ptr += (m_spSwapChainAndRtvDsvHeap->GetSwapChain()->GetCurrentBackBufferIndex() * m_spSwapChainAndRtvDsvHeap->GetRTVIncrementSize());

	float pfClearColor[4] = { 0.0f, 0.125f, 0.3f, 1.0f };
	m_pd3dCommandList->ClearRenderTargetView(d3dRtvCPUDescriptorHandle, pfClearColor, 0, NULL);

	D3D12_CPU_DESCRIPTOR_HANDLE d3dDsvCPUDescriptorHandle = m_spSwapChainAndRtvDsvHeap->GetDSVHeap()->GetCPUDescriptorHandleForHeapStart();
	
	m_pd3dCommandList->ClearDepthStencilView(d3dDsvCPUDescriptorHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, NULL);
	
	m_pd3dCommandList->OMSetRenderTargets(1, &d3dRtvCPUDescriptorHandle, TRUE, &d3dDsvCPUDescriptorHandle);

	CMDLIST->SetGraphicsRootSignature(GRAPHICS_ROOT_SIGNATURE.Get());

	
	//여기서 Viewport, ScissorRect 설정해도되지만, 카메라의 정보에 넣어 설정해도됨.그게 나은듯.
}

void CommandQueue::RenderEnd() {
	Util::SynchronizeResourceTransition(m_pd3dCommandList, m_spSwapChainAndRtvDsvHeap->GetRenderTarget(m_spSwapChainAndRtvDsvHeap->GetSwapChain()->GetCurrentBackBufferIndex()).Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);

	Util::ExecuteCommandList(m_pd3dCommandList, m_pd3dCommandQueue, m_cpd3dFence, ++m_nFenceValues[m_spSwapChainAndRtvDsvHeap->GetSwapChainIndex()],m_hFenceEvent);


	m_spSwapChainAndRtvDsvHeap->GetSwapChain()->Present(0, 0);
	MoveToNextFrame();


}


void CommandQueue::WaitForGpuComplete() {

	UINT64 nFenceValue = m_nFenceValues[m_spSwapChainAndRtvDsvHeap->GetSwapChainIndex()];
	DX::ThrowIfFailed(m_pd3dCommandQueue->Signal(m_cpd3dFence.Get(), nFenceValue));
	if (m_cpd3dFence.Get()->GetCompletedValue() < nFenceValue) {
		DX::ThrowIfFailed(m_cpd3dFence.Get()->SetEventOnCompletion(nFenceValue,m_hFenceEvent));
		WaitForSingleObjectEx(m_hFenceEvent, INFINITE, FALSE);
	}

}

void CommandQueue::MoveToNextFrame()
{
	m_spSwapChainAndRtvDsvHeap->SetSwapChainIndex(m_spSwapChainAndRtvDsvHeap->GetSwapChain()->GetCurrentBackBufferIndex());

	UINT64 nFenceValue = ++m_nFenceValues[m_spSwapChainAndRtvDsvHeap->GetSwapChainIndex()];
	DX::ThrowIfFailed(m_pd3dCommandQueue->Signal(m_cpd3dFence.Get(), nFenceValue));
	if (m_cpd3dFence.Get()->GetCompletedValue() < nFenceValue) {
		DX::ThrowIfFailed(m_cpd3dFence.Get()->SetEventOnCompletion(nFenceValue, m_hFenceEvent));
		::WaitForSingleObjectEx(m_hFenceEvent, INFINITE, FALSE);
	}
}