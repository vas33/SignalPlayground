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

#pragma once

#include "DXSampleHelper.h"

using namespace DirectX;
using Microsoft::WRL::ComPtr;

struct InstanceData
{
    DirectX::XMFLOAT4 Position = { 0.0f, 0.0f, 0.0f, 0.0f };
};

class FrameResource
{
private:
    void SetCityPositions(FLOAT intervalX, FLOAT intervalZ);

public:
    struct SceneConstantBuffer
    {
        XMFLOAT4X4 mvp;        // Model-view-projection (MVP) matrix.
        FLOAT padding[48];
    };



    ComPtr<ID3D12CommandAllocator> m_commandAllocator;
    ComPtr<ID3D12CommandAllocator> m_bundleAllocator;
    ComPtr<ID3D12GraphicsCommandList> m_bundle;
    ComPtr<ID3D12Resource> m_cbvUploadHeap;
    ComPtr<ID3D12Resource> m_cbvUploadHeapInstances;
    SceneConstantBuffer* m_pConstantBuffers;
    InstanceData* m_pInstanceData;
    unsigned int m_instanceCount{ 0 };
    UINT64 m_fenceValue;

    std::vector<XMFLOAT4X4> m_modelMatrices;

    FrameResource(ID3D12Device* pDevice, std::vector<InstanceData>& instances);
    ~FrameResource();

    void InitBundle(ID3D12Device* pDevice, ID3D12PipelineState* pPso1, ID3D12PipelineState* pPso2,
        UINT frameResourceIndex, UINT numIndices, D3D12_INDEX_BUFFER_VIEW* pIndexBufferViewDesc, D3D12_VERTEX_BUFFER_VIEW* pVertexBufferViewDesc,
        ID3D12DescriptorHeap* pCbvSrvDescriptorHeap, UINT cbvSrvDescriptorSize, ID3D12DescriptorHeap* pSamplerDescriptorHeap, ID3D12RootSignature* pRootSignature);

    void PopulateCommandList(ID3D12GraphicsCommandList* pCommandList, ID3D12PipelineState* pPso1, ID3D12PipelineState* pPso2,
        UINT frameResourceIndex, UINT numIndices, D3D12_INDEX_BUFFER_VIEW* pIndexBufferViewDesc, D3D12_VERTEX_BUFFER_VIEW* pVertexBufferViewDesc,
        ID3D12DescriptorHeap* pCbvSrvDescriptorHeap, UINT cbvSrvDescriptorSize, ID3D12DescriptorHeap* pSamplerDescriptorHeap, ID3D12RootSignature* pRootSignature);

    void XM_CALLCONV UpdateConstantBuffers(FXMMATRIX view, CXMMATRIX projection);
    void XM_CALLCONV UpdateInstanceData(std::vector<InstanceData>& instanceData);
};
