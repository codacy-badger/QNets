[![Build Status](https://travis-ci.com/DCM-UPB/FeedForwardNeuralNetwork.svg?branch=master)](https://travis-ci.com/DCM-UPB/FeedForwardNeuralNetwork)
[![Coverage Status](https://coveralls.io/repos/github/DCM-UPB/FeedForwardNeuralNetwork/badge.svg?branch=master)](https://coveralls.io/github/DCM-UPB/FeedForwardNeuralNetwork?branch=master)
[![Codacy Badge](https://api.codacy.com/project/badge/Grade/259f588d9bd44ca88b9e7dce9f83c36b)](https://www.codacy.com/app/DCM-UPB/FeedForwardNeuralNetwork?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=DCM-UPB/FeedForwardNeuralNetwork&amp;utm_campaign=Badge_Grade)
[![CodeFactor](https://www.codefactor.io/repository/github/dcm-upb/feedforwardneuralnetwork/badge)](https://www.codefactor.io/repository/github/dcm-upb/feedforwardneuralnetwork)

# FeedForwardNeuralNetwork

C++ Library for building and using a Feed Forward Neural Network.
It includes first and second derivatives in respect to the input values, and first derivatives in respect to the variational parameters.

In `doc/` there is a user manual in pdf.

In `examples/` there are several examples.



# Build the library

Insert the system parameters in a file named `config.sh` in the root folder (use `script/config_template.sh` as template) and then simply execute the command

   `./build.sh`


## Multi-threading: OpenMP

This library supports multi-threading computation with a shared memory paradigm, thanks to OpenMP.

To activate this feature add the flag `-DOPENMP -fopenmp` to `OPTFLAGS` in the `config.sh` file and recompile the library.
