#include "FeedForwardNeuralNetwork.hpp"

#include "NNUnit.hpp"
#include "ActivationFunctionManager.hpp"

#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <random>
#include <limits>
#include <algorithm>

#ifdef OPENMP
#include <thread> //to detect the number of hardware threads on the system
#endif

// --- Variational Parameters

int FeedForwardNeuralNetwork::getNBeta()
{
    using namespace std;
    int nbeta=0;
    for (vector<FedNetworkLayer *>::size_type i=0; i<_L_fed.size(); ++i)
        {
            for (int j=0; j<_L_fed[i]->getNFedUnits(); ++j)
                {
                    if (_L_fed[i]->getFedUnit(j)->getFeeder())
                        {
                            nbeta += _L_fed[i]->getFedUnit(j)->getFeeder()->getNBeta();
                        }
                }
        }
    return nbeta;
}


double FeedForwardNeuralNetwork::getBeta(const int &ib)
{
    using namespace std;
    if ( ib<0 || ib >= getNBeta() )
        {
            cout << endl << "ERROR FeedForwardNeuralNetwork::getBeta : index out of boundaries" << endl;
            cout << ib << " against the maximum allowed " << this->getNBeta() << endl << endl;
        }
    else
        {
            int idx=0;
            for (vector<FedNetworkLayer *>::size_type i=0; i<_L_fed.size(); ++i)
                {
                    for (int j=0; j<_L_fed[i]->getNFedUnits(); ++j)
                        {
                            if (_L_fed[i]->getFedUnit(j)->getFeeder())
                                {
                                    for (int k=0; k<_L_fed[i]->getFedUnit(j)->getFeeder()->getNBeta(); ++k)
                                        {
                                            if (idx==ib) return _L_fed[i]->getFedUnit(j)->getFeeder()->getBeta(k);
                                            idx++;
                                        }
                                }
                        }
                }
        }
    cout << endl << "ERROR FeedForwardNeuralNetwork::getBeta : index not found" << endl;
    cout << ib << " against the maximum allowed " << this->getNBeta() << endl << endl;
    return -666.;
}


void FeedForwardNeuralNetwork::getBeta(double * beta){
    using namespace std;
    int idx=0;
    for (vector<FedNetworkLayer *>::size_type i=0; i<_L_fed.size(); ++i)
        {
            for (int j=0; j<_L_fed[i]->getNFedUnits(); ++j)
                {
                    if (_L_fed[i]->getFedUnit(j)->getFeeder())
                        {
                            for (int k=0; k<_L_fed[i]->getFedUnit(j)->getFeeder()->getNBeta(); ++k)
                                {
                                    beta[idx] = _L_fed[i]->getFedUnit(j)->getFeeder()->getBeta(k);
                                    idx++;
                                }
                        }
                }
        }
}


void FeedForwardNeuralNetwork::setBeta(const int &ib, const double &beta)
{
    using namespace std;
    if ( ib >= this->getNBeta() )
        {
            cout << endl << "ERROR FeedForwardNeuralNetwork::getBeta : index out of boundaries" << endl;
            cout << ib << " against the maximum allowed " << this->getNBeta() << endl << endl;
        }
    else
        {
            int idx=0;
            for (vector<FedNetworkLayer *>::size_type i=0; i<_L_fed.size(); ++i)
                {
                    for (int j=0; j<_L_fed[i]->getNFedUnits(); ++j)
                        {
                            if (_L_fed[i]->getFedUnit(j)->getFeeder())
                                {
                                    for (int k=0; k<_L_fed[i]->getFedUnit(j)->getFeeder()->getNBeta(); ++k)
                                        {
                                            if (idx==ib) _L_fed[i]->getFedUnit(j)->getFeeder()->setBeta(k, beta);
                                            idx++;
                                        }
                                }
                        }
                }
        }

}


void FeedForwardNeuralNetwork::setBeta(const double * beta)
{
    using namespace std;
    int idx=0;
    for (vector<FedNetworkLayer *>::size_type i=0; i<_L_fed.size(); ++i)
        {
            for (int j=0; j<_L_fed[i]->getNFedUnits(); ++j)
                {
                    if (_L_fed[i]->getFedUnit(j)->getFeeder())
                        {
                            for (int k=0; k<_L_fed[i]->getFedUnit(j)->getFeeder()->getNBeta(); ++k)
                                {
                                    _L_fed[i]->getFedUnit(j)->getFeeder()->setBeta(k, beta[idx]);
                                    idx++;
                                }
                        }
                }
        }

}


void FeedForwardNeuralNetwork::randomizeBetas()
{
    using namespace std;

    random_device rdev;
    mt19937_64 rgen = std::mt19937_64(rdev());
    uniform_real_distribution<double> rd;

    int nsource;
    double bah;

    for (vector<FedNetworkLayer *>::size_type i=0; i<_L_fed.size(); ++i)
        {
            for (int j=0; j<_L_fed[i]->getNFedUnits(); ++j)
                {
                    if (_L_fed[i]->getFedUnit(j)->getFeeder())
                        {
                            nsource = _L_fed[i]->getFedUnit(j)->getFeeder()->getNBeta();
                            // target sigma to keep sum of weighted inputs in range [-4,4], assuming uniform distribution
                            // sigma = 8/sqrt(12) = (b-a)/sqrt(12) * m^(1/2)
                            bah = 4 * pow(nsource, -0.5); // (b-a)/2
                            rd = uniform_real_distribution<double>(-bah,bah);

                            for (int k=0; k<nsource; ++k)
                                {
                                    _L_fed[i]->getFedUnit(j)->getFeeder()->setBeta(k, rd(rgen));
                                }
                        }
                }
        }
}



// --- Computation


double FeedForwardNeuralNetwork::getCrossSecondDerivative(const int &i, const int &i1d, const int &iv1d){
    return _L_out->getUnit(i+1)->getCrossSecondDerivativeValue(i1d, iv1d);
}


void FeedForwardNeuralNetwork::getCrossSecondDerivative(double *** d1vd1){
    for (int i=0; i<getNOutput(); ++i){
        getCrossSecondDerivative(i, d1vd1[i]);
    }
}


void FeedForwardNeuralNetwork::getCrossSecondDerivative(const int &i, double ** d1vd1){
    for (int i1d=0; i1d<getNInput(); ++i1d){
        for (int iv1d=0; iv1d<getNBeta(); ++iv1d){
            d1vd1[i1d][iv1d] = getCrossSecondDerivative(i, i1d, iv1d);
        }
    }
}


double FeedForwardNeuralNetwork::getCrossFirstDerivative(const int &i, const int &i1d, const int &iv1d){
    return _L_out->getUnit(i+1)->getCrossFirstDerivativeValue(i1d, iv1d);
}


void FeedForwardNeuralNetwork::getCrossFirstDerivative(double *** d1vd1){
    for (int i=0; i<getNOutput(); ++i){
        getCrossFirstDerivative(i, d1vd1[i]);
    }
}


void FeedForwardNeuralNetwork::getCrossFirstDerivative(const int &i, double ** d1vd1){
    for (int i1d=0; i1d<getNInput(); ++i1d){
        for (int iv1d=0; iv1d<getNBeta(); ++iv1d){
            d1vd1[i1d][iv1d] = getCrossFirstDerivative(i, i1d, iv1d);
        }
    }
}



double FeedForwardNeuralNetwork::getVariationalFirstDerivative(const int &i, const int &iv1d)
{
    return _L_out->getUnit(i+1)->getVariationalFirstDerivativeValue(iv1d);
}


void FeedForwardNeuralNetwork::getVariationalFirstDerivative(const int &i, double * vd1)
{
    for (int iv1d=0; iv1d<getNInput(); ++iv1d){
        vd1[iv1d] = getVariationalFirstDerivative(i, iv1d);
    }
}


void FeedForwardNeuralNetwork::getVariationalFirstDerivative(double ** vd1)
{
    for (int i=0; i<getNOutput(); ++i){
        for (int iv1d=0; iv1d<getNBeta(); ++iv1d){
            vd1[i][iv1d] = getVariationalFirstDerivative(i, iv1d);
        }
    }
}


double FeedForwardNeuralNetwork::getSecondDerivative(const int &i, const int &i2d)
{
    return ( _L_out->getUnit(i+1)->getSecondDerivativeValue(i2d) );
}


void FeedForwardNeuralNetwork::getSecondDerivative(const int &i, double * d2)
{
    for (int i2d=0; i2d<getNInput(); ++i2d){
        d2[i2d] = getSecondDerivative(i, i2d);
    }
}


void FeedForwardNeuralNetwork::getSecondDerivative(double ** d2)
{
    for (int i=0; i<getNOutput(); ++i){
        for (int i2d=0; i2d<getNInput(); ++i2d){
            d2[i][i2d] = getSecondDerivative(i, i2d);
        }
    }
}


double FeedForwardNeuralNetwork::getFirstDerivative(const int &i, const int &i1d)
{
    return ( _L_out->getUnit(i+1)->getFirstDerivativeValue(i1d) );
}


void FeedForwardNeuralNetwork::getFirstDerivative(const int &i, double * d1)
{
    for (int i1d=0; i1d<getNInput(); ++i1d){
        d1[i1d] = getFirstDerivative(i, i1d);
    }
}


void FeedForwardNeuralNetwork::getFirstDerivative(double ** d1)
{
    for (int i=0; i<getNOutput(); ++i){
        for (int i1d=0; i1d<getNInput(); ++i1d){
            d1[i][i1d] = getFirstDerivative(i, i1d);
        }
    }
}


double FeedForwardNeuralNetwork::getOutput(const int &i)
{
    return _L_out->getUnit(i+1)->getValue();
}


void FeedForwardNeuralNetwork::getOutput(double * out)
{
    for (int i=1; i<_L_out->getNUnits(); ++i){
        out[i-1] = _L_out->getUnit(i)->getValue();
    }
}


void FeedForwardNeuralNetwork::evaluate(const double * in, double * out, double ** d1, double ** d2, double ** vd1){
    setInput(in);
    FFPropagate();
    if (out!=NULL) {
        getOutput(out);
    }
    if (hasFirstDerivativeSubstrate() && d1!=NULL){
        getFirstDerivative(d1);
    }
    if (hasSecondDerivativeSubstrate() && d2!=NULL){
        getSecondDerivative(d2);
    }
    if (hasVariationalFirstDerivativeSubstrate() && vd1!=NULL){
        getVariationalFirstDerivative(vd1);
    }
}

#ifdef OPENMP
bool compare_NUnits(NetworkLayer * A, NetworkLayer * B) { return A->getNUnits()<B->getNUnits(); }
#endif

void FeedForwardNeuralNetwork::FFPropagate()
{

    _L_in->computeValues(); // OpenMP not worth for input layer

#ifdef OPENMP
// compile with -DOPENMP -fopenmp flags to use parallelization here

    int nthreads = std::min( (int)std::thread::hardware_concurrency(), (*std::max_element(_L.begin()+1, _L.end(), compare_NUnits))->getNUnits() - 1 );
    if (nthreads>1) {
#pragma omp parallel num_threads(nthreads)
        for (std::vector<NetworkLayer *>::size_type i=1; i<_L.size(); ++i)
            {
                _L[i]->computeValues(); // actual omp for inside computeValues
#pragma omp barrier // just to be sure
            }
    }
    else {

#endif

    for (std::vector<NetworkLayer *>::size_type i=1; i<_L.size(); ++i)
        {
            _L[i]->computeValues();
        }

#ifdef OPENMP
    }
#endif

}


void FeedForwardNeuralNetwork::setInput(const double *in)
{
    // set the protovalues of the first layer units
    for (int i=0; i<_L_in->getNInputUnits(); ++i)
        {
            _L_in->getInputUnit(i)->setProtoValue(in[i]);
        }
}


void FeedForwardNeuralNetwork::setInput(const int &i, const double &in)
{
    // set the protovalues of the first layer units
    _L_in->getInputUnit(i)->setProtoValue(in);
}



// --- Substrates


void FeedForwardNeuralNetwork::addLastHiddenLayerCrossSecondDerivativeSubstrate()
{
    using namespace std;

    // cross second derivatives require first, second, and variational first derivatives
    if (!_flag_1d || !_flag_v1d || !_flag_2d){
        throw std::runtime_error( "CrossSecondDerivative requires FirstDerivative, VariationalFirstDerivative, and SecondDerivative" );
    }

    // set the substrate in the units
    for (std::vector<NNLayer *>::size_type i=_L_nn.size()-2; i<_L_nn.size(); ++i)
        {
            _L_nn[i]->addCrossSecondDerivativeSubstrate(getNInput(), _nvp);
        }

    _flag_c2d = true;
}


void FeedForwardNeuralNetwork::addCrossSecondDerivativeSubstrate()
{
    using namespace std;

    // cross second derivatives require first, second, and variational first derivatives
    if (!_flag_1d || !_flag_v1d || !_flag_2d){
        throw std::runtime_error( "CrossSecondDerivative requires FirstDerivative, VariationalFirstDerivative, and SecondDerivative" );
    }

    // set the substrate in the units
    for (std::vector<NetworkLayer *>::size_type i=0; i<_L.size(); ++i){
            _L[i]->addCrossSecondDerivativeSubstrate(getNInput(), _nvp);
        }

    _flag_c2d = true;
}



void FeedForwardNeuralNetwork::addLastHiddenLayerCrossFirstDerivativeSubstrate()
{
    using namespace std;

    // cross first derivatives require first and variational first derivatives
    if (!_flag_1d || !_flag_v1d){
        throw std::runtime_error( "CrossFirstDerivative requires FirstDerivative and VariationalFirstDerivative" );
    }

    // set the substrate in the units
    for (std::vector<NNLayer *>::size_type i=_L_nn.size()-2; i<_L_nn.size(); ++i){
            _L_nn[i]->addCrossFirstDerivativeSubstrate(getNInput(), _nvp);
        }

    _flag_c1d = true;
}


void FeedForwardNeuralNetwork::addCrossFirstDerivativeSubstrate()
{
    using namespace std;

    // cross first derivatives require first and variational first derivatives
    if (!_flag_1d || !_flag_v1d){
        throw std::runtime_error( "CrossFirstDerivative requires FirstDerivative and VariationalFirstDerivative" );
    }

    // set the substrate in the units
    for (std::vector<NetworkLayer *>::size_type i=0; i<_L.size(); ++i)
        {
            _L[i]->addCrossFirstDerivativeSubstrate(getNInput(), _nvp);
        }

    _flag_c1d = true;
}


void FeedForwardNeuralNetwork::addLastHiddenLayerVariationalFirstDerivativeSubstrate()
{
    // count the total number of variational parameters
    _nvp=0;
    for (std::vector<NNLayer *>::size_type i=_L_nn.size()-2; i<_L_nn.size(); ++i)
        {
            _nvp += _L_nn[i]->getNVariationalParameters();
        }
    // set the substrate in the units
    for (std::vector<NetworkLayer *>::size_type i=0; i<_L.size(); ++i)
        {
            _L[i]->addVariationalFirstDerivativeSubstrate(_nvp);
        }
    // set the id of the variational parameters for all the feeders
    int id_vp=0;
    for (std::vector<NNLayer *>::size_type i=_L_nn.size()-2; i<_L_nn.size(); ++i)
        {
            id_vp = _L_nn[i]->setVariationalParametersID(id_vp);
        }

    _flag_v1d = true;
}


void FeedForwardNeuralNetwork::addVariationalFirstDerivativeSubstrate()
{
    // count the total number of variational parameters
    _nvp=0;
    for (std::vector<FedNetworkLayer *>::size_type i=0; i<_L_fed.size(); ++i)
        {
            _nvp += _L_fed[i]->getNVariationalParameters();
        }
    // set the substrate in the units
    for (std::vector<NetworkLayer *>::size_type i=0; i<_L.size(); ++i)
        {
            _L[i]->addVariationalFirstDerivativeSubstrate(_nvp);
        }
    // set the id of the variational parameters for all the feeders
    int id_vp=0;
    for (std::vector<FedNetworkLayer *>::size_type i=0; i<_L_fed.size(); ++i)
        {
            id_vp = _L_fed[i]->setVariationalParametersID(id_vp);
        }

    _flag_v1d = true;
}


void FeedForwardNeuralNetwork::addSecondDerivativeSubstrate()
{
    // add the second derivative substrate to all the layers
    for (std::vector<NetworkLayer *>::size_type i=0; i<_L.size(); ++i)
        {
            _L[i]->addSecondDerivativeSubstrate(getNInput());
        }

    _flag_2d = true;
}


void FeedForwardNeuralNetwork::addFirstDerivativeSubstrate()
{
    // add the first derivative substrate to all the layers
    for (std::vector<NetworkLayer *>::size_type i=0; i<_L.size(); ++i)
        {
            _L[i]->addFirstDerivativeSubstrate(getNInput());
        }

    _flag_1d = true;
}


// --- Connect the neural network

void FeedForwardNeuralNetwork::connectFFNN()
{
    if(_flag_connected) this->disconnectFFNN();

    _L_fed[0]->connectOnTopOfLayer(_L_in); // connect the first fed layer to the input layer
    for (std::vector<FedNetworkLayer *>::size_type i=1; i<_L_fed.size(); ++i) // connect the rest
        {
            _L_fed[i]->connectOnTopOfLayer(_L_fed[i-1]);
        }
    _flag_connected = true;
}


void FeedForwardNeuralNetwork::connectAndAddSubstrates(bool flag_d1, bool flag_d2, bool flag_vd1, bool flag_c1d, bool flag_c2d){
    connectFFNN();
    if (flag_d1) addFirstDerivativeSubstrate();
    if (flag_d2) addSecondDerivativeSubstrate();
    if (flag_vd1) addVariationalFirstDerivativeSubstrate();
    if (flag_c1d) addCrossFirstDerivativeSubstrate();
    if (flag_c2d) addCrossSecondDerivativeSubstrate();
}


void FeedForwardNeuralNetwork::disconnectFFNN()
{
    if ( !_flag_connected )
    {
        using namespace std;
        cout << "ERROR: FeedForwardNeuralNetwork::disconnectFFNN() : trying to disconnect an already disconnected FFNN" << endl << endl;
    }

    for (std::vector<FedNetworkLayer *>::size_type i=0; i<_L_fed.size(); ++i)
    {
        _L_fed[i]->disconnect();
    }
    _flag_connected = false;
}


// --- Modify NN structure

void FeedForwardNeuralNetwork::setGlobalActivationFunctions(ActivationFunctionInterface * actf)
{
    for (std::vector<NNLayer *>::size_type i=0; i<_L_nn.size(); ++i)
        {
            _L_nn[i]->setActivationFunction(actf);
        }
}


void FeedForwardNeuralNetwork::setLayerSize(const int &li, const int &size)
{
    _L[li]->setSize(size);
}


void FeedForwardNeuralNetwork::setNNLayerActivationFunction(const int &li, ActivationFunctionInterface * actf)
{
    _L_nn[li]->setActivationFunction(actf);
}


void FeedForwardNeuralNetwork::pushHiddenLayer(const int &size)
{
    NNLayer * newhidlay = new NNLayer(size, std_actf::provideActivationFunction());

    std::vector<NetworkLayer *>::iterator it = _L.end()-1;
    std::vector<FedNetworkLayer *>::iterator it_fed = _L_fed.end()-1;
    std::vector<NNLayer *>::iterator it_nn = _L_nn.end()-1;

    if (_flag_connected)
        {
            using namespace std;
            // count the number of beta before the last (output) layer
            int nbeta = 0;
            for (vector<FedNetworkLayer *>::size_type i=0; i<_L_fed.size()-1; ++i)
                {
                    for (int j=0; j<_L_fed[i]->getNFedUnits(); ++j)
                        {
                            if (_L_fed[i]->getFedUnit(j)->getFeeder())
                                {
                                    nbeta += _L_fed[i]->getFedUnit(j)->getFeeder()->getNBeta();
                                }
                        }
                }
            int total_nbeta = this->getNBeta();
            // store the beta for the output
            double * old_beta = new double[total_nbeta-nbeta];
            for (int i=nbeta; i<total_nbeta; ++i)
                {
                    old_beta[i-nbeta] = getBeta(i);
                }

            // disconnect last layer
            _L_out->disconnect();  // disconnect the last (output) layer
            // insert new layer
            _L.insert(it, newhidlay);
            _L_fed.insert(it_fed, newhidlay);
            _L_nn.insert(it_nn, newhidlay);
            // reconnect the layers
            _L_nn[_L_nn.size()-2]->connectOnTopOfLayer(_L[_L.size()-3]);
            _L_nn[_L_nn.size()-1]->connectOnTopOfLayer(_L[_L.size()-2]);

            // restore the old beta
            for (int i=nbeta; i<total_nbeta; ++i)
                {
                    this->setBeta(i,old_beta[i-nbeta]);
                }
            // set all the other beta to zero
            for (int i=total_nbeta; i<this->getNBeta(); ++i)
                {
                    this->setBeta(i,0.);
                }
            for (int i=0; i<_L_fed[_L_fed.size()-1]->getNFedUnits(); ++i)
                {
                    _L_fed[_L_fed.size()-1]->getFedUnit(i)->getFeeder()->setBeta(i,1.);
                }
            // free memory
            delete[] old_beta;
        }
    else
        {
            _L.insert(it, newhidlay);
            _L_fed.insert(it_fed, newhidlay);
            _L_nn.insert(it_nn, newhidlay);
        }
}


void FeedForwardNeuralNetwork::popHiddenLayer()
{
    delete _L[_L.size()-2];

    std::vector<NetworkLayer *>::iterator it = _L.end()-2;
    std::vector<FedNetworkLayer *>::iterator it_fed = _L_fed.end()-2;
    std::vector<NNLayer *>::iterator it_nn = _L_nn.end()-2;

    _L.erase(it);
    _L_fed.erase(it_fed);
    _L_nn.erase(it_nn);
}


// --- Store FFNN on a file

void FeedForwardNeuralNetwork::storeOnFile(const char * filename)
{
    using namespace std;

    // open file
    ofstream file;
    file.open(filename);
    // store the number of layers
    file << getNLayers() << endl;
    // store the tree code of each layer
    for (int i=0; i<getNLayers(); ++i) {
        file << getLayer(i)->getTreeCode() << endl;
    }
    // store connected flag
    file << _flag_connected << endl;
    // store the flags for the substrates
    file << _flag_1d << " " << _flag_2d << " " << _flag_v1d << " " << _flag_c1d << " " << _flag_c2d << endl;
    file.close();
}


// --- Constructor

/* is this still useful?
FeedForwardNeuralNetwork::FeedForwardNeuralNetwork(std::vector<std::vector<std::string>> &actf){
    using namespace std;

    // check input
    if (actf.size() < 3)
        throw std::invalid_argument( "There must be at least 3 layers" );
    for (vector<string> layer_actf : actf){
        if (layer_actf.size() < 2)
            throw std::invalid_argument( "Each layer must contain at least 2 units (one is for the offset)" );
    }

    // declare the NN with the right geometry
    this->construct(actf[0].size(), actf[1].size(), actf.back().size());
    for (unsigned int l=2; l<actf.size()-1; ++l){
        this->pushHiddenLayer(actf[l].size());
    }

    // set the activation functions
    ActivationFunctionInterface * af;
    for (unsigned int l=0; l<actf.size(); ++l){
        for (unsigned int u=1; u<actf[l].size(); ++u){
            af = std_actf::provideActivationFunction(actf[l][u]);

            if (af){
                if (l != 0) _L_nn[l-1]->getNNUnit(u-1)->setActivationFunction(af);
            } else{
                cout << "ERROR FeedForwardNeuralNetwork(const int &nlayers, const int * layersize, const char ** actf) : given activation function " << actf[l][u] << " not known" << endl;
                throw std::invalid_argument( "invalid activation function id code" );
            }
        }
    }

}
*/

FeedForwardNeuralNetwork::FeedForwardNeuralNetwork(const char *filename)
{
    // open file
    using namespace std;

    string line, id, size;
    vector<string> layerMemberCodes;
    ifstream file;
    file.open(filename);

    // read the number of layers
    int nlayers;
    file >> nlayers;

    NNLayer * nnl;
    int nunits = 0;
    int il = 0;
    while (std::getline(file, line)) {
        if (line == "") continue; // idk why I get an empty line here in the first iteration
        id = readIdCode(line);
        if (setParamValue(readParams(line), "nunits", nunits)) {
            if (id == "INL") {
                _L_in = new InputLayer(nunits);
                _L.push_back(_L_in);
            }
            else if (id == "NNL") {
                nnl = new NNLayer(nunits);
                _L.push_back(nnl);
                _L_fed.push_back(nnl);
                _L_nn.push_back(nnl);
            }
            else if (id == "OUTL") {
                _L_out = new OutputNNLayer(nunits);
                _L.push_back(_L_out);
                _L_fed.push_back(_L_out);
                _L_nn.push_back(_L_out);
            }
            else {
                throw std::invalid_argument("Read unknown layer identifier from file!");
            }
            layerMemberCodes.push_back(readMemberTreeCode(line));
        }
        ++il;
        if (il==nlayers) break;
    }
    if (il!=nlayers) throw std::invalid_argument("Stored FFNN file declares to have more layers than it has layer codes.");

    // connect the NN, if connected is found true
    _nvp = 0;
    bool connected;
    file >> connected;
    _flag_connected = false;
    if (connected) connectFFNN();

    // set betas and all other params/actf
    for (int i=0; i<getNLayers(); ++i) getLayer(i)->setMemberParams(layerMemberCodes[i]);

    // read and set the substrates
    _flag_1d = 0; _flag_2d = 0; _flag_v1d = 0; _flag_c1d = 0; _flag_c2d = 0;
    bool flag_1d = 0, flag_2d = 0, flag_v1d = 0, flag_c1d = 0, flag_c2d = 0;
    file >> flag_1d;
    if (flag_1d) addFirstDerivativeSubstrate();
    file >> flag_2d;
    if (flag_2d) addSecondDerivativeSubstrate();
    file >> flag_v1d;
    if (flag_v1d) addVariationalFirstDerivativeSubstrate();
    file >> flag_c1d;
    if (flag_c1d) addCrossFirstDerivativeSubstrate();
    file >> flag_c2d;
    if (flag_c2d) addCrossSecondDerivativeSubstrate();

    file.close();
}


FeedForwardNeuralNetwork::FeedForwardNeuralNetwork(FeedForwardNeuralNetwork * ffnn){
    // read size and create input layer
    _L_in = new InputLayer(ffnn->getInputLayer()->getNUnits());
    _L.push_back(_L_in);

    // read size and create hidden layers
    NNLayer * nnl;
    for (int i=0; i<ffnn->getNNeuralLayers()-1; ++i){ // exclude output layer
        nnl = new NNLayer(ffnn->getNNLayer(i)->getNUnits());   // first create layers with default actf
        _L.push_back(nnl);
        _L_fed.push_back(nnl);
        _L_nn.push_back(nnl);
    }

    // read size and create output layer
    _L_out = new OutputNNLayer(ffnn->getOutputLayer()->getNUnits());
    _L.push_back(_L_out);
    _L_fed.push_back(_L_out);
    _L_nn.push_back(_L_out);

     // read and set the substrates
    _nvp = 0;
    _flag_connected = false;
    if (ffnn->isConnected()) connectFFNN();

    _flag_1d = 0; _flag_2d = 0; _flag_v1d = 0; _flag_c1d = 0; _flag_c2d = 0;
    if (ffnn->hasFirstDerivativeSubstrate()) addFirstDerivativeSubstrate();
    if (ffnn->hasSecondDerivativeSubstrate()) addSecondDerivativeSubstrate();
    if (ffnn->hasVariationalFirstDerivativeSubstrate()) addVariationalFirstDerivativeSubstrate();
    if (ffnn->hasCrossFirstDerivativeSubstrate()) addCrossFirstDerivativeSubstrate();
    if (ffnn->hasCrossSecondDerivativeSubstrate()) addCrossSecondDerivativeSubstrate();

    // now copy the parameter tree (incl. betas) for all layers
    for (int i=0; i<ffnn->getNLayers(); ++i) {
        _L[i]->setMemberParams(ffnn->getLayer(i)->getMemberTreeCode());
    }
}


FeedForwardNeuralNetwork::FeedForwardNeuralNetwork(const int &insize, const int &hidlaysize, const int &outsize)
{
    construct(insize, hidlaysize, outsize);
}


void FeedForwardNeuralNetwork::construct(const int &insize, const int &hidlaysize, const int &outsize){
    _L_in = new InputLayer(insize);
    NNLayer * hidlay = new NNLayer(hidlaysize, std_actf::provideActivationFunction());
    _L_out = new OutputNNLayer(outsize, std_actf::provideActivationFunction());

    _L.push_back(_L_in);
    _L.push_back(hidlay);
    _L_fed.push_back(hidlay);
    _L_nn.push_back(hidlay);
    _L.push_back(_L_out);
    _L_fed.push_back(_L_out);
    _L_nn.push_back(_L_out);

    _flag_connected = false;
    _flag_1d = false;
    _flag_2d = false;
    _flag_v1d = false;
    _flag_c1d = false;
    _flag_c2d = false;

    _nvp=0;
}


// --- Destructor

FeedForwardNeuralNetwork::~FeedForwardNeuralNetwork()
{
    for (std::vector<NetworkLayer *>::size_type i=0; i<_L.size(); ++i)
        {
            delete _L[i];
        }

    _L.clear();
    _L_fed.clear();
    _L_nn.clear();
    _L_in = NULL;
    _L_out = NULL;

    _flag_connected = false;
    _flag_1d = false;
    _flag_2d = false;
    _flag_v1d = false;
    _flag_c1d = false;
    _flag_c2d = false;

    _nvp=0;
}
