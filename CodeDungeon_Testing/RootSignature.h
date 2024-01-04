#pragma once

class RootSignature {
	
public:
	void InitRootSignature( ComPtr<ID3D12Device>& _Device);

	ComPtr<ID3D12RootSignature>	GetGraphicsRootSignature() { return m_pd3dGraphicsRootSignature; }
private:
	void CreateGraphicsRootSignature(ComPtr<ID3D12Device>& _Device);

	ComPtr<ID3D12RootSignature> m_pd3dGraphicsRootSignature = nullptr;
};