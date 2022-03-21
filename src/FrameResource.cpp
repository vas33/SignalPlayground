//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

#include "stdafx.h"
#include "FrameResource.h"

FrameResource::FrameResource(ID3D12Device* pDevice, std::vector<InstanceData>& instances) :
	m_fenceValue(0)
{
	m_modelMatrices.resize(1);

	m_instanceCount = instances.size();

	// The command allocator is used by the main sample class when 
	// resetting the command list in the main update loop. Each frame 
	// resource needs a command allocator because command allocators 
	// cannot be reused until the GPU is done executing the commands 
	// associated with it.
	ThrowIfFailed(pDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocator)));
	ThrowIfFailed(pDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_BUNDLE, IID_PPV_ARGS(&m_bundleAllocator)));

	// Create an upload heap for the constant buffers.
	ThrowIfFailed(pDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(sizeof(SceneConstantBuffer)),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&m_cbvUploadHeap)));

	// Map the constant buffers. Note that unlike D3D11, the resource 
	// does not need to be unmapped for use by the GPU. In this sample, 
	// the resource stays 'permenantly' mapped to avoid overhead with 
	// mapping/unmapping each frame.
	CD3DX12_RANGE readRange(0, 0);        // We do not intend to read from this resource on the CPU.
	ThrowIfFailed(m_cbvUploadHeap->Map(0, &readRange, reinterpret_cast<void**>(&m_pConstantBuffers)));


	//create upload heap for Instance data
	// Create an upload heap for the constant buffers.
	ThrowIfFailed(pDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(sizeof(InstanceData) * instances.size()),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&m_cbvUploadHeapInstances)));

	CD3DX12_RANGE readRange2(0, 0);        // We do not intend to read from this resource on the CPU.
	ThrowIfFailed(m_cbvUploadHeapInstances->Map(0, &readRange2, reinterpret_cast<void**>(&m_pInstanceData)));

	// Update all of the model matrices once; our cities don't move so 
	// we don't need to do this ever again.
	SetCityPositions(8.0f, -8.0f);
}

FrameResource::~FrameResource()
{
	m_cbvUploadHeap->Unmap(0, nullptr);
	m_cbvUploadHeapInstances->Unmap(0, nullptr);
	m_pConstantBuffers = nullptr;
	m_pInstanceData = nullptr;
}

void FrameResource::InitBundle(ID3D12Device* pDevice, ID3D12PipelineState* pPso1, ID3D12PipelineState* pPso2,
	UINT frameResourceIndex, UINT numIndices, D3D12_INDEX_BUFFER_VIEW* pIndexBufferViewDesc, D3D12_VERTEX_BUFFER_VIEW* pVertexBufferViewDesc,
	ID3D12DescriptorHeap* pCbvSrvDescriptorHeap, UINT cbvSrvDescriptorSize, ID3D12DescriptorHeap* pSamplerDescriptorHeap, ID3D12RootSignature* pRootSignature)
{
	ThrowIfFailed(pDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_BUNDLE, m_bundleAllocator.Get(), pPso1, IID_PPV_ARGS(&m_bundle)));
	NAME_D3D12_OBJECT(m_bundle);

	PopulateCommandList(m_bundle.Get(), pPso1, pPso2, frameResourceIndex, numIndices, pIndexBufferViewDesc,
		pVertexBufferViewDesc, pCbvSrvDescriptorHeap, cbvSrvDescriptorSize, pSamplerDescriptorHeap, pRootSignature);

	ThrowIfFailed(m_bundle->Close());
}

void FrameResource::SetCityPositions(FLOAT intervalX, FLOAT intervalZ)
{
	XMStoreFloat4x4(&m_modelMatrices[0], XMMatrixTranslation(0, 0, 0));
}

void FrameResource::PopulateCommandList(ID3D12GraphicsCommandList* pCommandList, ID3D12PipelineState* pPso1, ID3D12PipelineState* pPso2,
	UINT frameResourceIndex, UINT numIndices, D3D12_INDEX_BUFFER_VIEW* pIndexBufferViewDesc, D3D12_VERTEX_BUFFER_VIEW* pVertexBufferViewDesc,
	ID3D12DescriptorHeap* pCbvSrvDescriptorHeap, UINT cbvSrvDescriptorSize, ID3D12DescriptorHeap* pSamplerDescriptorHeap, ID3D12RootSignature* pRootSignature)
{
	// If the root signature matches the root signature of the caller, then
	// bindings are inherited, otherwise the bind space is reset.
	pCommandList->SetGraphicsRootSignature(pRootSignature);

	ID3D12DescriptorHeap* ppHeaps[] = { pCbvSrvDescriptorHeap, pSamplerDescriptorHeap };
	pCommandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
	pCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	pCommandList->IASetIndexBuffer(pIndexBufferViewDesc);
	pCommandList->IASetVertexBuffers(0, 1, pVertexBufferViewDesc);
	pCommandList->SetGraphicsRootDescriptorTable(0, pCbvSrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
	pCommandList->SetGraphicsRootDescriptorTable(1, pSamplerDescriptorHeap->GetGPUDescriptorHandleForHeapStart());

	// Calculate the descriptor offset due to multiple frame resources.
	// 1 SRV + how many CBVs we have currently.
	UINT frameResourceDescriptorOffset = 1 /*+ (frameResourceIndex)*/;
	CD3DX12_GPU_DESCRIPTOR_HANDLE cbvSrvHandle(pCbvSrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart(), frameResourceDescriptorOffset, cbvSrvDescriptorSize);
	

	PIXBeginEvent(pCommandList, 0, L"Draw cities");
	BOOL usePso1 = TRUE;
	{
		// Alternate which PSO to use; the pixel shader is different on 
		// each just as a PSO setting demonstration.
		pCommandList->SetPipelineState(usePso1 ? pPso1 : pPso2);
		usePso1 = !usePso1;

		// Set this city's CBV table and move to the next descriptor.
		pCommandList->SetGraphicsRootDescriptorTable(2, cbvSrvHandle);
		//TODO set desrtiptor table for instances here
		
		pCommandList->SetGraphicsRootShaderResourceView(3, m_cbvUploadHeapInstances->GetGPUVirtualAddress());

		pCommandList->DrawIndexedInstanced(numIndices, m_instanceCount, 0, 0, 0);
	}
	PIXEndEvent(pCommandList);
}

void XM_CALLCONV FrameResource::UpdateInstanceData(std::vector<InstanceData>& instanceData)
{
	for (int index = 0; index < instanceData.size(); ++index)
	{
		memcpy(&m_pInstanceData[index], &instanceData[index].Position, sizeof(XMFLOAT4));
	}
}


void XM_CALLCONV FrameResource::UpdateConstantBuffers(FXMMATRIX view, CXMMATRIX projection)
{
	XMMATRIX model;
	XMFLOAT4X4 mvp;

	model = XMLoadFloat4x4(&m_modelMatrices[0]);

	// Compute the model-view-projection matrix.
	XMStoreFloat4x4(&mvp, XMMatrixTranspose(model * view * projection));

	// Copy this matrix into the appropriate location in the upload heap subresource.
	memcpy(&m_pConstantBuffers[0], &mvp, sizeof(mvp));

}
