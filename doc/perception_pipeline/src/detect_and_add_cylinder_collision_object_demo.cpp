/*********************************************************************
 * Software License Agreement (BSD License)
 *
 *  Copyright (c) 2018, Ridhwan Luthra.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.
 *   * Neither the name of Ridhwan Luthra nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 *  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 *  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 *********************************************************************/

/* Author: Ridhwan Luthra */

#include <ros/ros.h>
#include <sensor_msgs/PointCloud2.h>
#include <pcl_conversions/pcl_conversions.h>
#include <pcl/filters/passthrough.h>
#include <pcl/filters/extract_indices.h>
#include <pcl/features/normal_3d.h>
#include <pcl/segmentation/sac_segmentation.h>
#include <moveit/move_group_interface/move_group_interface.h>
#include <tf2_geometry_msgs/tf2_geometry_msgs.h>

#include <moveit/planning_scene_interface/planning_scene_interface.h>
#include <moveit_msgs/CollisionObject.h>
const double tau = 2 * M_PI;

class CylinderSegment
{
  ros::NodeHandle nh_;
  moveit::planning_interface::PlanningSceneInterface planning_scene_interface_;
  ros::Subscriber cloud_subscriber_;

public:
  CylinderSegment()
    : cloud_subscriber_(nh_.subscribe("/camera/depth_registered/points", 1, &CylinderSegment::cloudCB, this))
  {
  }

  /** \brief Given the parameters of the cylinder add the cylinder to the planning scene. */
  void addCylinder()
  {
    // BEGIN_SUB_TUTORIAL add_cylinder
    //
    // Adding Cylinder to Planning Scene
    // ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
    // Define a collision object ROS message.
    moveit_msgs::CollisionObject collision_object;
    collision_object.header.frame_id = "camera_rgb_optical_frame";
    collision_object.id = "cylinder";

    // Define a cylinder which will be added to the world.
    shape_msgs::SolidPrimitive primitive;
    primitive.type = primitive.CYLINDER;
    primitive.dimensions.resize(2);
    /* Setting height of cylinder. */
    primitive.dimensions[0] = cylinder_params.height;
    /* Setting radius of cylinder. */
    primitive.dimensions[1] = cylinder_params.radius;

    // Define a pose for the cylinder (specified relative to frame_id).
    geometry_msgs::Pose cylinder_pose;
    /* Computing and setting quaternion from axis angle representation. */
    Eigen::Vector3d cylinder_z_direction(cylinder_params.direction_vec[0], cylinder_params.direction_vec[1],
                                         cylinder_params.direction_vec[2]);
    Eigen::Vector3d origin_z_direction(0., 0., 1.);
    Eigen::Vector3d axis;
    axis = origin_z_direction.cross(cylinder_z_direction);
    axis.normalize();
    double angle = acos(cylinder_z_direction.dot(origin_z_direction));
    cylinder_pose.orientation.x = axis.x() * sin(angle / 2);
    cylinder_pose.orientation.y = axis.y() * sin(angle / 2);
    cylinder_pose.orientation.z = axis.z() * sin(angle / 2);
    cylinder_pose.orientation.w = cos(angle / 2);

    // Setting the position of cylinder.
    cylinder_pose.position.x = cylinder_params.center_pt[0];
    cylinder_pose.position.y = cylinder_params.center_pt[1];
    cylinder_pose.position.z = cylinder_params.center_pt[2];

    // Add cylinder as collision object
    collision_object.primitives.push_back(primitive);
    collision_object.primitive_poses.push_back(cylinder_pose);
    collision_object.id="object";
    collision_object.operation = collision_object.ADD;
    planning_scene_interface_.applyCollisionObject(collision_object);
    // END_SUB_TUTORIAL
  }



void addCollisionObjects()
{
  // BEGIN_SUB_TUTORIAL table1
  //
  // Creating Environment
  // ^^^^^^^^^^^^^^^^^^^^
  // Create vector to hold 3 collision objects.
  std::vector<moveit_msgs::CollisionObject> collision_objects;
  collision_objects.resize(2);

  // Add the first table where the cube will originally be kept.
  collision_objects[0].id = "table1";
  collision_objects[0].header.frame_id = "panda_link0";

  /* Define the primitive and its dimensions. */
  collision_objects[0].primitives.resize(1);
  collision_objects[0].primitives[0].type = collision_objects[0].primitives[0].BOX;
  collision_objects[0].primitives[0].dimensions.resize(3);
  collision_objects[0].primitives[0].dimensions[0] = 0.03;
  collision_objects[0].primitives[0].dimensions[1] = 0.03;
  collision_objects[0].primitives[0].dimensions[2] = 0.9;

  /* Define the pose of the table. */
  collision_objects[0].primitive_poses.resize(1);
  collision_objects[0].primitive_poses[0].position.x = 0.5;
  collision_objects[0].primitive_poses[0].position.y = 0;
  collision_objects[0].primitive_poses[0].position.z = 0.45;
  collision_objects[0].primitive_poses[0].orientation.w = 1.0;
  // END_SUB_TUTORIAL

  collision_objects[0].operation = collision_objects[0].ADD;

  // BEGIN_SUB_TUTORIAL table2
  // Add the second table where we will be placing the cube.
  collision_objects[1].id = "table2";
  collision_objects[1].header.frame_id = "panda_link0";

  /* Define the primitive and its dimensions. */
  collision_objects[1].primitives.resize(1);
  collision_objects[1].primitives[0].type = collision_objects[1].primitives[0].BOX;
  collision_objects[1].primitives[0].dimensions.resize(3);
  collision_objects[1].primitives[0].dimensions[0] = 0.03;
  collision_objects[1].primitives[0].dimensions[1] = 0.03;
  collision_objects[1].primitives[0].dimensions[2] = 0.9;

  /* Define the pose of the table. */
  collision_objects[1].primitive_poses.resize(1);
  collision_objects[1].primitive_poses[0].position.x = 0.15;
  collision_objects[1].primitive_poses[0].position.y = 0.5;
  collision_objects[1].primitive_poses[0].position.z = 0.45;
  collision_objects[1].primitive_poses[0].orientation.w = 1.0;
  // END_SUB_TUTORIAL

  collision_objects[1].operation = collision_objects[1].ADD;
  planning_scene_interface_.applyCollisionObjects(collision_objects);
}








  /** \brief Given the pointcloud containing just the cylinder,
      compute its center point and its height and store in cylinder_params.
      @param cloud - point cloud containing just the cylinder. */
  void extractLocationHeight(const pcl::PointCloud<pcl::PointXYZ>::Ptr& cloud)
  {
    double max_angle_y = -std::numeric_limits<double>::infinity();
    double min_angle_y = std::numeric_limits<double>::infinity();

    double lowest_point[3] = { 0.0, 0.0, 0.0 };
    double highest_point[3] = { 0.0, 0.0, 0.0 };
    // BEGIN_SUB_TUTORIAL extract_location_height
    // Consider a point inside the point cloud and imagine that point is formed on a XY plane where the perpendicular
    // distance from the plane to the camera is Z. |br|
    // The perpendicular drawn from the camera to the plane hits at center of the XY plane. |br|
    // We have the x and y coordinate of the point which is formed on the XY plane. |br|
    // X is the horizontal axis and Y is the vertical axis. |br|
    // C is the center of the plane which is Z meter away from the center of camera and A is any point on the plane.
    // |br|
    // Now we know Z is the perpendicular distance from the point to the camera. |br|
    // If you need to find the  actual distance d from the point to the camera, you should calculate the hypotenuse-
    // |code_start| hypot(point.z, point.x);\ |code_end| |br|
    // angle the point made horizontally- |code_start| atan2(point.z,point.x);\ |code_end| |br|
    // angle the point made Vertically- |code_start| atan2(point.z, point.y);\ |code_end| |br|
    // Loop over the entire pointcloud.
    for (auto const point : cloud->points)
    {
      const double angle = atan2(point.z, point.y);
      /* Find the coordinates of the highest point */
      if (angle < min_angle_y)
      {
        min_angle_y = angle;
        lowest_point[0] = point.x;
        lowest_point[1] = point.y;
        lowest_point[2] = point.z;
      }
      /* Find the coordinates of the lowest point */
      else if (angle > max_angle_y)
      {
        max_angle_y = angle;
        highest_point[0] = point.x;
        highest_point[1] = point.y;
        highest_point[2] = point.z;
      }
    }
    /* Store the center point of cylinder */
    cylinder_params.center_pt[0] = (highest_point[0] + lowest_point[0]) / 2;
    cylinder_params.center_pt[1] = (highest_point[1] + lowest_point[1]) / 2;
    cylinder_params.center_pt[2] = (highest_point[2] + lowest_point[2]) / 2;
    /* Store the height of cylinder */
    cylinder_params.height =
        sqrt(pow((lowest_point[0] - highest_point[0]), 2) + pow((lowest_point[1] - highest_point[1]), 2) +
             pow((lowest_point[2] - highest_point[2]), 2));
    // END_SUB_TUTORIAL
  }

  /** \brief Given a pointcloud extract the ROI defined by the user.
      @param cloud - Pointcloud whose ROI needs to be extracted. */
  void passThroughFilter(const pcl::PointCloud<pcl::PointXYZ>::Ptr& cloud)
  {
    pcl::PassThrough<pcl::PointXYZ> pass;
    pass.setInputCloud(cloud);
    pass.setFilterFieldName("z");
    // min and max values in z axis to keep
    pass.setFilterLimits(0.3, 1.1);
    pass.filter(*cloud);
  }

  /** \brief Given the pointcloud and pointer cloud_normals compute the point normals and store in cloud_normals.
      @param cloud - Pointcloud.
      @param cloud_normals - The point normals once computer will be stored in this. */
  void computeNormals(const pcl::PointCloud<pcl::PointXYZ>::Ptr& cloud,
                      const pcl::PointCloud<pcl::Normal>::Ptr& cloud_normals)
  {
    pcl::search::KdTree<pcl::PointXYZ>::Ptr tree(new pcl::search::KdTree<pcl::PointXYZ>());
    pcl::NormalEstimation<pcl::PointXYZ, pcl::Normal> ne;
    ne.setSearchMethod(tree);
    ne.setInputCloud(cloud);
    // Set the number of k nearest neighbors to use for the feature estimation.
    ne.setKSearch(50);
    ne.compute(*cloud_normals);
  }

  /** \brief Given the point normals and point indices, extract the normals for the indices.
      @param cloud_normals - Point normals.
      @param inliers_plane - Indices whose normals need to be extracted. */
  void extractNormals(const pcl::PointCloud<pcl::Normal>::Ptr& cloud_normals,
                      const pcl::PointIndices::Ptr& inliers_plane)
  {
    pcl::ExtractIndices<pcl::Normal> extract_normals;
    extract_normals.setNegative(true);
    extract_normals.setInputCloud(cloud_normals);
    extract_normals.setIndices(inliers_plane);
    extract_normals.filter(*cloud_normals);
  }

  /** \brief Given the pointcloud and indices of the plane, remove the planar region from the pointcloud.
      @param cloud - Pointcloud.
      @param inliers_plane - Indices representing the plane. */
  void removePlaneSurface(const pcl::PointCloud<pcl::PointXYZ>::Ptr& cloud, const pcl::PointIndices::Ptr& inliers_plane)
  {
    // create a SAC segmenter without using normals
    pcl::SACSegmentation<pcl::PointXYZ> segmentor;
    segmentor.setOptimizeCoefficients(true);
    segmentor.setModelType(pcl::SACMODEL_PLANE);
    segmentor.setMethodType(pcl::SAC_RANSAC);
    /* run at max 1000 iterations before giving up */
    segmentor.setMaxIterations(1000);
    /* tolerance for variation from model */
    segmentor.setDistanceThreshold(0.01);
    segmentor.setInputCloud(cloud);
    /* Create the segmentation object for the planar model and set all the parameters */
    pcl::ModelCoefficients::Ptr coefficients_plane(new pcl::ModelCoefficients);
    segmentor.segment(*inliers_plane, *coefficients_plane);
    /* Extract the planar inliers from the input cloud */
    pcl::ExtractIndices<pcl::PointXYZ> extract_indices;
    extract_indices.setInputCloud(cloud);
    extract_indices.setIndices(inliers_plane);
    /* Remove the planar inliers, extract the rest */
    extract_indices.setNegative(true);
    extract_indices.filter(*cloud);
  }

  /** \brief Given the pointcloud, pointer to pcl::ModelCoefficients and point normals extract the cylinder from the
     pointcloud and store the cylinder parameters in coefficients_cylinder.
      @param cloud - Pointcloud whose plane is removed.
      @param coefficients_cylinder - Cylinder parameters used to define an infinite cylinder will be stored here.
      @param cloud_normals - Point normals corresponding to the plane on which cylinder is kept */
  void extractCylinder(const pcl::PointCloud<pcl::PointXYZ>::Ptr& cloud,
                       const pcl::ModelCoefficients::Ptr& coefficients_cylinder,
                       const pcl::PointCloud<pcl::Normal>::Ptr& cloud_normals)
  {
    // Create the segmentation object for cylinder segmentation and set all the parameters
    pcl::SACSegmentationFromNormals<pcl::PointXYZ, pcl::Normal> segmentor;
    pcl::PointIndices::Ptr inliers_cylinder(new pcl::PointIndices);
    segmentor.setOptimizeCoefficients(true);
    segmentor.setModelType(pcl::SACMODEL_CYLINDER);
    segmentor.setMethodType(pcl::SAC_RANSAC);
    // Set the normal angular distance weight
    segmentor.setNormalDistanceWeight(0.1);
    // run at max 1000 iterations before giving up
    segmentor.setMaxIterations(1000);
    // tolerance for variation from model
    segmentor.setDistanceThreshold(0.008);
    // min max values of radius in meters to consider
    segmentor.setRadiusLimits(0.01, 0.1);
    segmentor.setInputCloud(cloud);
    segmentor.setInputNormals(cloud_normals);

    // Obtain the cylinder inliers and coefficients
    segmentor.segment(*inliers_cylinder, *coefficients_cylinder);

    // Extract the cylinder inliers from the input cloud
    pcl::ExtractIndices<pcl::PointXYZ> extract;
    extract.setInputCloud(cloud);
    extract.setIndices(inliers_cylinder);
    extract.setNegative(false);
    extract.filter(*cloud);
  }





  void cloudCB(const sensor_msgs::PointCloud2ConstPtr& input)
  {
    // BEGIN_SUB_TUTORIAL callback
    //
    // Perception Related
    // ^^^^^^^^^^^^^^^^^^
    // This section uses a standard PCL-based processing pipeline to estimate a cylinder's pose in the point cloud.
    //
    // First, we convert from sensor_msgs to pcl::PointXYZ which is needed for most of the processing.
    pcl::PointCloud<pcl::PointXYZ>::Ptr cloud(new pcl::PointCloud<pcl::PointXYZ>);
    pcl::fromROSMsg(*input, *cloud);
    // Use a passthrough filter to get the region of interest.
    // The filter removes points outside the specified range.
    passThroughFilter(cloud);
    // Compute point normals for later sample consensus step.
    pcl::PointCloud<pcl::Normal>::Ptr cloud_normals(new pcl::PointCloud<pcl::Normal>);
    computeNormals(cloud, cloud_normals);
    // inliers_plane will hold the indices of the point cloud that correspond to a plane.
    pcl::PointIndices::Ptr inliers_plane(new pcl::PointIndices);
    // Detect and remove points on the (planar) surface on which the cylinder is resting.
    removePlaneSurface(cloud, inliers_plane);
    // Remove surface points from normals as well
    extractNormals(cloud_normals, inliers_plane);
    // ModelCoefficients will hold the parameters using which we can define a cylinder of infinite length.
    // It has a public attribute |code_start| values\ |code_end| of type |code_start| std::vector<float>\ |code_end|\ .
    // |br|
    // |code_start| values[0-2]\ |code_end| hold a point on the center line of the cylinder. |br|
    // |code_start| values[3-5]\ |code_end| hold direction vector of the z-axis. |br|
    // |code_start| values[6]\ |code_end| is the radius of the cylinder.
    pcl::ModelCoefficients::Ptr coefficients_cylinder(new pcl::ModelCoefficients);
    /* Extract the cylinder using SACSegmentation. */
    extractCylinder(cloud, coefficients_cylinder, cloud_normals);
    // END_SUB_TUTORIAL
    if (cloud->points.empty() || coefficients_cylinder->values.size() != 7)
    {
      ROS_ERROR_STREAM_NAMED("cylinder_segment", "Can't find the cylindrical component.");
      return;
    }

    ROS_INFO("Detected Cylinder - Adding CollisionObject to PlanningScene");

    // BEGIN_TUTORIAL
    // CALL_SUB_TUTORIAL callback
    //
    // Storing Relevant Cylinder Values
    // ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
    // The information that we have in |code_start| coefficients_cylinder\ |code_end| is not enough to define our
    // cylinder.
    // It does not have the actual location of the cylinder nor the actual height. |br|
    // We define a struct to hold the parameters that are actually needed for defining a collision object completely.
    // |br|
    // CALL_SUB_TUTORIAL param_struct
    /* Store the radius of the cylinder. */
    cylinder_params.radius = coefficients_cylinder->values[6];
    /* Store direction vector of z-axis of cylinder. */
    cylinder_params.direction_vec[0] = coefficients_cylinder->values[3];
    cylinder_params.direction_vec[1] = coefficients_cylinder->values[4];
    cylinder_params.direction_vec[2] = coefficients_cylinder->values[5];
    //
    // Extracting Location and Height
    // ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
    // Compute the center point of the cylinder using standard geometry
    extractLocationHeight(cloud);
    // CALL_SUB_TUTORIAL extract_location_height
    // Use the parameters extracted to add the cylinder to the planning scene as a collision object.
    addCylinder();
    addCollisionObjects();
    // CALL_SUB_TUTORIAL add_cylinder
    // END_TUTORIAL

  }

private:
  // BEGIN_SUB_TUTORIAL param_struct
  // There are 4 fields and a total of 7 parameters used to define this.
  struct AddCylinderParams
  {
    /* Radius of the cylinder. */
    double radius;
    /* Direction vector along the z-axis of the cylinder. */
    double direction_vec[3];
    /* Center point of the cylinder. */
    double center_pt[3];
    /* Height of the cylinder. */
    double height;
  };
  // Declare a variable of type AddCylinderParams and store relevant values from ModelCoefficients.
  AddCylinderParams cylinder_params;
  // END_SUB_TUTORIAL
};

// void openGripper(trajectory_msgs::JointTrajectory& posture)
// {
//   // BEGIN_SUB_TUTORIAL open_gripper
//   /* Add both finger joints of panda robot. */
//   posture.joint_names.resize(2);
//   posture.joint_names[0] = "panda_finger_joint1";
//   posture.joint_names[1] = "panda_finger_joint2";

//   /* Set them as open, wide enough for the object to fit. */
//   posture.points.resize(1);
//   posture.points[0].positions.resize(2);
//   posture.points[0].positions[0] = 0.04;
//   posture.points[0].positions[1] = 0.04;
//   posture.points[0].time_from_start = ros::Duration(0.5);
//   // END_SUB_TUTORIAL
// }

// void closedGripper(trajectory_msgs::JointTrajectory& posture)
// {
//   // BEGIN_SUB_TUTORIAL closed_gripper
//   /* Add both finger joints of panda robot. */
//   posture.joint_names.resize(2);
//   posture.joint_names[0] = "panda_finger_joint1";
//   posture.joint_names[1] = "panda_finger_joint2";

//   /* Set them as closed. */
//   posture.points.resize(1);
//   posture.points[0].positions.resize(2);
//   posture.points[0].positions[0] = 0.00;
//   posture.points[0].positions[1] = 0.00;
//   posture.points[0].time_from_start = ros::Duration(0.5);
//   // END_SUB_TUTORIAL
// }


// void pick(moveit::planning_interface::MoveGroupInterface& move_group)
// {
//   // BEGIN_SUB_TUTORIAL pick1
//   // Create a vector of grasps to be attempted, currently only creating single grasp.
//   // This is essentially useful when using a grasp generator to generate and test multiple grasps.
//   std::vector<moveit_msgs::Grasp> grasps;
//   grasps.resize(1);

//   // Setting grasp pose
//   // ++++++++++++++++++++++
//   // This is the pose of panda_link8. |br|
//   // Make sure that when you set the grasp_pose, you are setting it to be the pose of the last link in
//   // your manipulator which in this case would be `"panda_link8"` You will have to compensate for the
//   // transform from `"panda_link8"` to the palm of the end effector.
//   grasps[0].grasp_pose.header.frame_id = "panda_link0";
//   tf2::Quaternion orientation;
//   orientation.setRPY(3.1079, 0.0001, -0.7855);
//   grasps[0].grasp_pose.pose.orientation = tf2::toMsg(orientation);
//   grasps[0].grasp_pose.pose.position.x = -0.3959;
//   grasps[0].grasp_pose.pose.position.y = 0.43945;
//   grasps[0].grasp_pose.pose.position.z = 0.63966;

//   // Setting pre-grasp approach
//   // ++++++++++++++++++++++++++
//   /* Defined with respect to frame_id */
//   grasps[0].pre_grasp_approach.direction.header.frame_id = "panda_link0";
//   /* Direction is set as positive x axis */
//   grasps[0].pre_grasp_approach.direction.vector.x = 1.0;
//   grasps[0].pre_grasp_approach.min_distance = 0.095;
//   grasps[0].pre_grasp_approach.desired_distance = 0.115;

//   // Setting post-grasp retreat
//   // ++++++++++++++++++++++++++
//   /* Defined with respect to frame_id */
//   grasps[0].post_grasp_retreat.direction.header.frame_id = "panda_link0";
//   /* Direction is set as positive z axis */
//   grasps[0].post_grasp_retreat.direction.vector.z = 1.0;
//   grasps[0].post_grasp_retreat.min_distance = 0.1;
//   grasps[0].post_grasp_retreat.desired_distance = 0.25;

//   // Setting posture of eef before grasp
//   // +++++++++++++++++++++++++++++++++++
//   openGripper(grasps[0].pre_grasp_posture);
//   // END_SUB_TUTORIAL

//   // BEGIN_SUB_TUTORIAL pick2
//   // Setting posture of eef during grasp
//   // +++++++++++++++++++++++++++++++++++
//   closedGripper(grasps[0].grasp_posture);
//   // END_SUB_TUTORIAL

//   // BEGIN_SUB_TUTORIAL pick3
//   // Set support surface as table1.
//   move_group.setSupportSurfaceName("table1");
//   // Call pick to pick up the object using the grasps given
//   move_group.pick("cylinder", grasps);
//   // END_SUB_TUTORIAL
// }

int main(int argc, char** argv)
{
  // Initialize ROS
  ros::init(argc, argv, "cylinder_segment");

  // Start the segmentor
  CylinderSegment segmentor;

  moveit::planning_interface::MoveGroupInterface group("panda_arm");
  // group.setPlanningTime(45.0);
  ros::WallDuration(1.0).sleep();
  // pick(group);


  // Spin
  ros::spin();
}
