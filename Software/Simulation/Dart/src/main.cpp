#include <Eigen/Dense>
//#include <dart/collision/bullet/BulletCollisionDetector.hpp>
#include <dart/dynamics/Skeleton.hpp>
#include <dart/dynamics/DegreeOfFreedom.hpp>
#include <dart/dynamics/FreeJoint.hpp>
#include <dart/dynamics/WeldJoint.hpp>
#include <dart/dynamics/SphereShape.hpp>
#include <dart/simulation/World.hpp>
#include <dart/utils/urdf/DartLoader.hpp>
#include <dart/constraint/ConstraintSolver.hpp>
#include <dart/gui/osg/osg.hpp>
#include <dart/gui/osg/TrackballManipulator.hpp>


#include <Nomad/MessageTypes/sim_data_t.hpp>
#include <Nomad/MessageTypes/joint_control_cmd_t.hpp>
#include <condition_variable>
#include <vector>
#include "../include/NomadRobot.h"

// Third Party Includes
#include <zcm/zcm-cpp.hpp>

using namespace dart::dynamics;
using namespace dart::simulation;

WorldPtr g_world;
std::shared_ptr<NomadRobot> g_nomad;
Eigen::VectorXd joint_torques_cmd = Eigen::VectorXd::Zero(18);

std::mutex mtx;
std::condition_variable cv;

bool b_first = true;

class NomadSimWorldNode : public dart::gui::osg::RealTimeWorldNode
{

public:
  explicit NomadSimWorldNode(WorldPtr world, const std::shared_ptr<NomadRobot> nomad)
      : dart::gui::osg::RealTimeWorldNode(world), nomad_(nomad), world_(world)
  {
    // World gets saved as -> mWorld if ever needed.  Also ->getWorld()
    Eigen::Isometry3d tf = Eigen::Isometry3d::Identity();

    // TODO: Pass IP Address as a parameter
    context_ = std::make_unique<zcm::ZCM>("udpm://239.255.76.67:7667?ttl=0");

    std::cout << "Nomad DART Sim connecting." << std::endl;

    context_->subscribe("nomad.sim.joint_cmd", &NomadSimWorldNode::OnJointControlMsg, this);

    // TODO: Publish state back
    pub_context_ = std::make_unique<zcm::ZCM>("udpm://239.255.76.67:7667?ttl=0");

    sequence_num_ = 0;

    step_iter = 0;
  }

  void ReceiveCommand()
  {
    context_->handle();
  }

  void customPreStep()
  {
    if(step_iter < 10)
    {
      return;
    }
    if(b_first)
    {
      PublishState();
      b_first = false;
    }

    ReceiveCommand();

   nomad_->Skeleton()->setForces(joint_torques_cmd);
  }

  void customPostStep()
  {
    PublishState();

    //joint_torques = Eigen::VectorXd::Zero(18);
    step_iter++;
  }

  void OnJointControlMsg(const zcm::ReceiveBuffer *rbuf, const std::string &chan, const joint_control_cmd_t *msg)
  {
    // Read Input

    Eigen::VectorXd tau_cmd(12);

    tau_cmd[0] = msg->tau_ff[0];
    tau_cmd[1] = msg->tau_ff[1];
    tau_cmd[2] = msg->tau_ff[2];
    tau_cmd[3] = msg->tau_ff[3];
    tau_cmd[4] = msg->tau_ff[4];
    tau_cmd[5] = msg->tau_ff[5];
    tau_cmd[6] = msg->tau_ff[6];
    tau_cmd[7] = msg->tau_ff[7];
    tau_cmd[8] = msg->tau_ff[8];
    tau_cmd[9] = msg->tau_ff[9];
    tau_cmd[10] = msg->tau_ff[10];
    tau_cmd[11] = msg->tau_ff[11];

    joint_torques_cmd.tail(12) = tau_cmd;
  }

  void PublishState()
  {
    sim_data_t sim_data;

    uint64_t time_now = sequence_num_;

    // Get Relevant State Data
    Eigen::Quaterniond body_orientation = nomad_->GetBaseOrientation();
    Eigen::Vector3d body_pos = nomad_->GetBasePosition(); // World
    Eigen::Vector3d accel_body = nomad_->GetBaseLinearAcceleration(); // Body
    Eigen::Vector3d angular_body = nomad_->GetBaseAngularVelocity(); // Body
    Eigen::Vector3d vel_body = nomad_->GetBaseLinearVelocity(); // Body

    // Cheater Orientation
    sim_data.timestamp = time_now;
    sim_data.sequence_num = sequence_num_;

    // Copy Body World Position
    Eigen::Map<Eigen::VectorXd>(sim_data.com_pos, 3) = body_pos;

    // Copy Body World Velocity
    Eigen::Map<Eigen::VectorXd>(sim_data.com_vel, 3) = body_orientation.toRotationMatrix() * vel_body;

    // Copy Body Orientation 
    sim_data.com_orientation[0] = body_orientation.x();
    sim_data.com_orientation[1] = body_orientation.y();
    sim_data.com_orientation[2] = body_orientation.z();
    sim_data.com_orientation[3] = body_orientation.w();

    // Copy Body Angular Velocity (World)
    Eigen::Map<Eigen::VectorXd>(sim_data.com_omega, 3) = body_orientation.toRotationMatrix() * angular_body;

    // IMU Orientation
    // Copy Body Orientation 
    sim_data.imu_orientation[0] = body_orientation.x();
    sim_data.imu_orientation[1] = body_orientation.y();
    sim_data.imu_orientation[2] = body_orientation.z();
    sim_data.imu_orientation[3] = body_orientation.w();

    // IMU Linear Acceleration
    // Accelerometer (Local)
    Eigen::Map<Eigen::VectorXd>(sim_data.imu_accel, 3) = accel_body;

    // Gyro
    // Copy Body Angular Velocity (Local)
    Eigen::Map<Eigen::VectorXd>(sim_data.imu_omega, 3) = angular_body;

    // Copy Joint State "Estimates"
    Eigen::VectorXd joint_pos = nomad_->Skeleton()->getPositions();
    Eigen::VectorXd joint_vel = nomad_->Skeleton()->getVelocities();
    Eigen::VectorXd joint_tau = nomad_->Skeleton()->getForces();

    sim_data.q[0] = joint_pos[6];
    sim_data.q[1] = joint_pos[7];
    sim_data.q[2] = joint_pos[8];
    sim_data.q[3] = joint_pos[9];
    sim_data.q[4] = joint_pos[10];
    sim_data.q[5] = joint_pos[11];
    sim_data.q[6] = joint_pos[12];
    sim_data.q[7] = joint_pos[13];
    sim_data.q[8] = joint_pos[14];
    sim_data.q[9] = joint_pos[15];
    sim_data.q[10] = joint_pos[16];
    sim_data.q[11] = joint_pos[17];

    sim_data.q_dot[0] = joint_vel[6];
    sim_data.q_dot[1] = joint_vel[7];
    sim_data.q_dot[2] = joint_vel[8];
    sim_data.q_dot[3] = joint_vel[9];
    sim_data.q_dot[4] = joint_vel[10];
    sim_data.q_dot[5] = joint_vel[11];
    sim_data.q_dot[6] = joint_vel[12];
    sim_data.q_dot[7] = joint_vel[13];
    sim_data.q_dot[8] = joint_vel[14];
    sim_data.q_dot[9] = joint_vel[15];
    sim_data.q_dot[10] = joint_vel[16];
    sim_data.q_dot[11] = joint_vel[17];

    sim_data.tau[0] = joint_tau[6];
    sim_data.tau[1] = joint_tau[7];
    sim_data.tau[2] = joint_tau[8];
    sim_data.tau[3] = joint_tau[9];
    sim_data.tau[4] = joint_tau[10];
    sim_data.tau[5] = joint_tau[11];
    sim_data.tau[6] = joint_tau[12];
    sim_data.tau[7] = joint_tau[13];
    sim_data.tau[8] = joint_tau[14];
    sim_data.tau[9] = joint_tau[15];
    sim_data.tau[10] = joint_tau[16];
    sim_data.tau[11] = joint_tau[17];

    sequence_num_++;

    // Publish
    int rc = pub_context_->publish("nomad.sim.data", &sim_data);
    //pub_context_->flush();
  }

protected:
  std::shared_ptr<NomadRobot> nomad_;
  WorldPtr world_;

  std::unique_ptr<zcm::ZCM> context_;

  std::unique_ptr<zcm::ZCM> pub_context_;

  int step_iter;

  uint64_t sequence_num_;

};

SkeletonPtr CreateGround()
{
  SkeletonPtr ground = Skeleton::create("ground");

  // Give the ground a body
  BodyNodePtr body = ground->createJointAndBodyNodePair<WeldJoint>(nullptr).second;
  double thickness = 0.1;

  ShapePtr groundShape = std::make_shared<BoxShape>(Eigen::Vector3d(4, 4, thickness));
  auto shapeNode = body->createShapeNodeWith<VisualAspect, CollisionAspect, DynamicsAspect>(groundShape);

  shapeNode->getVisualAspect()->setColor(dart::Color::LightGray());

  Eigen::Isometry3d tf(Eigen::Isometry3d::Identity());
  tf.translation() = Eigen::Vector3d(0, 0, -thickness / 2.0);
  body->getParentJoint()->setTransformFromParentBodyNode(tf);

  return ground;
}

int main(int argc, char *argv[])
{

  // Create dart simulation world
  g_world = World::create();

  // Update Gravity vector to Z down
  Eigen::Vector3d gravity(0.0, 0.0, -9.81);
  //Eigen::Vector3d gravity(0.0, 0.0, 0.0);
  g_world->setGravity(gravity);

  // Create and add ground to world
  SkeletonPtr ground = CreateGround();
  g_world->addSkeleton(ground);

  // Update Collision
  //g_world->getConstraintSolver()->setCollisionDetector(dart::collision::BulletCollisionDetector::create());

  g_nomad = std::make_shared<NomadRobot>(g_world);

  std::string urdf = std::getenv("NOMAD_RESOURCE_PATH");
  urdf.append("/Robot/Nomad.urdf");

  g_nomad->LoadFromURDF(urdf);
  g_nomad->SetInitialPose();

  // Create osg world node
  ::osg::ref_ptr<NomadSimWorldNode> node = new NomadSimWorldNode(g_world, g_nomad);

  // Create osg Viewer
  dart::gui::osg::Viewer viewer = dart::gui::osg::Viewer();
  viewer.addWorldNode(node);

  // Default with Headlights
  viewer.switchHeadlights(false);

  //Add Grid Attachment
  ::osg::ref_ptr<dart::gui::osg::GridVisual> grid = new dart::gui::osg::GridVisual();

  viewer.addAttachment(grid);

  viewer.addAttachment(
      new dart::gui::osg::SupportPolygonVisual(g_nomad->Skeleton(), 0.025));


  // Print out instructions for the viewer
  std::cout << viewer.getInstructions() << std::endl;

  viewer.setUpViewInWindow(0, 0, 1280, 1024);
  viewer.getCameraManipulator()->setHomePosition(::osg::Vec3(0.0f, -5.0f, 0.0f),
                                                 ::osg::Vec3(0.0f, 0.0f, 0.0f),
                                                 ::osg::Vec3(0.0f, 0.707f, 0.707f), false);

  viewer.setCameraManipulator(viewer.getCameraManipulator());
  viewer.simulate(true);
  viewer.run();

  return 0;
}
