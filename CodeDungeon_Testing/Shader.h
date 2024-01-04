#include "GameObject.h"
#include "Camera.h"

struct CB_GAMEOBJECT_INFO
{
	XMFLOAT4X4 m_xmf4x4World;
};

class CShader
{
public:
	CShader();
	virtual ~CShader();
private:
	int m_nReferences = 0;
public:
	void AddRef() { m_nReferences++; }
	void Release() { if (--m_nReferences <= 0) delete this; }
	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout();
	virtual D3D12_RASTERIZER_DESC CreateRasterizerState();
	virtual D3D12_BLEND_DESC CreateBlendState();
	virtual D3D12_DEPTH_STENCIL_DESC CreateDepthStencilState();

	

	virtual void CreateShader(const wstring& shaderFile, const string& vs, const string& ps,const ComPtr<ID3D12Device>& _Device, const ComPtr<ID3D12RootSignature>& _RootSignature);
	virtual void CreateShaderVariables(const ComPtr<ID3D12Device>& _Device, const ComPtr<ID3D12GraphicsCommandList>& _CommandList);
	virtual void UpdateShaderVariables(const ComPtr<ID3D12GraphicsCommandList>& _CommandList);
	virtual void ReleaseShaderVariables();
	virtual void UpdateShaderVariable(const ComPtr<ID3D12GraphicsCommandList>& _CommandList,
		XMFLOAT4X4* pxmf4x4World);
	virtual void OnPrepareRender(const ComPtr<ID3D12GraphicsCommandList>& _CommandList);
	virtual void Render(const ComPtr<ID3D12GraphicsCommandList>& _CommandList, CCamera* pCamera);
protected:
	ComPtr<ID3D12PipelineState> m_pd3dPipelineStates;
	
};


class CDiffusedShader : public CShader
{
public:
	CDiffusedShader();
	virtual ~CDiffusedShader();
	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout();

	virtual void CreateShader(const wstring& shaderFile, const string& vs, const string& ps, const ComPtr<ID3D12Device>& _Device, const ComPtr<ID3D12RootSignature>& _RootSignature);
};