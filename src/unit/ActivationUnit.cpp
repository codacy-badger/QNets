#include "qnets/unit/ActivationUnit.hpp"

// --- String Code

void ActivationUnit::setMemberParams(const std::string &memberTreeCode)
{
    using namespace std;
    std::string actfCode = readTreeCode(memberTreeCode, countNMembers(memberTreeCode) > 1 ? 1 : 0); // atm using index
    this->setActivationFunction(std_actf::provideActivationFunction(readIdCode(actfCode)));
    _actf->setTreeParams(actfCode);
}

// --- Computation

void ActivationUnit::computeOutput()
{
    const bool flag_d1 = (_v1d != nullptr) || (_v2d != nullptr) || (_v1vd != nullptr) || (_v1d1vd != nullptr);
    const bool flag_d2 = (_v2d != nullptr) || (_v1vd != nullptr);
    const bool flag_d3 = _v2d1vd != nullptr;
    _actf->fad(_pv, _v, _a1d, _a2d, _a3d, flag_d1, flag_d2, flag_d3);
}
