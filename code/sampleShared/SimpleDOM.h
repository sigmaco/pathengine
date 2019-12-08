
#include <string>
#include <map>
#include <vector>

class cSimpleDOM
{
public:

    std::string _name;
    std::map<std::string, std::string> _attributes;
    std::vector<cSimpleDOM> _children;

    void clear();
    bool hasAttribute(const std::string& attribute) const;
    std::string getAttribute(const std::string& attribute) const;

    tSigned32 attributeAsLong(const std::string& attribute) const;
    tSigned32 attributeAsLongWithDefault(const std::string& attribute, tSigned32 defaultValue) const;
    float attributeAsFloat(const std::string& attribute) const;
    float attributeAsFloatWithDefault(const std::string& attribute, float defaultValue) const;
    bool attributeAsBool(const std::string& attribute) const;
    bool attributeAsBoolWithDefault(const std::string& attribute, bool defaultValue) const;

    void getAllAttributes(std::vector<std::string>& result) const;

    tSigned32 firstChildWithName(const std::string& name) const;
    cSimpleDOM& refFirstChildWithName(const std::string& name);
    const cSimpleDOM& refFirstChildWithName(const std::string& name) const;
};
