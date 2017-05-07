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
        loadImpl();
        ready_ = true;
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

void ResourceTable::destroy(const FilePath& path)
{
    const auto it = resourceMap_.find(path);
    if (it != std::cend(resourceMap_))
    {
        it->second->unload();
        resourceMap_.erase(it);
    }
}

void ResourceTable::destroy(const std::string& path)
{
    destroy(fileSystem.path(path));
}

void ResourceTable::clear()
{
    while (!resourceMap_.empty())
    {
        destroy(std::cbegin(resourceMap_)->first);
    }
}

ResourceTable resourceTable;

GF_NAMESPACE_END