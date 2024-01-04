#include "stdafx.h"
#include "DeviceAndFactory.h"

void DeviceAndFactory::InitDeviceAndFactory()
{
	CreateDxgiFactory();
	CreateDirect3DDevice();
}

void DeviceAndFactory::CreateDxgiFactory()
{
	UINT nDXGIFactoryFlags = 0;
#if defined(_DEBUG) //전처리기 조건문 #if있으면 #endif 는 무조건 세트, D3D12 디버그층 활성화

	ComPtr<ID3D12Debug> debugController;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
	{
		debugController->EnableDebugLayer();
		nDXGIFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
	}
#endif
	DX::ThrowIfFailed(CreateDXGIFactory2(nDXGIFactoryFlags, IID_PPV_ARGS(&m_cpdxgiFactory)));//(IID_PPV_ARGS(&m_pdxgiFactory)
}

void DeviceAndFactory::CreateDirect3DDevice()
{

	ComPtr<IDXGIAdapter1> pd3dAdapter = nullptr;


	for (UINT i = 0; m_cpdxgiFactory->EnumAdapters1(i, &pd3dAdapter) != DXGI_ERROR_NOT_FOUND; ++i) {
		DXGI_ADAPTER_DESC1 dxgiAdapterDesc;
		pd3dAdapter->GetDesc1(&dxgiAdapterDesc);

		if (dxgiAdapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
			continue;

		if (SUCCEEDED(D3D12CreateDevice(pd3dAdapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&m_cpd3dDevice))))
			break;
	}

	if (!pd3dAdapter) {
		m_cpdxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&pd3dAdapter));
		D3D12CreateDevice(pd3dAdapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&m_cpd3dDevice));
	}

	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS d3dMsaaQualityLevels;
	d3dMsaaQualityLevels.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	d3dMsaaQualityLevels.SampleCount = 4; //Msaa4x 다중 샘플링
	d3dMsaaQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;//특별한 기능 x 멀티 스레드 사용시 SHARED사용할수도
	d3dMsaaQualityLevels.NumQualityLevels = 0;
	DX::ThrowIfFailed(m_cpd3dDevice->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &d3dMsaaQualityLevels, sizeof(D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS)));
	m_nMsaa4xQualityLevels = d3dMsaaQualityLevels.NumQualityLevels;
	//디바이스가 지원하는 다중 샘플의 품질 수준을 확인한다. 
	m_bMsaa4xEnable = (m_nMsaa4xQualityLevels > 1) ? true : false;
	//다중 샘플의 품질 수준이 1보다 크면 다중 샘플링을 활성화한다. 

	



}
//-----------------------------------------
