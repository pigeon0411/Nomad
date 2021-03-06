/*
 * RigidBodyGRFSolver.hpp
 *
 *  Created on: August 21, 2020
 *      Author: Quincy Jones
 *
 * Copyright (c) <2020> <Quincy Jones - quincy@implementedrobotics.com/>
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the Software
 * is furnished to do so, subject to the following conditions:
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef ROBOT_NOMAD_CONTROLLERS_RIGIDBODYGRFSOLVER_H_
#define ROBOT_NOMAD_CONTROLLERS_RIGIDBODYGRFSOLVER_H_

// C System Files

// C++ System Files
#include <iostream>
#include <string>
#include <vector>
#include <memory>

// Project Include Files
#include <OptimalControl/ConvexMatrixEquationSolverQP.hpp>

namespace Robot::Nomad::Controllers
{
    struct ContactState
    {
        int contact;                            // 1 = Contact, 0 = No Contact
        double mu;                              // Friction at Contact
        Eigen::Vector3d pos_world;                    // Position of Contact in WCS (Undefined when not in contact)
        Eigen::MatrixXd J;                      // Contact Jacobian (Undefined when not in contact)
        Eigen::Quaterniond surface_orientation; // Contact Surface Orientation.  (Undefined when not in contact)
                                                // For Normal and Tangetial Friction Cone Computations (Undefined when not in contact)
    };

    class RigidBodyGRFSolverQP : public Core::OptimalControl::ConvexLinearSystemSolverQP
    {

    public:
        // Base Class RigidBodyGRFSolver
        // Simplfified Centroidal Dynamics Solver for Floating Base/Legged Robots
        // num_contacts = Number of ground contacts in system
        RigidBodyGRFSolverQP(const int num_contacts);

        // Body Dynamic Parameters
        // Set Mass - Units = kg
        void SetMass(double mass) { mass_ = mass; }
        void SetCentroidalMOI(double inertia) { I_b_ = Eigen::Matrix3d::Identity() * inertia; }
        void SetCentroidalMOI(Eigen::Vector3d inertia) { I_b_ = inertia.asDiagonal(); }

        // World Parameters
        void SetGravity(Eigen::Vector3d gravity) { gravity_ = gravity; }

        // Set allowable force range for optimization in the surface normal direction
        void SetNormalForceEnvelope(double force_min, double force_max);

        // Set Weights
        void SetAlpha(double alpha) { alpha_ = alpha; }                               // Force Minimization Influence
        void SetBeta(double beta) { beta_ = beta; };                                  // Solution Filtering Influence
        void SetControlWeights(Eigen::VectorXd weights) { S_ = weights.asDiagonal(); }             // Control weights between base position and orientation
        void SetMinimizationWeights(Eigen::VectorXd weights) { W_1_ = weights.asDiagonal(); }      // Force minimization Weight
        void SetSolutionFilteringWeights(Eigen::VectorXd weights) { W_2_ = weights.asDiagonal(); } // Solution Filtering Weight

        // idx = Index of contact
        // contact = New contact state
        void SetContactState(int idx, const ContactState &contact);

        // Set Desired State for Force Computation
        // x = [Θ^T, p^T, ω^T, p_dot^T]^T | Θ = orientation, p = position, ω = angular velocity, p_dot = velocity
        void SetDesiredState(Eigen::VectorXd x);

        // Set Current State for Force Computation
        // x = [Θ^T, p^T, ω^T, p_dot^T]^T | Θ = orientation, p = position, ω = angular velocity, p_dot = velocity
        void SetCurrentState(Eigen::VectorXd x);

        // Update Problem parameters and solve
        void Solve();

    protected:
        // Update QP Problem information prior to solve
        void UpdateConstraints();

        // Normal Force Envelope Values
        double normal_force_min_;
        double normal_force_max_;

        Eigen::Vector3d com_pos_;             // WCS Position of Floating Base CoM
        Eigen::Quaterniond base_orientation_; // Orientation of Floating Base

        // State Information
        Eigen::VectorXd x_desired_; // Desired State
        Eigen::VectorXd x_;         // Current State

        // Translational P/D Gains (Positive Definite Diagonal Matrix)
        Eigen::Matrix3d K_p_com_;
        Eigen::Matrix3d K_d_com_;

        // Orientation P/D Gains (Positive Definite Diagonal Matrix)
        Eigen::Matrix3d K_p_base_;
        Eigen::Matrix3d K_d_base_;

        Eigen::Vector3d gravity_; // World Gravity
        Eigen::Matrix3d I_b_;     // System Inertia ( Body Coordinate System )
        double mass_;             // System mass

        std::vector<ContactState> contacts_; // List of contacts in system
        int num_contacts_; // Number of ground contacts in system
    };
} // namespace Robot::Nomad::Controllers

#endif // ROBOT_NOMAD_CONTROLLERS_RIGIDBODYGRFSOLVER_H_
