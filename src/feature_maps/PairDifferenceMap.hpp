#ifndef PAIR_DIFFERENCE_MAP
#define PAIR_DIFFERENCE_MAP

#include "OneDimStaticMap.hpp"
#include "NetworkUnit.hpp"
#include "NetworkLayer.hpp"

#include <string>
#include <cstddef> // NULL

class PairDifferenceMap: public OneDimStaticMap
{
protected:
    // instead of the two element vector we use these pointers internally, for better performance
    NetworkUnit * _src0;
    NetworkUnit * _src1;

    void _fillSources(const std::vector<size_t> &source_ids); // we extend this to maintain _src1/2
    void _clearSources();

public:
    PairDifferenceMap(NetworkLayer * nl, const size_t &source_id0, const size_t &source_id1)
        : OneDimStaticMap(nl, 2), _src0(NULL), _src1(NULL) {setParameters(source_id0, source_id1);} // full initialization
    explicit PairDifferenceMap(NetworkLayer * nl): PairDifferenceMap(nl, 0, 0) {} // minimal default initialization
    ~PairDifferenceMap(){}

    // string code methods
    std::string getIdCode(){return "PDM";} // return an identification string

    // parameter manipulation
    void setParameters(const size_t &source_id0, const size_t &source_id1); // calls base setParameters with vectorized argument

    // return the feed mean value (mu) and standard deviation (sigma)
    double getFeedMu();
    double getFeedSigma();

    // Computation
    double getFeed();
    double getFirstDerivativeFeed(const int &i1d);
    double getSecondDerivativeFeed(const int &i2d);
    double getVariationalFirstDerivativeFeed(const int &iv1d);
    double getCrossFirstDerivativeFeed(const int &i1d, const int &iv1d);
    double getCrossSecondDerivativeFeed(const int &i2d, const int &iv2d);
};

#endif
