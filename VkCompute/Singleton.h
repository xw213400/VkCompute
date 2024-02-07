#ifndef __VE_SINGLETON_H__
#define __VE_SINGLETON_H__


template <typename T>
class Singleton
{
    public:
        static T& Instance()
        {
            static T instance;
            return instance;
        }

    protected:
        Singleton() {}
        ~Singleton() {}
        
    public:
        Singleton(Singleton const &) = delete;
        Singleton& operator=(Singleton const &) = delete;
};

#endif