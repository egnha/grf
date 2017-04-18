
/*-------------------------------------------------------------------------------
 This file is part of gradient-forest.
 gradient-forest is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 gradient-forest is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 GNU General Public License for more details.
 You should have received a copy of the GNU General Public License
 along with gradient-forest. If not, see <http://www.gnu.org/licenses/>.
 #-------------------------------------------------------------------------------*/



#include <cmath>
#include <string>
#include <vector>
#include <iostream>
#include "eigen3/Eigen/Dense"
#include "commons/utility.h"
#include "commons/Observations.h"
#include "prediction/LocallyLinearPredictionStrategy.h"

const size_t LocallyLinearPredictionStrategy::OUTCOME = 0;

size_t LocallyLinearPredictionStrategy::prediction_length() {
    return 1;
}

// currently just assuming lambda = 0.01. THIS OBVIOUSLY NEEDS TO BE CHANGED. But the goal is to see if the method works.

Prediction LocallyLinearPredictionStrategy::predict(size_t sampleID,
                                                    const std::vector<double>& average_prediction_values,
                                                    const std::unordered_map<size_t, double>& weights_by_sampleID,
                                                    const Observations& observations,
                                                    const Data *data) {
    
    int n = observations.get_num_samples(); // usage correct?
    int p = data->num_cols; // () or not?
    
    // initialize weight matrix
    Eigen::Matrix<float, n, n> weights = Eigen::Matrix<float, n, n>::Zero();
    
    
    // loop through all leaves and update weight matrix
    for (auto it = weights_by_sampleID.begin(); it != weights_by_sampleID.end(); ++it){
        size_t i = it->first;
        float weight = it->second;
        weights(i,i) << weight;
    }
    
    // we now move on to the local linear prediction assuming X has been formatted correctly
    
    //size_t p = Data::num_cols(data); // double check this method
    Eigen::Matrix<float, n, p> X;
    Eigen::Matrix<float, n, 1> Y;
    
    // loop through observations to fill in X, Y
    for(size_t i=0; i<n; ++i){
        for(size_t j=0; j<p; ++j){
            X.block<1,1>(i,j) << data->get(i,j);
        }
        Y.block<1,1>(i,0) << observations.get(Observations::OUTCOME, i);
    }
    
    // Pre-compute M = X^T X + lambda J
    float lambda = 0.01;
    Eigen::Matrix<float, p, p> Id = Eigen::Matrix<float, n, n>::Identity();
    
    Eigen::Matrix<float, p, p> J = Id;
    J(0,0) = 0;
    
    Eigen::Matrix<float, p, p> M = X.transpose()*weights*X + J*lambda;
    Eigen::Matrix<float, p, p> M_inverse = M.colPivHouseholderQr().solve(Id);
    
    Eigen::Matrix<float, 1, p> theta = M_inverse*X.transpose()*weights*Y;
    
    // fill in new data matrix
    int num_input_points = data->num_rows(); // usage correct?
    Eigen::Matrix<double, num_input_points, p> input_data_eigen;
    for(size_t i=0; i<num_input_points; ++i){
        for(size_t j=0; j<p; ++j){
            input_data_eigen.block<1,1>(i,j) << data->get(i,j);
        }
    }
    Eigen::Matrix<double,num_input_points,1> predictions = input_data_eigen.transpose()*theta;
    
    return Prediction(predictions);
}

// now defining dummy methods to see if the compiler stops complaining to me about pure virtual methods

Prediction LocallyLinearPredictionStrategy::predict_with_variance(size_t sampleID,
                                                                  const std::vector<std::vector<size_t>>& leaf_sampleIDs
                                                                  const std::unordered_map<size_t, double>& weights_by_sampleID,
                                                                  const Observations& observations) {
    throw std::runtime_error("Variance estimates are not yet implemented.");
}

bool LocallyLinearPredictionStrategy::requires_leaf_sample_IDs(){
    return false;
}

PredictionValues precompute_prediction_values(const std::vector<std::vector<size_t>> leaf_sampleIDs),
                                              const Observations& observations){
    throw std::runtime_error("Not implemented yet.");
}
