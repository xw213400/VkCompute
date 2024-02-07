#include "ComputeBuffer.h"
#include "VulkanContext.h"

ComputeBuffer::ComputeBuffer(int count, int stride, ComputeBufferMode usage) : _count(count), _stride(stride)
{
    VkDevice device = VulkanContext::Instance().device;

    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = count * stride;
    bufferInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(device, &bufferInfo, nullptr, &_buffer) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create buffer!");
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device, _buffer, &memRequirements);

    VkMemoryPropertyFlags properties =
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | // 允许 CPU 写入
        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT; // 保持内存可见一致性，内存映射后立即开始写入

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = VulkanContext::Instance().findMemoryType(memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(device, &allocInfo, nullptr, &_bufferMemory) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate buffer memory!");
    }

    vkBindBufferMemory(device, _buffer, _bufferMemory, 0);

    _storageBufferInfo.buffer = _buffer;
    _storageBufferInfo.offset = 0;
    _storageBufferInfo.range = stride * count;
}

void ComputeBuffer::setData(void* array, int count, int srcOffset, int dstOffset)
{
    VkDevice device = VulkanContext::Instance().device;
    void *data;
    vkMapMemory(device, _bufferMemory, dstOffset * _stride, _count * _stride, 0, &data);
    char* buffer = (char*)array;
    buffer += _stride * srcOffset;
    memcpy(data, buffer, count * _stride);
    vkUnmapMemory(device, _bufferMemory);
}

void ComputeBuffer::getData(void *array, int count, int srcOffset, int dstOffset)
{
    VkDevice device = VulkanContext::Instance().device;
    void *data;
    vkMapMemory(device, _bufferMemory, srcOffset * _stride, _count * _stride, 0, &data);
    char* buffer = (char*)array;
    buffer += _stride * dstOffset;
    memcpy(buffer, data, count * _stride);
    vkUnmapMemory(device, _bufferMemory);
}

void ComputeBuffer::release()
{
    VkDevice device = VulkanContext::Instance().device;
    vkDestroyBuffer(device, _buffer, nullptr);
    vkFreeMemory(device, _bufferMemory, nullptr);
}