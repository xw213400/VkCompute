#ifndef __VE_COMPUTE_BUFFER_H__
#define __VE_COMPUTE_BUFFER_H__

#include <vulkan/vulkan.h>

enum ComputeBufferMode
{
    Immutable = 1,
    Dynamic,
    SubUpdates
};

class ComputeBuffer
{
public:
    ComputeBuffer(int count, int stride, ComputeBufferMode usage = Immutable);

    void setData(void *array, int count, int srcOffset = 0, int dstOffset = 0);

    void getData(void *array, int count, int srcOffset = 0, int dstOffset = 0);

    void release();

    inline const VkDescriptorBufferInfo* getDescriptor() const
    {
        return &_storageBufferInfo;
    }

private:
    int _stride;
    int _count;
    VkBuffer _buffer;
    VkDeviceMemory _bufferMemory;
    VkDescriptorBufferInfo _storageBufferInfo;
};

#endif