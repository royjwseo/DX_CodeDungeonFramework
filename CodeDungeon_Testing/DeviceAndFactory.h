#pragma once
class DeviceAndFactory {
public:

	void InitDeviceAndFactory();
	void CreateDxgiFactory();
	void CreateDirect3DDevice(); //����̽� ���� �Լ�

public:
	ComPtr<IDXGIFactory4> GetFactory() { return m_cpdxgiFactory; }
	//ComPtr<IDXGISwapChain3> GetSwapChain() { return m_cpdxgiSwapChain; }
	ComPtr<ID3D12Device> GetDevice() { return m_cpd3dDevice; }
	ComPtr< ID3D12Fence> GetFence() { return m_cpd3dFence; }

	bool GetMsaa4xEnable() { return m_bMsaa4xEnable; }
	uint16 GetMsaa4xQualityLevels() { return m_nMsaa4xQualityLevels; }

private:
	ComPtr< IDXGIFactory4> m_cpdxgiFactory;

	bool m_bMsaa4xEnable = false;
	uint16 m_nMsaa4xQualityLevels = 0;

	ComPtr< ID3D12Device> m_cpd3dDevice;

	ComPtr<ID3D12Fence> m_cpd3dFence;
public:
	UINT64 m_nFenceValues[SWAP_CHAIN_BUFFER_COUNT]; //(���� ������ 0 ~ 2^64 -1) ������ ��ȣ�̴�. �� �����Ӹ��� ���� �þ��.
	HANDLE m_hFenceEvent; // CreateEventEx(NULL, bool Event_name, bool Flags, EVENT_ALL_ACCESS)�� �Լ��� HANDLE�� ��ȯ


};