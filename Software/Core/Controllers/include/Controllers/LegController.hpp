/*
 * LegController.hpp
 *
 *  Created on: July 13, 2019
 *      Author: Quincy Jones
 *
 * Copyright (c) <2019> <Quincy Jones - quincy@implementedrobotics.com/>
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

#ifndef NOMAD_CORE_CONTROLLERS_LEGCONTROLLER_H_
#define NOMAD_CORE_CONTROLLERS_LEGCONTROLLER_H_

// C System Files
#include <stdint.h>

// C++ System Files
#include <iostream>
#include <string>

// Third Party Includes
#include <Eigen/Dense>
#include <Communications/Messages/double_vec_t.hpp>
#include <Communications/Messages/generic_msg_t.hpp>
#include <Controllers/Messages/leg_controller_cmd_t.hpp>
#include <Controllers/Messages/joint_control_cmd_t.hpp>

// Project Includes
#include <Systems/SystemBlock.hpp>

namespace Controllers::Locomotion
{

    class LegController : public Core::Systems::SystemBlock
    {

    public:
        // TODO: Evaluate breaking these down into single message type, LegControllerSetpoint_t, LegControllerState_t etc
        enum OutputPort
        {
            SERVO_COMMAND = 0,
            NUM_OUTPUTS = 1
        };

        enum InputPort
        {
            LEG_COMMAND = 0,
            NUM_INPUTS = 1
        };
        // FL = 0, FR = 1, RL = 2, RR = 3

        // Base Class Leg ControllerTask Node
        // T_s = System sampling time
        LegController(const double T_s = -1);

    protected:
        
        // Update function for stateful outputs
        void UpdateStateOutputs();

        // Update function for stateless outputs
        void UpdateStatelessOutputs();

        // Update fucntion for next state from inputs
        void UpdateState();

        // Number of legs
        unsigned int num_legs_;

        // Number of dofs per each leg
        unsigned int num_dofs_;

        // Total number of dofs
        unsigned int total_dofs_;

        // Input (Messages)
        leg_controller_cmd_t leg_command_input_;

        // Output (Messages)
        joint_control_cmd_t servo_command_output_;

        // TODO: Set these at compile time?
        Eigen::VectorXd torque_ff_out_;
        Eigen::VectorXd q_out_;
        Eigen::VectorXd q_d_out_;

        Eigen::MatrixXd k_P_joint_;
        Eigen::MatrixXd k_D_joint_;

        Eigen::MatrixXd k_P_cartesian_;
        Eigen::MatrixXd k_D_cartesian_;

        Eigen::VectorXd foot_pos_;
        Eigen::VectorXd foot_vel_;

        // Jacobian Matrix
        Eigen::MatrixXd J_;

        void ResetState();
    };
} // namespace Controllers::Locomotion

#endif // NOMAD_CORE_CONTROLLERS_STATEESTIMATOR_H_
