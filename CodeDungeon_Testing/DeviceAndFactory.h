#pragma once
class DeviceAndFactory {
public:

	void InitDeviceAndFactory();
	void CreateDxgiFactory();
	void CreateDirect3DDevice(); //디바이스 생성 함수

public:
	ComPtr<IDXGIFactory4> GetFactory() { return m_cpdxgiFactory; }
	ComPtr<ID3D12Device> GetDevice() { return m_cpd3dDevice; }
	

	bool GetMsaa4xEnable() { return m_bMsaa4xEnable; }
	uint16 GetMsaa4xQualityLevels() { return m_nMsaa4xQualityLevels; }

private:
	ComPtr< IDXGIFactory4> m_cpdxgiFactory;

	bool m_bMsaa4xEnable = false;
	uint16 m_nMsaa4xQualityLevels = 0;

	ComPtr<ID3D12Device> m_cpd3dDevice;
	ComPtr<ID3D12Debug>			m_pd3dDebugController;


};