#pragma once
#include <span>
#include <vector>
#include <string>
#include <unordered_map>

namespace skr::shader::ast {

using Size = uint32_t;
using String = std::u8string;
using Name = String;
struct Type;
struct Expression;

struct Function
{
public:

private:
    std::vector<const Expression*> expressions; 
};

struct Member
{
public:
    Member(const Name& _name, const Type& type);

    const Name& name() const;
    const Size size() const;

private:
    Name _name = u8"__INVALID_MEMBER__";
    const Type* _type;
};

struct Method
{
public:
    
private:
    std::vector<const Expression*> expressions; 
};

struct Type
{
public:
    Type() = default;
    Type(const Name& name, uint32_t size, uint32_t alignment = 4);
    // Type(const Name& name, std::span<Member> members);

    const Name& name() const { return _name; }
    const Size size() const  { return _size; }

private:
    Name _name = u8"__INVALID_TYPE__";
    Size _size = 0;
    Size _alignment = 0;
    std::unordered_map<Name, Member> _members;
    // std::vector<Member> _members;
};

struct TypeFactory
{
public:
    bool add_type(const Type& type);
    Type& get_type(const Name& name);

private:
    std::unordered_map<Name, Type> types;
};

}