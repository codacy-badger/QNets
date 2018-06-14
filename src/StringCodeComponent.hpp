#ifndef STRING_CODE_COMPONENT
#define STRING_CODE_COMPONENT

#include "StringCodeUtilities.hpp" // for functions on stringCodes, look there for documentation about stringCodes

#include <string>

// Base class for all network components which can be configured by string codes
class StringCodeComponent
{
protected:

public:

    // If possible, every StringCodeComponent with params and/or members should implement a constructor which uses a string code as one of the arguments

    virtual ~StringCodeComponent(){};

    // virtual string code getters, to be extended by child
    // documentation on the code syntax is in StringCodeUtilities.hpp
    virtual std::string getIdCode() = 0; // return unique (at least within class) identifier for component type, should be upper case letters (same for classIdCode)
    virtual std::string getClassIdCode() = 0; // Usually should be set only by the direct child of StringCodeComponent. May serve to distinguish identifiers if necessary.
    virtual std::string getParams(){return "";} // return parameter code with identifiers in lower case letters
    virtual std::string getMemberTreeCode(){return "";} // return treeCodes of added StringCodeComponent members

    // string code getter composers
    std::string getFullCode(){return composeFullCode(this->getIdCode(), this->getParams());} //return id + params
    std::string getTreeCode(){return composeTreeCode(this->getFullCode(), this->getMemberTreeCode());} // return full tree of id+params

    // set by string code
    virtual void setParams(const std::string &params){} // set params of this by params string
    virtual void setMemberParams(const std::string &memberTreeCode){} // recursively set params of all members by memberTreeCode string

    void setTreeParams(const std::string treeCode){this->setParams(readParams(treeCode)); this->setMemberParams(readMemberTreeCode(treeCode));} // set the params of the full tree
};



#endif
