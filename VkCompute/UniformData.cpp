#include "UniformData.h"
#include "VulkanContext.h"

#define MAX_UNIFORM_COUNT 1024

void UniformData::initialize()
{
    VkDevice device = VulkanContext::Instance().device;
    uint32_t size = MAX_UNIFORM_COUNT * sizeof(Vector4);

    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(device, &bufferInfo, nullptr, &_buffer) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create buffer!");
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device, _buffer, &memRequirements);

    VkMemoryAllocateInfo memInfo{};
    memInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memInfo.allocationSize = memRequirements.size;
    VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    memInfo.memoryTypeIndex = VulkanContext::Instance().findMemoryType(memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(device, &memInfo, nullptr, &_bufferMemory) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate buffer memory!");
    }

    vkBindBufferMemory(device, _buffer, _bufferMemory, 0);

    _descriptorBufferInfo.buffer = _buffer;
    _descriptorBufferInfo.offset = 0;
    _descriptorBufferInfo.range = size;

    vkMapMemory(device, _bufferMemory, 0, size, 0, &_uniformBufferMapped);
}

void UniformData::updateMemory()
{
    memcpy(_uniformBufferMapped, _uniformDatas.data(), _uniformDatas.size() * sizeof(Vector4));
}

void UniformData::release()
{
    VkDevice device = VulkanContext::Instance().device;

    vkDestroyBuffer(device, _buffer, nullptr);
    vkFreeMemory(device, _bufferMemory, nullptr);
}

void UniformData::addUniform(const std::string &name)
{
    auto it = _uniformsMap.find(name);
    if (it == _uniformsMap.end())
    {
        _uniformsMap.insert(std::make_pair(name, (int)_uniformDatas.size()));
        _uniformDatas.push_back({0.f, 0.f, 0.f, 0.f});
    }
}

void UniformData::setUniform(const std::string &name, const Vector4 &value)
{
    auto it = _uniformsMap.find(name);
    if (it == _uniformsMap.end())
    {
        throw std::runtime_error("failed to set uniform!");
    }
    int i = it->second;
    _uniformDatas[i] = value;
}