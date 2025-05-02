#pragma once
#include <cassert>
#include <string>

namespace skr::SSL
{
struct SourceBuilder
{
public:
    SourceBuilder() = default;
    SourceBuilder(std::wstring_view indent_str)
        : _indent_str(indent_str) 
    {

    }

    SourceBuilder& append(const std::wstring& content)
    {
        if (is_line_start)
        {
            for (size_t i = 0; i < _indent; i++)
                _source += _indent_str;
            is_line_start = false;
        }
        _source += content;
        return *this;
    }

    SourceBuilder& endline()
    {
        _source += L"\n";
        is_line_start = true;
        return *this;
    }

    SourceBuilder& endline(wchar_t sep)
    {
        _source += sep;
        _source += L"\n";
        is_line_start = true;
        return *this;
    }

    SourceBuilder& endline(std::wstring_view sep)
    {
        _source += sep;
        _source += L"\n";
        is_line_start = true;
        return *this;
    }

    template <typename F>
    SourceBuilder& indent(F&& f)
    {
        _indent += 1;
        f();
        _indent -= 1;
        return *this;
    }

    const std::wstring& content() 
    { 
        _r = _source; 
        _source = L""; 
        return _r; 
    }

private:
    std::wstring _indent_str = L"  ";
    std::wstring _source = L"";
    std::wstring _r = L"";
    size_t _indent = 0;
    bool is_line_start = true;
};
  
} // namespace skr::SSL