#include "DirectXCommon.h"
#include "StringUtility.h"
#include <cassert>
#include "externals/DirectXTex/DirectXTex.h"
#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")
#include "Logger.h"
#include <format>
#include <thread>




using namespace Microsoft::WRL;
DirectXCommon::DirectXCommon()
{
}
DirectXCommon::~DirectXCommon()
{
	
	//delete input;
}
void DirectXCommon::Initialize(WinApp* winapp_)
{
	
	winapp = winapp_;
	assert(winapp);

	InitializeFixFPS();
	
	ComInitialize();//コマンド
	SwapChainInitialize();//スワップチェーン
	depthStencil();//深層バッファ
	
	
	DescriptorHeap();//デスクリプタヒープ
	TextureSrv();//テクスチャ
	RenderViwe();//レンダービュー
	
	

	Fence();//フェンス
	Viewport();//ビューポート
	Scissor();//シザリング
	DXCcom();//DXC
	//input = new Input;
	//input->Initialize(winapp);

	ImGuiInitialize();//ImGuiの初期化
}

void DirectXCommon::ComInitialize()
{
	dxgiFactory = nullptr;

	hr = CreateDXGIFactory(IID_PPV_ARGS(&dxgiFactory));

	assert(SUCCEEDED(hr));
	Microsoft::WRL::ComPtr <IDXGIAdapter4> useAdapter = nullptr;

	for (UINT i = 0; dxgiFactory->EnumAdapterByGpuPreference(
		i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&useAdapter)) !=
		DXGI_ERROR_NOT_FOUND;++i) {

		DXGI_ADAPTER_DESC3 adapterDesc{};
		hr = useAdapter->GetDesc3(&adapterDesc);
		assert(SUCCEEDED(hr));

		if (!(adapterDesc.Flags & DXGI_ADAPTER_FLAG3_SOFTWARE)) {
			Logger::Log(StringUtility::ConvertString(std::format(L"Use Adapater:{}\n", adapterDesc.Description)));
			break;
		}
		useAdapter = nullptr;
	}
	assert(useAdapter != nullptr);

	D3D_FEATURE_LEVEL featureLevels[] = {
		D3D_FEATURE_LEVEL_12_2,D3D_FEATURE_LEVEL_12_1,D3D_FEATURE_LEVEL_12_0
	};
	const char* featureLevelStrings[] = { "12.2","12.1","12.0" };

	for (size_t i = 0;i < _countof(featureLevels);++i) {
		hr = D3D12CreateDevice(useAdapter.Get(), featureLevels[i], IID_PPV_ARGS(&device));

		if (SUCCEEDED(hr)) {
			Logger::Log(std::format("FeatureLevel : {}\n", featureLevelStrings[i]));
			break;
		}
	}


#ifdef _DEBUG
	Microsoft::WRL::ComPtr <ID3D12InfoQueue> infoQueue = nullptr;
	if (SUCCEEDED(device->QueryInterface(IID_PPV_ARGS(&infoQueue)))) {
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);
		//infoQueue->Release();

		D3D12_MESSAGE_ID denyIds[] = {
			D3D12_MESSAGE_ID_RESOURCE_BARRIER_MISMATCHING_COMMAND_LIST_TYPE
		};

		D3D12_MESSAGE_SEVERITY severities[] = { D3D12_MESSAGE_SEVERITY_INFO };
		D3D12_INFO_QUEUE_FILTER filter{};
		filter.DenyList.NumIDs = _countof(denyIds);
		filter.DenyList.pIDList = denyIds;
		filter.DenyList.NumSeverities = _countof(severities);
		filter.DenyList.pSeverityList = severities;

		infoQueue->PushStorageFilter(&filter);
	}



#endif 

	assert(device != nullptr);
	hr = device->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&commandQueue));
	assert(SUCCEEDED(hr));

	hr = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator));
	assert(SUCCEEDED(hr));

	
	hr = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator.Get(), nullptr, IID_PPV_ARGS(&commandList));
	assert(SUCCEEDED(hr));
}

void DirectXCommon::SwapChainInitialize()
{
	
	swapChainDesc.Width = winapp->kClientWidth;
	swapChainDesc.Height = winapp->kClientHeight;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 2;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

	hr = dxgiFactory->CreateSwapChainForHwnd(commandQueue.Get(), winapp->hwnd_, &swapChainDesc, nullptr, nullptr, reinterpret_cast<IDXGISwapChain1**>(swapChain.GetAddressOf()));
	assert(SUCCEEDED(hr));

	
	hr = swapChain->GetBuffer(0, IID_PPV_ARGS(&swapChainResources[0]));
	assert(SUCCEEDED(hr));
	hr = swapChain->GetBuffer(1, IID_PPV_ARGS(&swapChainResources[1]));
	assert(SUCCEEDED(hr));

}

void DirectXCommon::depthStencil()
{
	
	depthStencilResource = CreateDepthStencilTextureResource(device, winapp->kClientWidth, winapp->kClientHeight);
	//////////////////////////////////////////////////////////////////////////////
	Microsoft::WRL::ComPtr < ID3D12DescriptorHeap>dsvDescriptorHeap = CreateDescriptorHeap(device.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1, false);

	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;

	device->CreateDepthStencilView(depthStencilResource.Get(), &dsvDesc, dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
}

Microsoft::WRL::ComPtr<ID3D12Resource> DirectXCommon::CreateDepthStencilTextureResource(Microsoft::WRL::ComPtr<ID3D12Device> device, int32_t width, int32_t height)
{
	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Width = width;
	resourceDesc.Height = height;
	resourceDesc.MipLevels = 1;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;

	D3D12_CLEAR_VALUE depthCllearValue{};
	depthCllearValue.DepthStencil.Depth = 1.0f;
	depthCllearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	Microsoft::WRL::ComPtr <ID3D12Resource> resource = nullptr;


	HRESULT hr = device->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAG_NONE,
		&resourceDesc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&depthCllearValue,
		IID_PPV_ARGS(&resource)
	);
	assert(SUCCEEDED(hr));


	return resource;
}

Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> DirectXCommon::CreateDescriptorHeap(Microsoft::WRL::ComPtr<ID3D12Device> device, D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDescriptors, bool shaderVisible)
{
	Microsoft::WRL::ComPtr <ID3D12DescriptorHeap> descriptorHeap = nullptr;
	D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc{};
	descriptorHeapDesc.Type = heapType;
	descriptorHeapDesc.NumDescriptors = numDescriptors;
	descriptorHeapDesc.Flags = shaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	HRESULT hr = device->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&descriptorHeap));
	assert(SUCCEEDED(hr));
	return descriptorHeap;
}

void DirectXCommon::DescriptorHeap()
{
	rtvDescriptorHeap = CreateDescriptorHeap(device.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 2, false);
	D3D12_DESCRIPTOR_HEAP_DESC rtvDescriptorHeapDesc{};
	rtvDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvDescriptorHeapDesc.NumDescriptors = 2;
	hr = device->CreateDescriptorHeap(&rtvDescriptorHeapDesc, IID_PPV_ARGS(&rtvDescriptorHeap));
	assert(SUCCEEDED(hr));

	
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

	

	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	
	dsvDescriptorHeap = CreateDescriptorHeap(device.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1, false);

	device->CreateDepthStencilView(depthStencilResource.Get(), &dsvDesc, dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

	desriptorSizeSRV = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	desriptorSizeRTV = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	desriptorSizeDSV = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

	GetCPUDescriptorHandle(rtvDescriptorHeap.Get(), desriptorSizeRTV, 0);
	srvDescriptorHeap = CreateDescriptorHeap(device.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 128, true);

	
}

Microsoft::WRL::ComPtr<ID3D12Resource> DirectXCommon::CreateBufferResource(Microsoft::WRL::ComPtr<ID3D12Device> device, size_t sizeInBytes)
{
	D3D12_HEAP_PROPERTIES uploadHeapProperties{};
	uploadHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;

	D3D12_RESOURCE_DESC vertexResourceDesc{};
	vertexResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	vertexResourceDesc.Width = sizeInBytes;

	vertexResourceDesc.Height = 1;
	vertexResourceDesc.DepthOrArraySize = 1;
	vertexResourceDesc.MipLevels = 1;
	vertexResourceDesc.SampleDesc.Count = 1;

	vertexResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	Microsoft::WRL::ComPtr <ID3D12Resource> vertexResource;
	device->CreateCommittedResource(&uploadHeapProperties, D3D12_HEAP_FLAG_NONE,
		&vertexResourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&vertexResource));

	return vertexResource;
}

Microsoft::WRL::ComPtr<ID3D12Resource> DirectXCommon::CreateTextureResource(Microsoft::WRL::ComPtr<ID3D12Device> device, const DirectX::TexMetadata& metadata)
{
	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Width = UINT(metadata.width);
	resourceDesc.Height = UINT(metadata.height);
	resourceDesc.MipLevels = UINT16(metadata.mipLevels);
	resourceDesc.DepthOrArraySize = UINT16(metadata.arraySize);
	resourceDesc.Format = metadata.format;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION(metadata.dimension);

	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_CUSTOM;
	heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;
	heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;

	Microsoft::WRL::ComPtr < ID3D12Resource> resource = nullptr;


	HRESULT hr = device->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAG_NONE,
		&resourceDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&resource));
	assert(SUCCEEDED(hr));
	return resource;
}

Microsoft::WRL::ComPtr<IDxcBlob> DirectXCommon::CompileShader(const std::wstring& filePath, const wchar_t* profile, Microsoft::WRL::ComPtr<IDxcUtils> dxccUtils, Microsoft::WRL::ComPtr<IDxcCompiler3> dxcCompiler, Microsoft::WRL::ComPtr<IDxcIncludeHandler> includeHandler)
{
	Logger::Log(StringUtility::ConvertString(std::format(L"Begin CompileShader, path:{},profile:{}\n", filePath, profile)));

	Microsoft::WRL::ComPtr<IDxcBlobEncoding> shaderSource = nullptr;

	HRESULT hr = dxccUtils->LoadFile(filePath.c_str(), nullptr, &shaderSource);

	assert(SUCCEEDED(hr));

	DxcBuffer shaderSourceBuffer;
	shaderSourceBuffer.Ptr = shaderSource->GetBufferPointer();
	shaderSourceBuffer.Size = shaderSource->GetBufferSize();
	shaderSourceBuffer.Encoding = DXC_CP_UTF8;

	LPCWSTR arguments[] = {
		filePath.c_str(),
		L"-E",L"main",
		L"-T",profile,
		L"-Zi",L"-Qembed_debug",
		L"-Od",
		L"-Zpr",
	};

	Microsoft::WRL::ComPtr <IDxcResult> shaderResult = nullptr;
	hr = dxcCompiler->Compile(
		&shaderSourceBuffer,
		arguments,
		_countof(arguments),
		includeHandler.Get(),
		IID_PPV_ARGS(&shaderResult)
	);
	assert(SUCCEEDED(hr));

	IDxcBlobUtf8* shaderError = nullptr;
	shaderResult->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&shaderError), nullptr);
	if (shaderError != nullptr && shaderError->GetStringLength() != 0) {
		Logger::Log(shaderError->GetStringPointer());
		assert(false);
	}

	Microsoft::WRL::ComPtr < IDxcBlob> shaderBlob = nullptr;
	hr = shaderResult->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&shaderBlob), nullptr);
	assert(SUCCEEDED(hr));

	Logger::Log(StringUtility::ConvertString(std::format(L"Compile Succeeded,path:{},profile:{}\n", filePath, profile)));

	//shaderSource->Release();
	//shaderResult->Release();

	return shaderBlob;
}

void DirectXCommon::RenderViwe()
{
	rtvStartHandle = rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	rtvHandles[0] = rtvStartHandle;
	device->CreateRenderTargetView(swapChainResources[0].Get(), &rtvDesc, rtvHandles[0]);
	rtvHandles[1].ptr = rtvHandles[0].ptr + device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	device->CreateRenderTargetView(swapChainResources[1].Get(), &rtvDesc, rtvHandles[1]);

	/*for (uint32_t i = 0;i < 2;i++) {

	}*/
}

void DirectXCommon::ImGuiInitialize()
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGui_ImplWin32_Init(winapp->hwnd_);
	ImGui_ImplDX12_Init(
		device.Get(),
		swapChainDesc.BufferCount,
		rtvDesc.Format,
		srvDescriptorHeap.Get(),
		srvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
		srvDescriptorHeap->GetGPUDescriptorHandleForHeapStart()
	);
}

void DirectXCommon::TextureSrv()
{
	DirectX::ScratchImage mipImages = LoadTexture("resources/uvChecker.png");
	const DirectX::TexMetadata& metadata = mipImages.GetMetadata();
	textureResource = CreateTextureResource(device, metadata);

	UploadTextureData(textureResource.Get(), mipImages);
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = metadata.format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = UINT(metadata.mipLevels);

	
}

void DirectXCommon::Fence()
{
	
	hr = device->CreateFence(fenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
	assert(SUCCEEDED(hr));

	fenceEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	assert(fenceEvent != nullptr);
}

void DirectXCommon::Viewport()
{
	

	viewport.Width = (float)winapp->kClientWidth;
	viewport.Height = (float)winapp->kClientHeight;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
}

void DirectXCommon::DXCcom()
{
	
	hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&dxcUtils));
	assert(SUCCEEDED(hr));
	hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&dxcCompiler));
	assert(SUCCEEDED(hr));
	hr = dxcUtils->CreateDefaultIncludeHandler(&includeHandler);
	assert(SUCCEEDED(hr));

	
}

void DirectXCommon::Scissor()
{
	

	scissorRect.left = 0;
	scissorRect.right = winapp->kClientWidth;
	scissorRect.top = 0;
	scissorRect.bottom = winapp->kClientHeight;
}

void DirectXCommon::PreDraw()
{
	backBufferIndex = swapChain->GetCurrentBackBufferIndex();
	commandList->OMSetRenderTargets(1, &rtvHandles[backBufferIndex], false, nullptr);

	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	commandList->OMSetRenderTargets(1, &rtvHandles[backBufferIndex], false, &dsvHandle);

	//input->Updat();

	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = GetswapChainResources(backBufferIndex).Get();
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;




	commandList->ResourceBarrier(1, &barrier);
	float clearColor[] = { 0.1f,0.25f,0.5f,1.0f };
	commandList->ClearRenderTargetView(GetrtvHandles(backBufferIndex), clearColor, 0, nullptr);

	commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	ID3D12DescriptorHeap* descriptorHeaps[] = { GetsrvDescriptorHeap().Get() };
	commandList->SetDescriptorHeaps(1, descriptorHeaps);

	commandList->RSSetViewports(1, &Getviewport());
	commandList->RSSetScissorRects(1, &GetscissorRect());

}

void DirectXCommon::PostDraw()
{
	

	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;



	GetcommandList()->ResourceBarrier(1, &barrier);
	hr = GetcommandList()->Close();
	assert(SUCCEEDED(hr));


	ID3D12CommandList* commandLists[] = { GetcommandList().Get() };
	GetcommandQueue()->ExecuteCommandLists(1, commandLists);
	GetswapChain()->Present(1, 0);

	fenceValue++;
	GetcommandQueue()->Signal(Getfence().Get(), fenceValue);
	if (Getfence()->GetCompletedValue() < fenceValue) {
		Getfence()->SetEventOnCompletion(fenceValue, GetfenceEvent());
		WaitForSingleObject(GetfenceEvent(), INFINITE);
	}

	UpdateFixFPS();

	hr = GetcommandAllocator()->Reset();
	assert(SUCCEEDED(hr));
	hr = GetcommandList()->Reset(GetcommandAllocator().Get(), nullptr);
	assert(SUCCEEDED(hr));
}

DirectX::ScratchImage DirectXCommon::LoadTexture(const std::string& filePath)
{
	DirectX::ScratchImage image{};
	std::wstring filePathW = StringUtility::ConvertString(filePath);
	HRESULT hr = DirectX::LoadFromWICFile(filePathW.c_str(), DirectX::WIC_FLAGS_FORCE_SRGB, nullptr, image);
	assert(SUCCEEDED(hr));

	DirectX::ScratchImage mipImages{};
	hr = DirectX::GenerateMipMaps(image.GetImages(), image.GetImageCount(), image.GetMetadata(), DirectX::TEX_FILTER_SRGB, 0, mipImages);
	assert(SUCCEEDED(hr));

	return mipImages;
}

D3D12_GPU_DESCRIPTOR_HANDLE DirectXCommon::GetGPUDescriptorHandle(Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap, uint32_t descriptorSize, uint32_t index)
{

	D3D12_GPU_DESCRIPTOR_HANDLE handleGPU = descriptorHeap->GetGPUDescriptorHandleForHeapStart();
	handleGPU.ptr += (descriptorSize * index);
	return handleGPU;

}



void DirectXCommon::UploadTextureData(Microsoft::WRL::ComPtr < ID3D12Resource> texture, const DirectX::ScratchImage& mipImages) {
	const DirectX::TexMetadata& metadata = mipImages.GetMetadata();

	for (size_t mipLevel = 0;mipLevel < metadata.mipLevels;++mipLevel) {
		const DirectX::Image* img = mipImages.GetImage(mipLevel, 0, 0);

		HRESULT hr = texture->WriteToSubresource(
			UINT(mipLevel),
			nullptr,
			img->pixels,
			UINT(img->rowPitch),
			UINT(img->slicePitch)
		);
		assert(SUCCEEDED(hr));
	}

}

void DirectXCommon::InitializeFixFPS()
{
	reference_ = std::chrono::steady_clock::now();
}

void DirectXCommon::UpdateFixFPS()
{
	const std::chrono::microseconds kMinTime(uint64_t(1000000.0f / 60.0f));
	const std::chrono::microseconds kMinCheckTime(uint64_t(1000000.0f / 65.0f));

	std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
	std::chrono::microseconds elapsed = std::chrono::duration_cast<std::chrono::microseconds>(now - reference_);


	if (elapsed < kMinTime) {
		while (std::chrono::steady_clock::now() - reference_ < kMinTime) {
			std::this_thread::sleep_for(std::chrono::microseconds(1));
		}
	}
	reference_ = std::chrono::steady_clock::now();



}
