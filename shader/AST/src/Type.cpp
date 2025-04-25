#include "Type.hpp"

namespace skr::shader::ast {

Member::Member(const Name& name, const Type& type)
    : _name(name), _type(&type)
{

}

const Name& Member::name() const
{
    return _name;
}

const Size Member::size() const
{
    return _type->size();
}

Type::Type(const Name& name, uint32_t size, uint32_t alignment)
    : _name(name), _size(size), _alignment(alignment)
{

}

/*
Type::Type(const Name& name, std::span<Member> members)
    : _name(name), _members(members.begin(), members.end())
{

}
*/

bool TypeFactory::add_type(const Type& type)
{
    if (types.contains(type.name()))
        return false;
    types[type.name()] = type;
    return true;
}

Type& TypeFactory::get_type(const Name& name)
{
    auto it = types.find(name);
    if (it != types.end())
        return it->second;
    throw std::exception("Type not found");
}

}