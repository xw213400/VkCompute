#ifndef __VE_UNIFORM_DATA_H__
#define __VE_UNIFORM_DATA_H__

#include <vulkan/vulkan.h>
#include "Singleton.h"
#include <vector>
#include <map>
#include <string>


struct Vector4
{
    float x;
    float y;
    float z;
    float w;
};


class UniformData : public Singleton<UniformData>
{
public:
    void initialize();

    void release();

    void updateMemory();

    void addUniform(const std::string &name);

    void setUniform(const std::string &name, const Vector4 &value);

    inline VkDescriptorBufferInfo* getDescriptorBufferInfo()
    {
        return &_descriptorBufferInfo;
    }

private:
    void* _uniformBufferMapped;
    std::vector<Vector4> _uniformDatas;
    std::map<std::string, int> _uniformsMap;
    VkBuffer _buffer;
    VkDeviceMemory _bufferMemory;
    VkDescriptorBufferInfo _descriptorBufferInfo;
};

#endif