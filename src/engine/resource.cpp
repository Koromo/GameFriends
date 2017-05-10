#include "resource.h"
#include "logging.h"

GF_NAMESPACE_BEGIN

Resource::Resource(const EnginePath& path)
    : path_(path)
    , osPath_(fileSystem.toOSPath(path))
    , ready_(false)
{
}

EnginePath Resource::path() const
{
    return path_;
}

std::string Resource::osPath() const
{
    return osPath_;
}

bool Resource::ready() const
{
    return ready_;
}

void Resource::load()
{
    if (!ready_)
    {
        ready_ = loadImpl();
        if (ready_)
        {
            GF_LOG_DEBUG("Resource {} loaded.", osPath());
        }
    }
}

void Resource::unload()
{
    if (ready_)
    {
        unloadImpl();
        GF_LOG_DEBUG("Resource {} unloaded.", osPath());
        ready_ = false;
    }
}

void ResourceManager::startup()
{
    GF_LOG_INFO("ResourceManager initialized.");
}

void ResourceManager::shutdown()
{
    clear();
    GF_LOG_INFO("ResourceManager shutdown.");
}

void ResourceManager::destroy(const EnginePath& path)
{
    const auto it = resourceMap_.find(path);
    if (it != std::cend(resourceMap_))
    {
        it->second->unload();
        resourceMap_.erase(it);
    }
}

void ResourceManager::clear()
{
    while (!resourceMap_.empty())
    {
        destroy(std::cbegin(resourceMap_)->first);
    }
}

ResourceManager resourceManager;

GF_NAMESPACE_END