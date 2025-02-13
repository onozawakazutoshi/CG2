#pragma once
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>
#include "WinApp.h"
#include "Logger.h"
#include <combaseapi.h>
#include <dxcapi.h>
#include "externals/DirectXTex/DirectXTex.h"
#include <cmath>
#include "StringUtility.h"
#include "Input.h"
#include <chrono>


class DirectXCommon
{
public:
	DirectXCommon();
	~DirectXCommon();
	void Initialize(WinApp* winapp);
	void ComInitialize();
	void SwapChainInitialize();
	void depthStencil();
	Microsoft::WRL::ComPtr<ID3D12Resource> CreateDepthStencilTextureResource(int32_t width, int32_t height);
	Microsoft::WRL::ComPtr <ID3D12DescriptorHeap> CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDescriptors, bool shaderVisible);

	void DescriptorHeap();

	Microsoft::WRL::ComPtr < IDxcBlob> CompileShader(
		const std::wstring& filePath,
		const wchar_t* profile
	);
	void RenderViwe();

	void ImGuiInitialize();

	void TextureSrv();

	void Fence();
	void Viewport();

	void DXCcom();

	void Scissor();

	void PreDraw();

	void PostDraw();

	

	DirectX::ScratchImage LoadTexture(const std::string& filePath);

	
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> GetcommandList() {
		return commandList;
	}
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> GetcommandQueue() {
		return commandQueue;
	}
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> GetcommandAllocator() {
		return commandAllocator;
	}
	Microsoft::WRL::ComPtr <IDXGISwapChain4> GetswapChain() {
		return swapChain;
	}
	Microsoft::WRL::ComPtr < ID3D12DescriptorHeap> GetrtvDescriptorHeap() {
		return rtvDescriptorHeap;
	}
	D3D12_CPU_DESCRIPTOR_HANDLE GetrtvStartHandle() {
		return rtvStartHandle;
	}
	D3D12_CPU_DESCRIPTOR_HANDLE& GetrtvHandles(int i) {
		return rtvHandles[i];
	}
	Microsoft::WRL::ComPtr < ID3D12DescriptorHeap>GetdsvDescriptorHeap() {
		return dsvDescriptorHeap;
	}
	Microsoft::WRL::ComPtr < ID3D12Resource> GetswapChainResources(int i) {
		return swapChainResources[i];
	}

	Microsoft::WRL::ComPtr < ID3D12DescriptorHeap> GetsrvDescriptorHeap() {
		return srvDescriptorHeap;
	}
	D3D12_VIEWPORT& Getviewport() {
		return viewport;
	}
	D3D12_RECT &GetscissorRect() {
		return scissorRect;
	}
	Microsoft::WRL::ComPtr <ID3D12Resource> GettextureResource() {
		return textureResource;
	}
	
	
	Microsoft::WRL::ComPtr < ID3D12Fence> Getfence() {
		return fence;
	}
	uint64_t GetfenceValue() {
		return fenceValue;
	}
	HANDLE GetfenceEvent() {
		return fenceEvent;
	}
	
	Microsoft::WRL::ComPtr<ID3D12Device> Getdevice() {
		return device;
	}

	HRESULT Gethr() {
		return hr;
	}
	uint32_t GetdesriptorSizeSRV() {
		return desriptorSizeSRV;
	}
	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(uint32_t index);

	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(uint32_t index);
	Microsoft::WRL::ComPtr <IDxcIncludeHandler> GetincludeHandler() {
		return includeHandler;
	}
	Microsoft::WRL::ComPtr <IDxcUtils>GetdxcUtils() {
		return dxcUtils;
	}
	Microsoft::WRL::ComPtr <IDxcCompiler3> GetdxcCompiler() {
		return dxcCompiler;
	}

	Microsoft::WRL::ComPtr < ID3D12Resource> CreateBufferResource(size_t sizeInBytes);
	Microsoft::WRL::ComPtr<ID3D12Resource> CreateTextureResource(const DirectX::TexMetadata& metadata);
	void UploadTextureData(Microsoft::WRL::ComPtr < ID3D12Resource> texture, const DirectX::ScratchImage& mipImages);


private:

	void InitializeFixFPS();

	void UpdateFixFPS();

	std::chrono::steady_clock::time_point reference_;

	Microsoft::WRL::ComPtr<ID3D12Device> device = nullptr;
	Microsoft::WRL::ComPtr<IDXGIFactory7> dxgiFactory;
	D3D12_COMMAND_QUEUE_DESC commandQueueDesc{};
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue = nullptr;
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator = nullptr;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList = nullptr;
	WinApp* winapp;
	HRESULT hr;
	
	Microsoft::WRL::ComPtr < ID3D12Resource> swapChainResources[2] = { nullptr };
	Microsoft::WRL::ComPtr < ID3D12Resource> depthStencilResource;
	Microsoft::WRL::ComPtr < ID3D12DescriptorHeap> rtvDescriptorHeap;
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};

	Microsoft::WRL::ComPtr <IDXGISwapChain4> swapChain = nullptr;
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};

	Microsoft::WRL::ComPtr < ID3D12DescriptorHeap> srvDescriptorHeap;

	Microsoft::WRL::ComPtr < ID3D12DescriptorHeap>dsvDescriptorHeap;
	
	Microsoft::WRL::ComPtr <IDxcIncludeHandler> includeHandler = nullptr;

	D3D12_CPU_DESCRIPTOR_HANDLE rtvStartHandle;

	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[2];
	D3D12_VIEWPORT viewport{};

	D3D12_RECT scissorRect{};

	Microsoft::WRL::ComPtr <IDxcUtils>dxcUtils = nullptr;
	Microsoft::WRL::ComPtr <IDxcCompiler3> dxcCompiler = nullptr;

	Microsoft::WRL::ComPtr <ID3D12Resource> textureResource;

	IDxcBlobUtf8* shaderError = nullptr;

	Microsoft::WRL::ComPtr < ID3D12Fence> fence = nullptr;
	uint64_t fenceValue = 0;
	HANDLE fenceEvent;


	//Input* input;
	

	D3D12_RESOURCE_BARRIER barrier{};
	UINT backBufferIndex = NULL;

	 uint32_t desriptorSizeSRV =NULL;
	 uint32_t desriptorSizeRTV = NULL;
	 uint32_t desriptorSizeDSV = NULL;
	
};

