#include "base/Header.h"
#include "sampleShared/SimpleDOM.h"
#include <stdlib.h>

using std::string;

void
cSimpleDOM::clear()
{
    _name = "";
    _attributes.clear();
    _children.clear();
}

bool
cSimpleDOM::hasAttribute(const std::string& attribute) const
{
    return _attributes.find(attribute) != _attributes.end();
}

std::string
cSimpleDOM::getAttribute(const std::string& attribute) const
{
    std::map<std::string, std::string>::const_iterator i = _attributes.find(attribute);
    if(i == _attributes.end())
    {
        return "";
    }
    return i->second;
}

tSigned32
cSimpleDOM::attributeAsLong(const std::string& attribute) const
{
    string value = getAttribute(attribute);
    assertD(!value.empty());
    char* ptr;
    tSigned32 result = strtol(value.c_str(), &ptr, 10);
    assertD(*ptr == 0);
    return result;
}
tSigned32
cSimpleDOM::attributeAsLongWithDefault(const std::string& attribute, tSigned32 defaultValue) const
{
    if(!hasAttribute(attribute))
    {
        return defaultValue;
    }
    string value = getAttribute(attribute);
    assertD(!value.empty());
    char* ptr;
    tSigned32 result = strtol(value.c_str(), &ptr, 10);
    assertD(*ptr == 0);
    return result;
}
float
cSimpleDOM::attributeAsFloat(const std::string& attribute) const
{
    string value = getAttribute(attribute);
    assertD(!value.empty());
    char* ptr;
    float result = static_cast<float>(strtod(value.c_str(), &ptr));
    assertD(*ptr == 0);
    return result;
}
float
cSimpleDOM::attributeAsFloatWithDefault(const std::string& attribute, float defaultValue) const
{
    if(!hasAttribute(attribute))
    {
        return defaultValue;
    }
    string value = getAttribute(attribute);
    assertD(!value.empty());
    char* ptr;
    float result = static_cast<float>(strtod(value.c_str(), &ptr));
    assertD(*ptr == 0);
    return result;
}
bool
cSimpleDOM::attributeAsBool(const std::string& attribute) const
{
    string value = getAttribute(attribute);
    if(value == "true")
    {
        return true;
    }
    assertD(value == "false");
    return false;
}
bool
cSimpleDOM::attributeAsBoolWithDefault(const std::string& attribute, bool defaultValue) const
{
    if(!hasAttribute(attribute))
    {
        return defaultValue;
    }
    return attributeAsBool(attribute);
}

void
cSimpleDOM::getAllAttributes(std::vector<std::string>& result) const
{
    std::map<std::string, std::string>::const_iterator i;
    for(i = _attributes.begin(); i != _attributes.end(); ++i)
    {
        result.push_back(i->first);        
        result.push_back(i->second);        
    }
}

tSigned32
cSimpleDOM::firstChildWithName(const std::string& name) const
{
    size_t i;
    for(i = 0; i < _children.size(); i++)
    {
        if(_children[i]._name == name)
        {
            return static_cast<tSigned32>(i);
        }
    }
    return -1;
}

cSimpleDOM&
cSimpleDOM::refFirstChildWithName(const std::string& name)
{
    tSigned32 i = firstChildWithName(name);
    assertD(i >= 0);
    return _children[i];
}

const cSimpleDOM&
cSimpleDOM::refFirstChildWithName(const std::string& name) const
{
    tSigned32 i = firstChildWithName(name);
    assertD(i >= 0);
    return _children[i];
}
