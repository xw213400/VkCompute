#ifndef __VE_COMPUTE_SHADER_H__
#define __VE_COMPUTE_SHADER_H__


#include <vulkan/vulkan.h>
#include <string>
#include <vector>
#include <map>


class ComputeBuffer;

class ComputeShader
{
public:
    ComputeShader(const std::string& filename, const std::string& kernel="main");

    void setBuffer(const std::string& name, ComputeBuffer* buffer);

    void setUniform(const std::string& name, float);

    void dispatch(int threadGroupsX, int threadGroupsY, int threadGroupsZ);

    void release();

private:
    int _uniformBindingsCount;
    int _storageBingingsCount;

    VkPipelineLayout _computePipelineLayout;
    VkPipeline _computePipeline;

    VkDescriptorSetLayout _descriptorSetLayout;
    VkDescriptorPool _descriptorPool;
    VkDescriptorSet _descriptorSet;

    std::vector<VkDescriptorSetLayoutBinding> _bindings;
    std::vector<VkWriteDescriptorSet> _descriptorWrites;

    std::map<std::string, int> _bindingsMap;

    VkShaderModule createShaderModule(const std::vector<char> &code);

    void addBinding(const std::string& name, VkDescriptorType descriptorType);

    void createDescriptorSet();
};

#endif
