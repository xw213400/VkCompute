#include "ComputeShader.h"
#include "VulkanContext.h"
#include "ComputeBuffer.h"
#include <iostream>
#include <fstream>
#include <array>
#include "UniformData.h"
#include "BindingsTable.h"


ComputeShader::ComputeShader(const std::string &filename, const std::string& kernel)
{
    VkDevice device = VulkanContext::Instance().device;

    std::string shaderFilename = filename.substr(0, filename.length() - 4) + ".spv";
    std::ifstream file(shaderFilename, std::ios::ate | std::ios::binary);

    if (!file.is_open())
    {
        throw std::runtime_error("failed to open file!");
    }

    size_t fileSize = (size_t)file.tellg();
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);

    file.close();

    VkShaderModule computeShaderModule = createShaderModule(buffer);

    // create VkDescriptorSetLayout
    BindingsTable bindings(filename);

    _uniformBindingsCount = 0;
    _storageBingingsCount = 0;

    for (int i = 0; i != bindings.size(); ++i)
    {
        VkDescriptorType descType = bindings.getType(i) == "uniform" ? VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER : VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        addBinding(bindings.getName(i), descType);
    }

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = (uint32_t)_bindings.size();
    layoutInfo.pBindings = _bindings.data();

    if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &_descriptorSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create compute descriptor set layout!");
    }

    VkPipelineShaderStageCreateInfo computeShaderStageInfo{};
    computeShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    computeShaderStageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    computeShaderStageInfo.module = computeShaderModule;
    computeShaderStageInfo.pName = kernel.c_str();

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &_descriptorSetLayout;

    if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &_computePipelineLayout) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create compute pipeline layout!");
    }

    VkComputePipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    pipelineInfo.layout = _computePipelineLayout;
    pipelineInfo.stage = computeShaderStageInfo;

    if (vkCreateComputePipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &_computePipeline) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create compute pipeline!");
    }

    vkDestroyShaderModule(device, computeShaderModule, nullptr);

    createDescriptorSet();
}

void ComputeShader::createDescriptorSet()
{
    VkDevice device = VulkanContext::Instance().device;

    std::array<VkDescriptorPoolSize, 2> poolDataTypes{};
    poolDataTypes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolDataTypes[0].descriptorCount = _uniformBindingsCount;
    poolDataTypes[1].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    poolDataTypes[1].descriptorCount = _storageBingingsCount;

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = (uint32_t)poolDataTypes.size();
    poolInfo.pPoolSizes = poolDataTypes.data();
    poolInfo.maxSets = 1;

    if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &_descriptorPool) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create descriptor pool!");
    }

    VkDescriptorSetAllocateInfo allocateInfo = {};
    allocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocateInfo.descriptorPool = _descriptorPool;
    allocateInfo.descriptorSetCount = 1;
    allocateInfo.pSetLayouts = &_descriptorSetLayout;

    if (vkAllocateDescriptorSets(device, &allocateInfo, &_descriptorSet) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate descriptor sets!");
    }

    uint32_t count = _uniformBindingsCount + _storageBingingsCount;
    _descriptorWrites.resize(count);
}

VkShaderModule ComputeShader::createShaderModule(const std::vector<char> &code)
{
    VkDevice device = VulkanContext::Instance().device;

    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t *>(code.data());

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create shader module!");
    }

    return shaderModule;
}

void ComputeShader::dispatch(int threadGroupsX, int threadGroupsY, int threadGroupsZ)
{
    VkDevice device = VulkanContext::Instance().device;
    vkUpdateDescriptorSets(device, (uint32_t)_descriptorWrites.size(), _descriptorWrites.data(), 0, nullptr);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    VkCommandBuffer cmd = VulkanContext::Instance().getCommandBuffer();

    if (vkBeginCommandBuffer(cmd, &beginInfo) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to begin recording compute command buffer!");
    }

    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, _computePipeline);

    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, _computePipelineLayout, 0, 1, &_descriptorSet, 0, nullptr);

    vkCmdDispatch(cmd, threadGroupsX, threadGroupsY, threadGroupsZ);

    if (vkEndCommandBuffer(cmd) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to record compute command buffer!");
    }
}

void ComputeShader::addBinding(const std::string &name, VkDescriptorType descriptorType)
{
    uint32_t i = (uint32_t)_bindings.size();

    VkDescriptorSetLayoutBinding binding = {};

    binding.binding = i;
    binding.descriptorCount = 1;
    binding.descriptorType = descriptorType;
    binding.pImmutableSamplers = nullptr;
    binding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

    _bindings.push_back(binding);
    _bindingsMap.insert(std::make_pair(name, i));

    if (binding.descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
    {
        _uniformBindingsCount++;
        UniformData::Instance().addUniform(name);
    }
    else if (binding.descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER)
    {
        _storageBingingsCount++;
    }
}

void ComputeShader::setBuffer(const std::string &name, ComputeBuffer *buffer)
{
    auto it = _bindingsMap.find(name);

    if (it == _bindingsMap.end())
    {
        throw std::runtime_error("failed to set buffer!");
    }

    int i = it->second;

    _descriptorWrites[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    _descriptorWrites[i].dstSet = _descriptorSet;
    _descriptorWrites[i].dstBinding = i;
    _descriptorWrites[i].dstArrayElement = 0;
    _descriptorWrites[i].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    _descriptorWrites[i].descriptorCount = 1;
    _descriptorWrites[i].pBufferInfo = buffer->getDescriptor();
}

void ComputeShader::setUniform(const std::string &name, float data)
{
    auto it = _bindingsMap.find(name);

    if (it == _bindingsMap.end())
    {
        throw std::runtime_error("failed to set buffer!");
    }

    Vector4 vec4 = {data, 0.f, 0.f, 0.f};
    UniformData::Instance().setUniform(name, vec4);

    int i = it->second;

    _descriptorWrites[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    _descriptorWrites[i].dstSet = _descriptorSet;
    _descriptorWrites[i].dstBinding = i;
    _descriptorWrites[i].dstArrayElement = 0;
    _descriptorWrites[i].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    _descriptorWrites[i].descriptorCount = 1;
    _descriptorWrites[i].pBufferInfo = UniformData::Instance().getDescriptorBufferInfo();
}

void ComputeShader::release()
{
    VkDevice device = VulkanContext::Instance().device;

    vkDestroyPipeline(device, _computePipeline, nullptr);
    vkDestroyPipelineLayout(device, _computePipelineLayout, nullptr);
    vkDestroyDescriptorSetLayout(device, _descriptorSetLayout, nullptr);
    vkDestroyDescriptorPool(device, _descriptorPool, nullptr);
}
