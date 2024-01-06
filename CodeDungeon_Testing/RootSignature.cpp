#include "stdafx.h"
#include "RootSignature.h"
#include "Engine.h"

void RootSignature::InitRootSignature(ComPtr<ID3D12Device>& _Device) {
	CreateGraphicsRootSignature(_Device);
}

void RootSignature::CreateGraphicsRootSignature(ComPtr<ID3D12Device>& _Device)
{
	//¿¹½Ã
	//CD3DX12_DESCRIPTOR_RANGE ranges[4]{};
	//ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND);

	CD3DX12_ROOT_PARAMETER pd3dRootParameters[2]{};
	pd3dRootParameters[0].InitAsConstants(16, 0, 0, D3D12_SHADER_VISIBILITY_VERTEX);
	pd3dRootParameters[1].InitAsConstants(32, 1, 0, D3D12_SHADER_VISIBILITY_VERTEX);

	//CD3DX12_STATIC_SAMPLER_DESC samplerDesc[2]{};
	//samplerDesc[0].Init(
	//	0,								 				// ShaderRegister
	//	D3D12_FILTER_MIN_MAG_MIP_LINEAR, 				// filter
	//	D3D12_TEXTURE_ADDRESS_MODE_WRAP, 				// addressU
	//	D3D12_TEXTURE_ADDRESS_MODE_WRAP, 				// addressV
	//	D3D12_TEXTURE_ADDRESS_MODE_WRAP, 				// addressW
	//	0.0f,											// mipLODBias
	//	1,												// maxAnisotropy
	//	D3D12_COMPARISON_FUNC_ALWAYS,					// comparisonFunc
	//	D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK,	// borderColor
	//	0.0f,											// minLOD
	//	D3D12_FLOAT32_MAX,								// maxLOD
	//	D3D12_SHADER_VISIBILITY_PIXEL,					// shaderVisibility
	//	0												// registerSpace
	//);
	//samplerDesc[1].Init(
	//	1,								 					// ShaderRegister
	//	D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT,	// filter
	//	D3D12_TEXTURE_ADDRESS_MODE_BORDER, 					// addressU
	//	D3D12_TEXTURE_ADDRESS_MODE_BORDER, 					// addressV
	//	D3D12_TEXTURE_ADDRESS_MODE_BORDER, 					// addressW
	//	0.0f,												// mipLODBias
	//	16,													// maxAnisotropy
	//	D3D12_COMPARISON_FUNC_LESS_EQUAL,					// comparisonFunc
	//	D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK			// borderColor
	//);

	
	CD3DX12_ROOT_SIGNATURE_DESC d3dRootSignatureDesc;
	d3dRootSignatureDesc.Init(_countof(pd3dRootParameters), pd3dRootParameters,0 /*_countof(samplerDesc)*/, nullptr/*samplerDesc*/, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);


	ComPtr<ID3DBlob> pd3dSignatureBlob, pd3dErrorBlob;
	::D3D12SerializeRootSignature(&d3dRootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &pd3dSignatureBlob, &pd3dErrorBlob);
	DX::ThrowIfFailed(DEVICE->CreateRootSignature(0, pd3dSignatureBlob->GetBufferPointer(), pd3dSignatureBlob->GetBufferSize(), IID_PPV_ARGS(&m_pd3dGraphicsRootSignature)));


}
