#include "../VkCompute/VulkanContext.h"
#include <random>
#include <iostream>
#include <array>

const uint32_t PARTICLE_COUNT = 8192;

struct Particle
{
    float r;
    float g;
    float b;
    float a;
};

// Initialize particles
void initParticles(std::vector<Particle>& particles)
{
    // Initialize particles
    std::default_random_engine rndEngine((unsigned)time(nullptr));
    std::uniform_real_distribution<float> rndDist(0.0f, 1.0f);

    // Initial particle positions on a circle
    for (auto &particle : particles)
    {
        particle.r = rndDist(rndEngine);
        particle.g = rndDist(rndEngine);
        particle.b = rndDist(rndEngine);
        particle.a = rndDist(rndEngine);
    }
}

int main()
{
    try
    {
        VulkanContext::Instance().initialize();

        VulkanContext::Instance().reset();

        ComputeShader* cs = new ComputeShader("../res/shaders/ComputeShader.csv");

        cs->setUniform("ParameterUBO", 0.5f);

        std::vector<Particle> particles(PARTICLE_COUNT);
        initParticles(particles);
        ComputeBuffer* bufferIn = new ComputeBuffer(PARTICLE_COUNT, sizeof(Particle));
        bufferIn->setData(particles.data(), PARTICLE_COUNT);

        for (int i = 0; i != 5; ++i)
        {
            std::cout << i << "#\t" << particles[i].r << ", " << particles[i].g << ", " << particles[i].b << ", " << particles[i].a << std::endl;
        }

        cs->setBuffer("ParticleSSBOIn", bufferIn);

        ComputeBuffer* bufferOut = new ComputeBuffer(PARTICLE_COUNT, sizeof(Particle));
        cs->setBuffer("ParticleSSBOOut", bufferOut);

        cs->dispatch(PARTICLE_COUNT / 256, 1, 1);

        VulkanContext::Instance().compute();

        bufferOut->getData(particles.data(), PARTICLE_COUNT);

        for (int i = 0; i != 5; ++i)
        {
            std::cout << i << ":\t" << particles[i].r << ", " << particles[i].g << ", " << particles[i].b << ", " << particles[i].a << std::endl;
        }

        bufferIn->release();
        bufferOut->release();
        cs->release();

        VulkanContext::Instance().release();
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}