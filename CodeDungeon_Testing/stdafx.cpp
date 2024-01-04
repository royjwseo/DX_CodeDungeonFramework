#include "stdafx.h"

/*
m_pd3dVertexBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, pVertices, m_nStride* m_nVertices, D3D12_HEAP_TYPE_DEFAULT,
        D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, & m_pd3dVertexUploadBuffer);
        */

//ComPtr<ID3D12Resource> CreateBufferResource(const ComPtr<ID3D12Device>& device, const ComPtr<ID3D12GraphicsCommandList>& commandList,
//	const void* data, UINT sizePerData, UINT dataCount, D3D12_HEAP_TYPE heapType, D3D12_RESOURCE_STATES resourceState, ID3D12Resource** uploadBuffer)
//{
//	ComPtr<ID3D12Resource> buffer;
//	const UINT bufferSize{ sizePerData * dataCount };
//
//	// 디폴트 버퍼에 데이터를 넣을 경우 업로드 버퍼가 필요함
//	if (heapType == D3D12_HEAP_TYPE_DEFAULT)
//	{
//		// 디폴트 버퍼 생성
//		DX::ThrowIfFailed(device->CreateCommittedResource(
//			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
//			D3D12_HEAP_FLAG_NONE,
//			&CD3DX12_RESOURCE_DESC::Buffer(bufferSize),
//			D3D12_RESOURCE_STATE_COPY_DEST,
//			NULL,
//			IID_PPV_ARGS(&buffer)));
//
//		// 데이터 복사
//		if (data)
//		{
//			// 업로드 버퍼 생성
//			DX::ThrowIfFailed(device->CreateCommittedResource(
//				&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
//				D3D12_HEAP_FLAG_NONE,
//				&CD3DX12_RESOURCE_DESC::Buffer(bufferSize),
//				D3D12_RESOURCE_STATE_GENERIC_READ,
//				NULL,
//				IID_PPV_ARGS(uploadBuffer)));
//
//			// 업로드 버퍼에서 디폴트 버퍼로 복사
//			D3D12_SUBRESOURCE_DATA bufferData{};
//			bufferData.pData = data;
//			bufferData.RowPitch = bufferSize;
//			bufferData.SlicePitch = bufferData.RowPitch;
//			UpdateSubresources<1>(commandList.Get(), buffer.Get(), *uploadBuffer, 0, 0, 1, &bufferData);
//		}
//
//		// 버퍼 리소스 베리어 설정
//		commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(buffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, resourceState));
//		return buffer;
//	}
//
//	// 업로드 버퍼에 데이터를 넣을 경우 바로 복사함
//	if (heapType == D3D12_HEAP_TYPE_UPLOAD)
//	{
//		// 업로드 버퍼 생성
//		DX::ThrowIfFailed(device->CreateCommittedResource(
//			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
//			D3D12_HEAP_FLAG_NONE,
//			&CD3DX12_RESOURCE_DESC::Buffer(bufferSize),
//			D3D12_RESOURCE_STATE_GENERIC_READ,
//			NULL,
//			IID_PPV_ARGS(&buffer)));
//
//		// 데이터 복사
//		if (data)
//		{
//			UINT8* pBufferDataBegin{ NULL };
//			CD3DX12_RANGE readRange{ 0, 0 };
//			DX::ThrowIfFailed(buffer->Map(0, &readRange, reinterpret_cast<void**>(&pBufferDataBegin)));
//			memcpy(pBufferDataBegin, data, bufferSize);
//			buffer->Unmap(0, NULL);
//		}
//
//		return buffer;
//	}
//
//	if (heapType == D3D12_HEAP_TYPE_READBACK)
//	{
//		// 리드백 버퍼 생성
//		DX::ThrowIfFailed(device->CreateCommittedResource(
//			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_READBACK),
//			D3D12_HEAP_FLAG_NONE,
//			&CD3DX12_RESOURCE_DESC::Buffer(bufferSize),
//			D3D12_RESOURCE_STATE_COPY_DEST,
//			NULL,
//			IID_PPV_ARGS(&buffer)));
//
//		// 데이터 복사
//		if (data)
//		{
//			UINT8* pBufferDataBegin{ NULL };
//			CD3DX12_RANGE readRange{ 0, 0 };
//			DX::ThrowIfFailed(buffer->Map(0, &readRange, reinterpret_cast<void**>(&pBufferDataBegin)));
//			memcpy(pBufferDataBegin, data, bufferSize);
//			buffer->Unmap(0, NULL);
//		}
//		return buffer;
//	}
//	return NULL;
//}

ComPtr<ID3D12Resource> CreateBufferResource(const ComPtr<ID3D12Device>& _Device, const ComPtr<ID3D12GraphicsCommandList>& _CommandList, void* pData, UINT nBytes, D3D12_HEAP_TYPE d3dHeapType, D3D12_RESOURCE_STATES d3dResourceStates, ID3D12Resource** ppd3dUploadBuffer)
{
    ComPtr<ID3D12Resource> pd3dBuffer = NULL;

    D3D12_HEAP_PROPERTIES d3dHeapPropertiesDesc;
    ::ZeroMemory(&d3dHeapPropertiesDesc, sizeof(D3D12_HEAP_PROPERTIES));
    d3dHeapPropertiesDesc.Type = d3dHeapType;
    d3dHeapPropertiesDesc.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    d3dHeapPropertiesDesc.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    d3dHeapPropertiesDesc.CreationNodeMask = 1;
    d3dHeapPropertiesDesc.VisibleNodeMask = 1;

    D3D12_RESOURCE_DESC d3dResourceDesc;
    ::ZeroMemory(&d3dResourceDesc, sizeof(D3D12_RESOURCE_DESC));
    d3dResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    d3dResourceDesc.Alignment = 0;
    d3dResourceDesc.Width = nBytes;
    d3dResourceDesc.Height = 1;
    d3dResourceDesc.DepthOrArraySize = 1;
    d3dResourceDesc.MipLevels = 1;
    d3dResourceDesc.Format = DXGI_FORMAT_UNKNOWN;
    d3dResourceDesc.SampleDesc.Count = 1;
    d3dResourceDesc.SampleDesc.Quality = 0;
    d3dResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    d3dResourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

    D3D12_RESOURCE_STATES d3dResourceInitialStates = D3D12_RESOURCE_STATE_COPY_DEST;
    if (d3dHeapType == D3D12_HEAP_TYPE_UPLOAD) d3dResourceInitialStates =
        D3D12_RESOURCE_STATE_GENERIC_READ;
    else if (d3dHeapType == D3D12_HEAP_TYPE_READBACK) d3dResourceInitialStates =
        D3D12_RESOURCE_STATE_COPY_DEST;
    HRESULT hResult = _Device->CreateCommittedResource(&d3dHeapPropertiesDesc,
        D3D12_HEAP_FLAG_NONE, &d3dResourceDesc, d3dResourceInitialStates, NULL,
        IID_PPV_ARGS(&pd3dBuffer));
    if (pData)
    {
        switch (d3dHeapType)
        {
        case D3D12_HEAP_TYPE_DEFAULT:
        {
            if (ppd3dUploadBuffer)
            {
                //업로드 버퍼를 생성한다. 
                d3dHeapPropertiesDesc.Type = D3D12_HEAP_TYPE_UPLOAD;
				_Device->CreateCommittedResource(&d3dHeapPropertiesDesc,
                    D3D12_HEAP_FLAG_NONE, &d3dResourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, NULL,
                    IID_PPV_ARGS(ppd3dUploadBuffer));
                //업로드 버퍼를 매핑하여 초기화 데이터를 업로드 버퍼에 복사한다. 
                D3D12_RANGE d3dReadRange = { 0, 0 };
                UINT8* pBufferDataBegin = NULL;
                (*ppd3dUploadBuffer)->Map(0, &d3dReadRange, reinterpret_cast<void**>(&pBufferDataBegin));
                memcpy(pBufferDataBegin, pData, nBytes);
                (*ppd3dUploadBuffer)->Unmap(0, NULL);
                //업로드 버퍼의 내용을 디폴트 버퍼에 복사한다. 
				_CommandList->CopyResource(pd3dBuffer.Get(), *ppd3dUploadBuffer);
                D3D12_RESOURCE_BARRIER d3dResourceBarrier;
                ::ZeroMemory(&d3dResourceBarrier, sizeof(D3D12_RESOURCE_BARRIER));
                d3dResourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
                d3dResourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
                d3dResourceBarrier.Transition.pResource = pd3dBuffer.Get();
                d3dResourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
                d3dResourceBarrier.Transition.StateAfter = d3dResourceStates;//D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER
                d3dResourceBarrier.Transition.Subresource =
                    D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
				_CommandList->ResourceBarrier(1, &d3dResourceBarrier);
            }
            break;
        }
        case D3D12_HEAP_TYPE_UPLOAD:
        {
            D3D12_RANGE d3dReadRange = { 0, 0 };
            UINT8* pBufferDataBegin = NULL;
            pd3dBuffer->Map(0, &d3dReadRange, reinterpret_cast<void**>(&pBufferDataBegin));
            memcpy(pBufferDataBegin, pData, nBytes);
            pd3dBuffer->Unmap(0, NULL);
            break;
        }
        case D3D12_HEAP_TYPE_READBACK:
            break;
        }
    }
    return(pd3dBuffer);
}
