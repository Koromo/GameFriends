#include "resource.h"

GF_NAMESPACE_BEGIN

Resource::Resource(const FilePath& path)
    : path_(path)
    , ready_(false)
{
}

FilePath Resource::path() const
{
    return path_;
}

bool Resource::ready() const
{
    return ready_;
}

void Resource::load()
{
    if (!ready_)
    {
        /// LOG
        ready_ = loadImpl();
    }
}

void Resource::unload()
{
    if (ready_)
    {
        unloadImpl();
        ready_ = false;
    }
}

void ResourceManager::startup()
{
    /// LOG
}

void ResourceManager::shutdown()
{
    clear();
    /// LOG
}

void ResourceManager::destroy(const FilePath& path)
{
    const auto it = resourceMap_.find(path);
    if (it != std::cend(resourceMap_))
    {
        it->second->unload();
        resourceMap_.erase(it);
    }
}

void ResourceManager::destroy(const std::string& path)
{
    destroy(fileSystem.path(path));
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