#include "Input.h"
#include "Enemy.h"
#include "WinApp.h"
#include "DirectXCommon.h"
#include "Logger.h"
#include "StringUtility.h"
#pragma warning(push)
#define _USE_MATH_DEFINES
#pragma warning(disable:4023)

#include <windef.h>
#include <winuser.h>
#include <cstdint>
#include <string>
#include <format>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <cassert>
#include <cmath>
#include<math.h>

#include <fstream>
#include <sstream>

#include <wrl.h>
#pragma warning(pop)

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")
#include <dxgidebug.h>
#pragma comment(lib,"dxguid.lib")
#include<dxcapi.h>
#pragma comment(lib,"dxcompiler.lib")

#include "externals/imgui/imgui.h"
#include "externals/imgui/imgui_impl_dx12.h"
#include "externals/imgui/imgui_impl_win32.h"


#include <stdio.h>

#include "externals/DirectXTex/DirectXTex.h"

//Microsoft::WRL::ComPtr < LRESULT> CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);


struct Vector4 {
	float x, y, z, w;
};
struct Vector3 {
	float x, y, z;
};
struct Vector2 {
	float x, y;
};
struct Matrix3x3
{
	float m[3][3];
};
struct Matrix4x4
{
	float m[4][4];
};
struct Transform {
	Vector3 scale;
	Vector3 rotate;
	Vector3 translate;
};
struct VertexData {
	Vector4 position;
	Vector2 texcoord;
	Vector3 normal;
};
struct Material {
	Vector4 color;
	int32_t enablleLighting;
	float padding[3];
	Matrix4x4 uvTransform;
};
struct TransformationMatrix {
	Matrix4x4 WVP;
	Matrix4x4 World;
};

struct DirectionalLight {
	Vector4 color;
	Vector3 direction;
	float intensity;
};

struct MaterialData {
	std::string textureFilePath;

};
struct ModelData {
	std::vector<VertexData> vertices;
	MaterialData material;
};
struct D3DResourceLeakChecker {
	~D3DResourceLeakChecker() {
		Microsoft::WRL::ComPtr <IDXGIDebug1> debug;
		if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&debug)))) {
			debug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);
			debug->ReportLiveObjects(DXGI_DEBUG_APP, DXGI_DEBUG_RLO_ALL);
			debug->ReportLiveObjects(DXGI_DEBUG_D3D12, DXGI_DEBUG_RLO_ALL);
			//debug->Release();
		}
	}
};

Matrix4x4 MakeIdenty4x4();
Matrix4x4 MakeAffinMatrix(const Vector3& S, const Vector3& R, const Vector3& T);
Matrix4x4 Multiply(const Matrix4x4& m1, const Matrix4x4& m2);
Matrix4x4 MAkeTranslateMatrix(const Vector3& vector3);

Matrix4x4  MAkeScaleMatrix(Vector3& vector3);

Matrix4x4 MakeRotateXMatrix(Vector3& vector);

Matrix4x4 MakeRotateYMatrix(Vector3& vector);

Matrix4x4 MakeRotateZMatrix(Vector3& vector);
Matrix4x4 MakePerspectiveMatrix(float fovY, float aspectRatio, float nearClip, float farClip);
Matrix4x4 Inverse(Matrix4x4& m);


Matrix4x4 MakeOrthographicMatrix(float left, float top, float right, float bottom, float nearClip, float farClip);



ModelData LoadObjFile(const std::string& directoryPath, const std::string& filename);

MaterialData LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& filename);

DirectXCommon* dxCommon = nullptr;
Logger* Log = new Logger;


int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
	

	D3DResourceLeakChecker checker;
	CoInitializeEx(0, COINIT_MULTITHREADED);

	WinApp* winapp = new WinApp;
	winapp->Initialize();

	
#ifdef _DEBUG
	Microsoft::WRL::ComPtr <ID3D12Debug1> debufController = nullptr;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debufController)))) {
		debufController->EnableDebugLayer();
		debufController->SetEnableGPUBasedValidation(TRUE);
	}
#endif // _DEBUG
	//出力ウィンドウの文字出力
	Log->Log("Hello,DirectX\n");
	
	

	dxCommon = new DirectXCommon;
	dxCommon->Initialize(winapp);
	Log->Log("Complete create D3D12Device!!!\n");

	HRESULT hr = dxCommon->Gethr();

	

	Microsoft::WRL::ComPtr < ID3D12Resource> VertexResourceSprite = dxCommon->CreateBufferResource(dxCommon->Getdevice().Get(), sizeof(VertexData) * 6);

	D3D12_VERTEX_BUFFER_VIEW vertexBufferViewSprite{};

	vertexBufferViewSprite.BufferLocation = VertexResourceSprite->GetGPUVirtualAddress();

	vertexBufferViewSprite.SizeInBytes = sizeof(VertexData) * 6;

	vertexBufferViewSprite.StrideInBytes = sizeof(VertexData);

	VertexData* vertexDataSprite = nullptr;
	VertexResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&vertexDataSprite));

	vertexDataSprite[0].position = { 0.0f,360.0f,0.0f,1.0f };
	vertexDataSprite[0].texcoord = { 0.0f,1.0f };
	vertexDataSprite[0].normal = { 0.0f,0.0f,-1.0f };
	vertexDataSprite[1].position = { 0.0f,0.0f,0.0f,1.0f };
	vertexDataSprite[1].texcoord = { 0.0f,0.0f };
	vertexDataSprite[1].normal = { 0.0f,0.0f,-1.0f };
	vertexDataSprite[2].position = { 640.0f,360.0f,0.0f,1.0f };
	vertexDataSprite[2].texcoord = { 1.0f,1.0f };
	vertexDataSprite[2].normal = { 0.0f,0.0f,-1.0f };

	vertexDataSprite[3].position = { 640.0f,0.0f,0.0f,1.0f };
	vertexDataSprite[3].texcoord = { 1.0f,0.0f };
	vertexDataSprite[3].normal = { 0.0f,0.0f,-1.0f };
	//vertexDataSprite[4].position = { 640.0f,0.0f,0.0f,1.0f };
	//vertexDataSprite[4].texcoord = { 1.0f,0.0f };
	//vertexDataSprite[4].normal = { 0.0f,0.0f,-1.0f };
	//vertexDataSprite[5].position = { 640.0f,360.0f,0.0f,1.0f };
	//vertexDataSprite[5].texcoord = { 1.0f,1.0f };
	///vertexDataSprite[5].normal = { 0.0f,0.0f,-1.0f };

	Microsoft::WRL::ComPtr < ID3D12Resource> transformationMatrixResourceSprite = dxCommon->CreateBufferResource(dxCommon->Getdevice().Get(), sizeof(TransformationMatrix));
	TransformationMatrix* transformationMatrixDataSprite = nullptr;
	transformationMatrixResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixDataSprite));
	transformationMatrixDataSprite->World = MakeIdenty4x4();

	Microsoft::WRL::ComPtr < ID3D12Resource> indexResourceSprite = dxCommon->CreateBufferResource(dxCommon->Getdevice().Get(), sizeof(uint32_t) * 6);

	D3D12_INDEX_BUFFER_VIEW indexBufferViewSprite{};

	indexBufferViewSprite.BufferLocation = indexResourceSprite->GetGPUVirtualAddress();

	indexBufferViewSprite.SizeInBytes = sizeof(uint32_t) * 6;

	indexBufferViewSprite.Format = DXGI_FORMAT_R32_UINT;

	uint32_t* indexDataSprite = nullptr;
	indexResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&indexDataSprite));
	indexDataSprite[0] = 0;
	indexDataSprite[1] = 1;
	indexDataSprite[2] = 2;

	indexDataSprite[3] = 1;
	indexDataSprite[4] = 3;
	indexDataSprite[5] = 2;

	Transform transformSprite{ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };


	Microsoft::WRL::ComPtr < ID3D12Resource> depthStencilResource = dxCommon->CreateDepthStencilTextureResource(dxCommon->Getdevice(), winapp->kClientWidth, winapp->kClientHeight);

	D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
	descriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	D3D12_DESCRIPTOR_RANGE descriptorRange[1] = {};
	descriptorRange[0].BaseShaderRegister = 0;
	descriptorRange[0].NumDescriptors = 1;
	descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	D3D12_ROOT_PARAMETER rootParameters[4] = {};
	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[0].Descriptor.ShaderRegister = 0;
	rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	rootParameters[1].Descriptor.ShaderRegister = 0;
	rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[2].DescriptorTable.pDescriptorRanges = descriptorRange;
	rootParameters[2].DescriptorTable.NumDescriptorRanges = _countof(descriptorRange);
	rootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[3].Descriptor.ShaderRegister = 1;

	descriptionRootSignature.pParameters = rootParameters;
	descriptionRootSignature.NumParameters = _countof(rootParameters);

	D3D12_STATIC_SAMPLER_DESC staticSamplers[1] = {};
	staticSamplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	staticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	staticSamplers[0].MaxLOD = D3D12_FLOAT32_MAX;
	staticSamplers[0].ShaderRegister = 0;
	staticSamplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	descriptionRootSignature.pStaticSamplers = staticSamplers;
	descriptionRootSignature.NumStaticSamplers = _countof(staticSamplers);

	Microsoft::WRL::ComPtr < ID3D12Resource> wvpResource = dxCommon->CreateBufferResource(dxCommon->Getdevice().Get(), sizeof(TransformationMatrix));

	TransformationMatrix* wvpData = nullptr;
	wvpResource->Map(0, nullptr, reinterpret_cast<void**>(&wvpData));
	wvpData->World = MakeIdenty4x4();
	wvpData->WVP = MakeIdenty4x4();

	Transform transform{ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };
	Matrix4x4 worldMatrix = MakeAffinMatrix(transform.scale, transform.rotate, transform.translate);

	Microsoft::WRL::ComPtr < ID3DBlob> signatureBlob = nullptr;
	Microsoft::WRL::ComPtr < ID3DBlob> errorBlob = nullptr;

	hr = D3D12SerializeRootSignature(&descriptionRootSignature,
		D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);


	if (FAILED(hr)) {
		Log->Log(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
		assert(false);
	}

	Microsoft::WRL::ComPtr < ID3D12RootSignature> rootSignature = nullptr;
	hr = dxCommon->Getdevice()->CreateRootSignature(0, signatureBlob->GetBufferPointer(),
		signatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature));
	assert(SUCCEEDED(hr));

	D3D12_INPUT_ELEMENT_DESC inputElementDescs[3] = {};
	inputElementDescs[0].SemanticName = "POSITION";
	inputElementDescs[0].SemanticIndex = 0;
	inputElementDescs[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	inputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	inputElementDescs[1].SemanticName = "TEXCOORD";
	inputElementDescs[1].SemanticIndex = 0;
	inputElementDescs[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	inputElementDescs[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	inputElementDescs[2].SemanticName = "NORMAL";
	inputElementDescs[2].SemanticIndex = 0;
	inputElementDescs[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	inputElementDescs[2].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
	inputLayoutDesc.pInputElementDescs = inputElementDescs;
	inputLayoutDesc.NumElements = _countof(inputElementDescs);

	D3D12_BLEND_DESC blendDesc{};

	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	D3D12_RASTERIZER_DESC rasterizerDesc{};
	rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;


	D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
	depthStencilDesc.DepthEnable = true;
	depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;

	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc{};
	graphicsPipelineStateDesc.pRootSignature = rootSignature.Get();
	graphicsPipelineStateDesc.InputLayout = inputLayoutDesc;

	graphicsPipelineStateDesc.BlendState = blendDesc;

	graphicsPipelineStateDesc.VS = {
		dxCommon->GetvertexShaderBlob()->GetBufferPointer(),
		dxCommon->GetvertexShaderBlob()->GetBufferSize()
	};
	graphicsPipelineStateDesc.RasterizerState = rasterizerDesc;
	graphicsPipelineStateDesc.PS = {
		dxCommon->GetpixelShaderBlob()->GetBufferPointer(),
		dxCommon->GetpixelShaderBlob()->GetBufferSize()
	};

	graphicsPipelineStateDesc.DepthStencilState = depthStencilDesc;
	graphicsPipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

	graphicsPipelineStateDesc.NumRenderTargets = 1;
	graphicsPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	graphicsPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

	graphicsPipelineStateDesc.SampleDesc.Count = 1;
	graphicsPipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;



	Microsoft::WRL::ComPtr < ID3D12PipelineState> graphicsPipelineState = nullptr;
	hr = dxCommon->Getdevice()->CreateGraphicsPipelineState(&graphicsPipelineStateDesc, IID_PPV_ARGS(&graphicsPipelineState));

	assert(SUCCEEDED(hr));

	int kSubdivision = 16;
	int latIndex = kSubdivision;
	int lonIndex = kSubdivision;

	Microsoft::WRL::ComPtr < ID3D12Resource> vertexResource = dxCommon->CreateBufferResource(dxCommon->Getdevice().Get(), sizeof(VertexData) * (kSubdivision * kSubdivision * 6));

	D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};
	vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();
	vertexBufferView.SizeInBytes = sizeof(VertexData) * (kSubdivision * kSubdivision * 6);
	vertexBufferView.StrideInBytes = sizeof(VertexData);


	Microsoft::WRL::ComPtr < ID3D12Resource> materialResourceSprite = dxCommon->CreateBufferResource(dxCommon->Getdevice().Get(), sizeof(Material));


	Material* materialData = nullptr;

	materialResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&materialData));

	materialData->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);

	materialData->enablleLighting = true;

	materialData->uvTransform = MakeIdenty4x4();


	Microsoft::WRL::ComPtr < ID3D12Resource> materialSpriteResourceSprite = dxCommon->CreateBufferResource(dxCommon->Getdevice().Get(), sizeof(Material));

	Material* materialDataSprite = nullptr;

	materialSpriteResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&materialDataSprite));

	materialDataSprite->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);

	materialDataSprite->enablleLighting = true;

	materialDataSprite->uvTransform = MakeIdenty4x4();

	VertexData* vertexData = nullptr;

	vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));

	Transform uvTransformSprite{
		{1.0f,1.0f,1.0f},
		{0.0f,0.0f,0.0f},
		{0.0f,0.0f,0.0f}
	};

	ModelData modelData = LoadObjFile("resources", "axis.obj");

	Microsoft::WRL::ComPtr < ID3D12Resource> vertexResource2 = dxCommon->CreateBufferResource(dxCommon->Getdevice().Get(), sizeof(VertexData) * modelData.vertices.size());

	D3D12_VERTEX_BUFFER_VIEW vertexBufferView2{};
	vertexBufferView2.BufferLocation = vertexResource2->GetGPUVirtualAddress();
	vertexBufferView2.SizeInBytes = UINT(sizeof(VertexData) * modelData.vertices.size());
	vertexBufferView2.StrideInBytes = sizeof(VertexData);

	VertexData* vertexData2 = nullptr;
	vertexResource2->Map(0, nullptr, reinterpret_cast<void**>(&vertexData2));
	std::memcpy(vertexData2, modelData.vertices.data(), sizeof(VertexData) * modelData.vertices.size());

	const float kLonEvery = (float)M_PI * 2.0f / float(kSubdivision);

	const float kLatEvery = (float)M_PI / float(kSubdivision);

	float u;

	float v;

	for (latIndex = 0;latIndex < kSubdivision;++latIndex) {
		float lat = (float)-M_PI / 2.0f + kLatEvery * latIndex;

		for (lonIndex = 0; lonIndex < kSubdivision;++lonIndex) {
			uint32_t start = (uint32_t)(latIndex * kSubdivision + lonIndex) * 6;
			float lon = lonIndex * kLonEvery;

			u = float(lonIndex) / float(kSubdivision);
			v = 1.0f - float(latIndex) / float(kSubdivision);
			//b
			vertexData[start + 5].position.x = cosf(lat + kLatEvery) * cosf(lon);
			vertexData[start + 5].position.y = sinf(lat + kLatEvery);
			vertexData[start + 5].position.z = cosf(lat + kLatEvery) * sinf(lon);
			vertexData[start + 5].position.w = 1;
			vertexData[start + 5].texcoord = { u,v };

			vertexData[start + 5].normal.x = vertexData[start + 5].position.x;
			vertexData[start + 5].normal.y = vertexData[start + 5].position.y;
			vertexData[start + 5].normal.z = vertexData[start + 5].position.z;


			u = float(lonIndex + 1) / float(kSubdivision);
			v = 1.0f - float(latIndex) / float(kSubdivision);
			//d
			vertexData[start + 3].position.x = cosf(lat + kLatEvery) * cosf(lon + kLonEvery);
			vertexData[start + 3].position.y = sinf(lat + kLatEvery);
			vertexData[start + 3].position.z = cosf(lat + kLatEvery) * sinf(lon + kLonEvery);
			vertexData[start + 3].position.w = 1;
			vertexData[start + 3].texcoord = { u,v };

			vertexData[start + 3].normal.x = vertexData[start + 3].position.x;
			vertexData[start + 3].normal.y = vertexData[start + 3].position.y;
			vertexData[start + 3].normal.z = vertexData[start + 3].position.z;

			u = float(lonIndex + 1) / float(kSubdivision);
			v = 1.0f - float(latIndex - 1) / float(kSubdivision);
			//c
			vertexData[start + 4].position.x = cosf(lat) * cosf(lon + kLonEvery);
			vertexData[start + 4].position.y = sinf(lat);
			vertexData[start + 4].position.z = cosf(lat) * sinf(lon + kLonEvery);
			vertexData[start + 4].position.w = 1;
			vertexData[start + 4].texcoord = { u,v };

			vertexData[start + 4].normal.x = vertexData[start + 4].position.x;
			vertexData[start + 4].normal.y = vertexData[start + 4].position.y;
			vertexData[start + 4].normal.z = vertexData[start + 4].position.z;


			u = float(lonIndex) / float(kSubdivision);
			v = 1.0f - float(latIndex - 1) / float(kSubdivision);
			// //a
			vertexData[start].position.x = cosf(lat) * cosf(lon);
			vertexData[start].position.y = sinf(lat);
			vertexData[start].position.z = cosf(lat) * sinf(lon);
			vertexData[start].position.w = 1;
			vertexData[start].texcoord = { u,v };

			vertexData[start].normal.x = vertexData[start].position.x;
			vertexData[start].normal.y = vertexData[start].position.y;
			vertexData[start].normal.z = vertexData[start].position.z;

			u = float(lonIndex) / float(kSubdivision);
			v = 1.0f - float(latIndex) / float(kSubdivision);
			//b
			vertexData[start + 1].position.x = cosf(lat + kLatEvery) * cosf(lon);
			vertexData[start + 1].position.y = sinf(lat + kLatEvery);
			vertexData[start + 1].position.z = cosf(lat + kLatEvery) * sinf(lon);
			vertexData[start + 1].position.w = 1;
			vertexData[start + 1].texcoord = { u,v };

			vertexData[start + 1].normal.x = vertexData[start + 1].position.x;
			vertexData[start + 1].normal.y = vertexData[start + 1].position.y;
			vertexData[start + 1].normal.z = vertexData[start + 1].position.z;

			u = float(lonIndex + 1) / float(kSubdivision);
			v = 1.0f - float(latIndex - 1) / float(kSubdivision);
			//c
			vertexData[start + 2].position.x = cosf(lat) * cosf(lon + kLonEvery);
			vertexData[start + 2].position.y = sinf(lat);
			vertexData[start + 2].position.z = cosf(lat) * sinf(lon + kLonEvery);
			vertexData[start + 2].position.w = 1;
			vertexData[start + 2].texcoord = { u,v };

			vertexData[start + 2].normal.x = vertexData[start + 2].position.x;
			vertexData[start + 2].normal.y = vertexData[start + 2].position.y;
			vertexData[start + 2].normal.z = vertexData[start + 2].position.z;
			//commandList.DrawInstanced(6, 1, 0, 0);


		}


	}

	const uint32_t desriptorSizeSRV = dxCommon->Getdevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	const uint32_t desriptorSizeRTV = dxCommon->Getdevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	const uint32_t desriptorSizeDSV = dxCommon->Getdevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

	

	Transform cameraTransform{ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,-30.0f} };

	Microsoft::WRL::ComPtr < ID3D12Resource> directionalLightDataResource = dxCommon->CreateBufferResource(dxCommon->Getdevice().Get(), sizeof(DirectionalLight));
	DirectionalLight* directionalLightData = nullptr;
	directionalLightDataResource->Map(0, nullptr, reinterpret_cast<void**>(&directionalLightData));
	directionalLightData->color = { 1.0f,1.0f,1.0f,1.0f };
	directionalLightData->direction = { 0.0f,-1.0f,0.0f };
	directionalLightData->intensity = 1.0f;

	bool useMonsterBall = false;

	if (modelData.material.textureFilePath != "") {
		DirectX::ScratchImage mipImages2 = dxCommon->LoadTexture(modelData.material.textureFilePath);
	}

	uint64_t fenceValue = 0;

	/*Input* input = nullptr;
	input = new Input();
	input->Initialize(winapp);*/

	Enemy* enemy = new Enemy;

	MSG msg{};
	while (msg.message != WM_QUIT) {
		
		if (winapp->ProcessMessage()) {
			break;
		}
		else {
			enemy->Atack();
			enemy->Move();
			enemy->Remove();
			//transform.rotate.y += 0.03f;
			ImGui_ImplDX12_NewFrame();
			ImGui_ImplWin32_NewFrame();
			ImGui::NewFrame();
			ImGui::ShowDemoWindow();


			ImGui::Begin("Color");
			ImGui::SliderFloat4("Color", &materialData->color.x, 0.0f, 1.0f);
			ImGui::SliderFloat3("Camera", &cameraTransform.translate.x, -150.0f, 150.0f);
			ImGui::SliderFloat4("dir", &directionalLightData->direction.x, -1.0f, 1.0f);

			ImGui::Checkbox("useMonsterBall", &useMonsterBall);

			ImGui::DragFloat2("UVTranslate", &uvTransformSprite.translate.x, 0.01f, -10.0f, 10.0f);
			ImGui::DragFloat2("UVScale", &uvTransformSprite.scale.x, 0.01f, -10.0f, 10.0f);
			ImGui::SliderAngle("UVROtate", &uvTransformSprite.rotate.z);


			ImGui::DragFloat2("rotate", &transform.rotate.x, 0.01f, -10.0f, 10.0f);
			ImGui::DragFloat2("scale", &transform.scale.x, 0.01f, -10.0f, 10.0f);
			ImGui::DragFloat3("translate", &transform.translate.x, 0.01f, -10.0f, 10.0f);

			if(enemy->GetAtack())

			ImGui::End();

			ImGui::Render();
			//Matrix4x4 projectionMatrix = MakePerspectiveMatrix(0.45f, float(kClientWidth) / float(kClientHeight), 0.1f, 100.0f);
			Matrix4x4 worldMatrix = MakeAffinMatrix(transform.scale, transform.rotate, transform.translate);
			Matrix4x4 cameraMatrix = MakeAffinMatrix(cameraTransform.scale, cameraTransform.rotate, cameraTransform.translate);
			Matrix4x4 viewMatrix = Inverse(cameraMatrix);
			Matrix4x4 projectionMatrix = MakePerspectiveMatrix(0.45f, float(winapp->kClientWidth) / float(winapp->kClientHeight), 0.1f, 100.0f);
			Matrix4x4 worldViewProjectionMatrix = Multiply(worldMatrix, Multiply(viewMatrix, projectionMatrix));
			Matrix4x4 transformationMatrixDate = worldViewProjectionMatrix;
			//Matrix4x4 worldMatrix = MakeAffinMatrix(transform.scale, transform.rotate, transform.translate);
			wvpData->World = worldMatrix;
			wvpData->WVP = worldViewProjectionMatrix;

			Matrix4x4 worldMatrixSprite = MakeAffinMatrix(transformSprite.scale, transformSprite.rotate, transformSprite.translate);
			Matrix4x4 viewMatrixSprite = MakeIdenty4x4();
			Matrix4x4 projectionMatrixSprite = MakeOrthographicMatrix(0.0f, 0.0f, float(winapp->kClientWidth), float(winapp->kClientHeight), 0.0f, 100.0f);
			Matrix4x4 worldViewProjectionMatrixSprite = Multiply(worldMatrixSprite, Multiply(viewMatrixSprite, projectionMatrixSprite));
			transformationMatrixDataSprite->World = worldViewProjectionMatrixSprite;
			transformationMatrixDataSprite->WVP = worldViewProjectionMatrixSprite;

			Matrix4x4 uvTransformMatrix = MAkeScaleMatrix(uvTransformSprite.scale);
			uvTransformMatrix = Multiply(uvTransformMatrix, MakeRotateZMatrix(uvTransformSprite.rotate));
			uvTransformMatrix = Multiply(uvTransformMatrix, MAkeTranslateMatrix(uvTransformSprite.translate));
			materialDataSprite->uvTransform = uvTransformMatrix;

			UINT backBufferIndex = dxCommon->GetswapChain()->GetCurrentBackBufferIndex();
			dxCommon->GetcommandList()->OMSetRenderTargets(1, &dxCommon->GetrtvHandles(backBufferIndex), false, nullptr);

			D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = dxCommon->GetdsvDescriptorHeap()->GetCPUDescriptorHandleForHeapStart();
			dxCommon->GetcommandList()->OMSetRenderTargets(1, &dxCommon->GetrtvHandles(backBufferIndex), false, &dsvHandle);

			//input->Updat();


			D3D12_RESOURCE_BARRIER barrier{};

			barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
			barrier.Transition.pResource = dxCommon->GetswapChainResources(backBufferIndex).Get();
			barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
			barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;

			

			dxCommon->PreDraw();


			dxCommon->GetcommandList()->SetGraphicsRootSignature(rootSignature.Get());
			dxCommon->GetcommandList()->SetPipelineState(graphicsPipelineState.Get());
			dxCommon->GetcommandList()->IASetVertexBuffers(0, 1, &vertexBufferView2);



			dxCommon->GetcommandList()->SetGraphicsRootConstantBufferView(3, directionalLightDataResource->GetGPUVirtualAddress());

			dxCommon->GetcommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

			dxCommon->GetcommandList()->SetGraphicsRootConstantBufferView(0, materialResourceSprite->GetGPUVirtualAddress());



			dxCommon->GetcommandList()->SetGraphicsRootConstantBufferView(1, wvpResource->GetGPUVirtualAddress());
			dxCommon->GetcommandList()->SetGraphicsRootDescriptorTable(2, useMonsterBall ? dxCommon->GettextureSrvHandleGPU2() : dxCommon->GettextureSrvHandleGPU());
			//commandList->DrawInstanced(latIndex * lonIndex * 6, 1, 0, 0);
			dxCommon->GetcommandList()->DrawInstanced(UINT(modelData.vertices.size()), 1, 0, 0);

			dxCommon->GetcommandList()->IASetIndexBuffer(&indexBufferViewSprite);
			dxCommon->GetcommandList()->IASetVertexBuffers(0, 1, &vertexBufferViewSprite);

			dxCommon->GetcommandList()->SetGraphicsRootConstantBufferView(0, materialSpriteResourceSprite->GetGPUVirtualAddress());
			dxCommon->GetcommandList()->SetGraphicsRootDescriptorTable(2, dxCommon->GettextureSrvHandleGPU());
			dxCommon->GetcommandList()->SetGraphicsRootConstantBufferView(1, transformationMatrixResourceSprite->GetGPUVirtualAddress());
			//commandList->DrawInstanced(6, 1, 0, 0);
			dxCommon->GetcommandList()->DrawIndexedInstanced(6, 1, 0, 0, 0);

			ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), dxCommon->GetcommandList().Get());
			
			

			dxCommon->PostDraw();
		}
	}


	winapp->Finalize();


#ifdef _DEBUG
#endif
	
	
	
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
	delete dxCommon;
	
	winapp->Finalize();
	return 0;
}


Matrix4x4 MakeIdenty4x4()
{
	Matrix4x4 ans;
	for (int i = 0;i < 4;i++) {
		for (int j = 0;j < 4;j++) {
			ans.m[i][j] = 0;
			if (i == j) {
				ans.m[i][j] = 1;
			}
		}
	}
	return ans;
}


Matrix4x4 MakeAffinMatrix(const Vector3& S, const Vector3& R, const Vector3& T) {
	Matrix4x4 ans{ 0 };
	Vector3 S2 = S;
	Vector3 R2 = R;
	Vector3 T2 = T;
	Matrix4x4 matrixS = MAkeTranslateMatrix(S2);
	Matrix4x4 matrixR = Multiply(MakeRotateXMatrix(R2), Multiply(MakeRotateYMatrix(R2), MakeRotateZMatrix(R2)));
	Matrix4x4 matrixT = MAkeScaleMatrix(T2);
	ans = Multiply(matrixS, Multiply(matrixR, matrixT));
	return ans;
}
Matrix4x4 Multiply(const Matrix4x4& m1, const Matrix4x4& m2) {
	Matrix4x4 ans = { 0 };
	for (int i = 0;i < 4;i++) {
		for (int j = 0;j < 4;j++) {
			ans.m[i][j] += m1.m[i][0] * m2.m[0][j];
			ans.m[i][j] += m1.m[i][1] * m2.m[1][j];
			ans.m[i][j] += m1.m[i][2] * m2.m[2][j];
			ans.m[i][j] += m1.m[i][3] * m2.m[3][j];
		}
	}
	return ans;
}
Matrix4x4 MAkeTranslateMatrix(const Vector3& vector3) {
	Matrix4x4 ans{ 0 };
	ans.m[0][0] = vector3.x;
	ans.m[1][1] = vector3.y;
	ans.m[2][2] = vector3.z;
	ans.m[3][3] = 1;
	return ans;
}
Matrix4x4  MAkeScaleMatrix(Vector3& vector3) {
	Matrix4x4 ans{ 0 };
	ans.m[0][0] = 1;
	ans.m[1][1] = 1;
	ans.m[2][2] = 1;
	ans.m[3][0] = vector3.x;
	ans.m[3][1] = vector3.y;
	ans.m[3][2] = vector3.z;
	ans.m[3][3] = 1;
	return ans;
}

Matrix4x4 MakeRotateXMatrix(Vector3& vector) {
	Matrix4x4 matrix{ 0 };
	matrix.m[1][1] = std::cosf(vector.x);
	matrix.m[1][2] = std::sinf(vector.x);
	matrix.m[2][1] = -std::sinf(vector.x);
	matrix.m[2][2] = std::cosf(vector.x);
	matrix.m[0][0] = 1;
	matrix.m[3][3] = 1;
	return matrix;
}

Matrix4x4 MakeRotateYMatrix(Vector3& vector) {
	Matrix4x4 matrix{ 0 };
	matrix.m[0][0] = std::cosf(vector.y);
	matrix.m[0][2] = -std::sinf(vector.y);
	matrix.m[2][0] = std::sinf(vector.y);
	matrix.m[2][2] = std::cosf(vector.y);
	matrix.m[1][1] = 1;
	matrix.m[3][3] = 1;
	return matrix;
}

Matrix4x4 MakeRotateZMatrix(Vector3& vector) {
	Matrix4x4 matrix{ 0 };
	matrix.m[0][0] = std::cosf(vector.z);
	matrix.m[0][1] = std::sinf(vector.z);
	matrix.m[1][0] = -std::sinf(vector.z);
	matrix.m[1][1] = std::cosf(vector.z);
	matrix.m[2][2] = 1;
	matrix.m[3][3] = 1;
	return matrix;
}

Matrix4x4 MakePerspectiveMatrix(float fovY, float aspectRatio, float nearClip, float farClip) {
	Matrix4x4 ans = { 0 };
	ans.m[0][0] = 1 / aspectRatio * (1 / std::tanf(fovY / 2));
	ans.m[1][1] = (1 / std::tanf(fovY / 2));
	ans.m[2][2] = farClip / (farClip - nearClip);
	ans.m[3][2] = -nearClip * farClip / (farClip - nearClip);
	ans.m[2][3] = 1;
	return ans;
}
Matrix4x4 Inverse(Matrix4x4& m) {
	Matrix4x4 ans;
	float A;
	A = m.m[0][0] * m.m[1][1] * m.m[2][2] * m.m[3][3]
		+ m.m[0][0] * m.m[1][2] * m.m[2][3] * m.m[3][1]
		+ m.m[0][0] * m.m[1][3] * m.m[2][1] * m.m[3][2]

		- m.m[0][0] * m.m[1][3] * m.m[2][2] * m.m[3][1]
		- m.m[0][0] * m.m[1][2] * m.m[2][1] * m.m[3][3]
		- m.m[0][0] * m.m[1][1] * m.m[2][3] * m.m[3][2]

		- m.m[0][1] * m.m[1][0] * m.m[2][2] * m.m[3][3]
		- m.m[0][2] * m.m[1][0] * m.m[2][3] * m.m[3][1]
		- m.m[0][3] * m.m[1][0] * m.m[2][1] * m.m[3][2]

		+ m.m[0][3] * m.m[1][0] * m.m[2][2] * m.m[3][1]
		+ m.m[0][2] * m.m[1][0] * m.m[2][1] * m.m[3][3]
		+ m.m[0][1] * m.m[1][0] * m.m[2][3] * m.m[3][2]

		+ m.m[0][1] * m.m[1][2] * m.m[2][0] * m.m[3][3]
		+ m.m[0][2] * m.m[1][3] * m.m[2][0] * m.m[3][1]
		+ m.m[0][3] * m.m[1][1] * m.m[2][0] * m.m[3][2]

		- m.m[0][3] * m.m[1][2] * m.m[2][0] * m.m[3][1]
		- m.m[0][2] * m.m[1][1] * m.m[2][0] * m.m[3][3]
		- m.m[0][1] * m.m[1][3] * m.m[2][0] * m.m[3][2]

		- m.m[0][1] * m.m[1][2] * m.m[2][3] * m.m[3][0]
		- m.m[0][2] * m.m[1][3] * m.m[2][1] * m.m[3][0]
		- m.m[0][3] * m.m[1][1] * m.m[2][2] * m.m[3][0]

		+ m.m[0][3] * m.m[1][2] * m.m[2][1] * m.m[3][0]
		+ m.m[0][2] * m.m[1][1] * m.m[2][3] * m.m[3][0]
		+ m.m[0][1] * m.m[1][3] * m.m[2][2] * m.m[3][0];

	ans.m[0][0] =
		m.m[1][1] * m.m[2][2] * m.m[3][3]
		+ m.m[1][2] * m.m[2][3] * m.m[3][1]
		+ m.m[1][3] * m.m[2][1] * m.m[3][2]

		- m.m[1][3] * m.m[2][2] * m.m[3][1]
		- m.m[1][2] * m.m[2][1] * m.m[3][3]
		- m.m[1][1] * m.m[2][3] * m.m[3][2];

	ans.m[0][1] =
		-m.m[0][1] * m.m[2][2] * m.m[3][3]
		- m.m[0][2] * m.m[2][3] * m.m[3][1]
		- m.m[0][3] * m.m[2][1] * m.m[3][2]

		+ m.m[0][3] * m.m[2][2] * m.m[3][1]
		+ m.m[0][2] * m.m[2][1] * m.m[3][3]
		+ m.m[0][1] * m.m[2][3] * m.m[3][2];

	ans.m[0][2] =
		m.m[0][1] * m.m[1][2] * m.m[3][3]
		+ m.m[0][2] * m.m[1][3] * m.m[3][1]
		+ m.m[0][3] * m.m[1][1] * m.m[3][2]

		- m.m[0][3] * m.m[1][2] * m.m[3][1]
		- m.m[0][2] * m.m[1][1] * m.m[3][3]
		- m.m[0][1] * m.m[1][3] * m.m[3][2];

	ans.m[0][3] =
		-m.m[0][1] * m.m[1][2] * m.m[2][3]
		- m.m[0][2] * m.m[1][3] * m.m[2][1]
		- m.m[0][3] * m.m[1][1] * m.m[2][2]

		+ m.m[0][3] * m.m[1][2] * m.m[2][1]
		+ m.m[0][2] * m.m[1][1] * m.m[2][3]
		+ m.m[0][1] * m.m[1][3] * m.m[2][2];



	ans.m[1][0] =
		-m.m[1][0] * m.m[2][2] * m.m[3][3]
		- m.m[1][2] * m.m[2][3] * m.m[3][0]
		- m.m[1][3] * m.m[2][0] * m.m[3][2]

		+ m.m[1][3] * m.m[2][2] * m.m[3][0]
		+ m.m[1][2] * m.m[2][0] * m.m[3][3]
		+ m.m[1][0] * m.m[2][3] * m.m[3][2];

	ans.m[1][1] =
		m.m[0][0] * m.m[2][2] * m.m[3][3]
		+ m.m[0][2] * m.m[2][3] * m.m[3][0]
		+ m.m[0][3] * m.m[2][0] * m.m[3][2]

		- m.m[0][3] * m.m[2][2] * m.m[3][0]
		- m.m[0][2] * m.m[2][0] * m.m[3][3]
		- m.m[0][0] * m.m[2][3] * m.m[3][2];

	ans.m[1][2] =
		-m.m[0][0] * m.m[1][2] * m.m[3][3]
		- m.m[0][2] * m.m[1][3] * m.m[3][0]
		- m.m[0][3] * m.m[1][0] * m.m[3][2]

		+ m.m[0][3] * m.m[1][2] * m.m[3][0]
		+ m.m[0][2] * m.m[1][0] * m.m[3][3]
		+ m.m[0][0] * m.m[1][3] * m.m[3][2];

	ans.m[1][3] =
		+m.m[0][0] * m.m[1][2] * m.m[2][3]
		+ m.m[0][2] * m.m[1][3] * m.m[2][0]
		+ m.m[0][3] * m.m[1][0] * m.m[2][2]

		- m.m[0][3] * m.m[1][2] * m.m[2][0]
		- m.m[0][2] * m.m[1][0] * m.m[2][3]
		- m.m[0][0] * m.m[1][3] * m.m[2][2];



	ans.m[2][0] =
		m.m[1][0] * m.m[2][1] * m.m[3][3]
		+ m.m[1][1] * m.m[2][3] * m.m[3][0]
		+ m.m[1][3] * m.m[2][0] * m.m[3][1]

		- m.m[1][3] * m.m[2][1] * m.m[3][0]
		- m.m[1][1] * m.m[2][0] * m.m[3][3]
		- m.m[1][0] * m.m[2][3] * m.m[3][1];

	ans.m[2][1] =
		-m.m[0][0] * m.m[2][1] * m.m[3][3]
		- m.m[0][1] * m.m[2][3] * m.m[3][0]
		- m.m[0][3] * m.m[2][0] * m.m[3][1]

		+ m.m[0][3] * m.m[2][1] * m.m[3][0]
		+ m.m[0][1] * m.m[2][0] * m.m[3][3]
		+ m.m[0][0] * m.m[2][3] * m.m[3][1];

	ans.m[2][2] =
		+m.m[0][0] * m.m[1][1] * m.m[3][3]
		+ m.m[0][1] * m.m[1][3] * m.m[3][0]
		+ m.m[0][3] * m.m[1][0] * m.m[3][1]

		- m.m[0][3] * m.m[1][1] * m.m[3][0]
		- m.m[0][1] * m.m[1][0] * m.m[3][3]
		- m.m[0][0] * m.m[1][3] * m.m[3][1];

	ans.m[2][3] =
		-m.m[0][0] * m.m[1][1] * m.m[2][3]
		- m.m[0][1] * m.m[1][3] * m.m[2][0]
		- m.m[0][3] * m.m[1][0] * m.m[2][1]

		+ m.m[0][3] * m.m[1][1] * m.m[2][0]
		+ m.m[0][1] * m.m[1][0] * m.m[2][3]
		+ m.m[0][0] * m.m[1][3] * m.m[2][1];




	ans.m[3][0] =
		-m.m[1][0] * m.m[2][1] * m.m[3][2]
		- m.m[1][1] * m.m[2][2] * m.m[3][0]
		- m.m[1][2] * m.m[2][0] * m.m[3][1]

		+ m.m[1][2] * m.m[2][1] * m.m[3][0]
		+ m.m[1][1] * m.m[2][0] * m.m[3][2]
		+ m.m[1][0] * m.m[2][2] * m.m[3][1];

	ans.m[3][1] =
		m.m[0][0] * m.m[2][1] * m.m[3][2]
		+ m.m[0][1] * m.m[2][2] * m.m[3][0]
		+ m.m[0][2] * m.m[2][0] * m.m[3][1]

		- m.m[0][2] * m.m[2][1] * m.m[3][0]
		- m.m[0][1] * m.m[2][0] * m.m[3][2]
		- m.m[0][0] * m.m[2][2] * m.m[3][1];

	ans.m[3][2] =
		-m.m[0][0] * m.m[1][1] * m.m[3][2]
		- m.m[0][1] * m.m[1][2] * m.m[3][0]
		- m.m[0][2] * m.m[1][0] * m.m[3][1]

		+ m.m[0][2] * m.m[1][1] * m.m[3][0]
		+ m.m[0][1] * m.m[1][0] * m.m[3][2]
		+ m.m[0][0] * m.m[1][2] * m.m[3][1];

	ans.m[3][3] =
		+m.m[0][0] * m.m[1][1] * m.m[2][2]
		+ m.m[0][1] * m.m[1][2] * m.m[2][0]
		+ m.m[0][2] * m.m[1][0] * m.m[2][1]

		- m.m[0][2] * m.m[1][1] * m.m[2][0]
		- m.m[0][1] * m.m[1][0] * m.m[2][2]
		- m.m[0][0] * m.m[1][2] * m.m[2][1];


	for (int i = 0;i < 4;i++) {
		for (int j = 0;j < 4;j++) {
			ans.m[i][j] = ans.m[i][j] * 1 / A;
		}
	}
	return ans;
}


Matrix4x4 MakeOrthographicMatrix(float left, float top, float right, float bottom, float nearClip, float farClip) {
	Matrix4x4 ans = { 0 };
	ans.m[0][0] = 2 / (right - left);
	ans.m[1][1] = 2 / (top - bottom);
	ans.m[2][2] = 1 / (farClip - nearClip);
	ans.m[3][3] = 1;
	ans.m[3][0] = (left + right) / (left - right);
	ans.m[3][1] = (top + bottom) / (bottom - top);
	ans.m[3][2] = nearClip / (nearClip - farClip);
	return ans;

}



ModelData LoadObjFile(const std::string& directoryPath, const std::string& filename)
{
	ModelData modelData;
	std::vector<Vector4> positions;
	std::vector<Vector3> normals;
	std::vector<Vector2> texcoords;
	std::string line;

	std::ifstream file(directoryPath + "/" + filename);
	assert(file.is_open());

	while (std::getline(file, line)) {
		std::string identifier;
		std::istringstream s(line);
		s >> identifier;

		if (identifier == "v") {
			Vector4 position;
			s >> position.x >> position.y >> position.z;
			position.w = 1.0f;
			positions.push_back(position);
		}
		else if (identifier == "vt") {
			Vector2 texcoord;
			s >> texcoord.x >> texcoord.y;
			texcoords.push_back(texcoord);
		}
		else if (identifier == "vn") {
			Vector3 normal;
			s >> normal.x >> normal.y >> normal.z;
			//normal.x *= -1;
			normals.push_back(normal);
		}
		else if (identifier == "f") {
			VertexData triangle[3];

			for (int32_t faceVertex = 0;faceVertex < 3;++faceVertex) {
				std::string vertexDefinition;
				s >> vertexDefinition;

				std::istringstream v(vertexDefinition);
				uint32_t elementIndices[3];
				for (int32_t element = 0;element < 3;++element) {
					std::string index;
					std::getline(v, index, '/');
					elementIndices[element] = std::stoi(index);
				}

				Vector4 posision = positions[elementIndices[0] - 1];
				Vector2 texcoord = texcoords[elementIndices[1] - 1];
				Vector3 normal = normals[elementIndices[2] - 1];

				texcoord.y = 1.0f - texcoord.y;
				posision.x *= -1;
				normal.x *= -1;
				//VertexData vertex = { posision,texcoord,normal };
				//modelData.vertices.push_back(vertex);


				triangle[faceVertex] = { posision,texcoord,normal };
			}
			modelData.vertices.push_back(triangle[2]);
			modelData.vertices.push_back(triangle[1]);
			modelData.vertices.push_back(triangle[0]);
		}
		else if (identifier == "mtllib") {
			std::string materialFilename;
			s >> materialFilename;

			modelData.material = LoadMaterialTemplateFile(directoryPath, materialFilename);
		}
	}
	return modelData;
}

MaterialData LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& filename)
{
	MaterialData materialData;
	std::string line;
	std::ifstream file(directoryPath + "/" + filename);
	assert(file.is_open());

	while (std::getline(file, line)) {
		std::string identifier;
		std::istringstream s(line);

		s >> identifier;

		if (identifier == "map_kd") {
			std::string textureFilename;
			s >> textureFilename;

			materialData.textureFilePath = directoryPath + "/" + textureFilename;
		}

	}
	return materialData;
}



