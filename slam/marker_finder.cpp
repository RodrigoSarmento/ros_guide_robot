/* 
 *  Software License Agreement (BSD License)
 *
 *  Copyright (c) 2016, Natalnet Laboratory for Perceptual Robotics
 *  All rights reserved.
 *  Redistribution and use in source and binary forms, with or without modification, are permitted provided
 *  that the following conditions are met:
 *
 *  1. Redistributions of source code must retain the above copyright notice, this list of conditions and
 *     the following disclaimer.
 *
 *  2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and
 *     the following disclaimer in the documentation and/or other materials provided with the distribution.
 * 
 *  3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or
 *     promote products derived from this software without specific prior written permission.
 * 
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, *  INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 *  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 *  WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 *  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include <opencv2/core/core.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include "marker_finder.h"
#include <math.h>

using namespace std;
using namespace cv;
using namespace aruco;

/**
 * Finds Marker pose local and add to the list of marker_poses_local
 */
void MarkerFinder::setMarkerPosesLocal(float aruco_minimum_distance)
{// This function finds ther marker pose local(camera related)
	marker_poses_.clear();
	double x=0,y=0,z=0;
	for(size_t i = 0; i < markers_.size(); i++)
	{
		Mat R = Mat::eye(3, 3, CV_32FC1); // Rotation Matrix
		Eigen::Affine3f P = Eigen::Affine3f::Identity(); // Aruco Pose

		Rodrigues(markers_[i].Rvec, R);
	
		P(0,0) = R.at<float>(0,0); P(0,1) = R.at<float>(0,1); P(0,2) = R.at<float>(0,2);
		P(1,0) = R.at<float>(1,0); P(1,1) = R.at<float>(1,1); P(1,2) = R.at<float>(1,2);
		P(2,0) = R.at<float>(2,0); P(2,1) = R.at<float>(2,1); P(2,2) = R.at<float>(2,2);
		P(0,3) = markers_[i].Tvec.at<float>(0,0); P(1,3) = markers_[i].Tvec.at<float>(1,0); P(2,3) = markers_[i].Tvec.at<float>(2,0);

		x = pow(P(0,3),2);
		y = pow(P(1,3),2);
		z = pow(P(2,3),2);

		///getting the absolute distance between camera and marker
		///if their distance is closer then aruco_minimum_distance save marker pose
		if(aruco_minimum_distance == -1){//infinite
			marker_poses_local_.push_back(P);
			continue ;
		}
		if(sqrt(x + y + z) < aruco_minimum_distance){//aruco is closer than the minimum distance
			marker_poses_local_.push_back(P);  //Find the pose point 3d Global ref frame
			continue;
		}
		if(sqrt(x +y +z) >= aruco_minimum_distance){//aruco is further than the minimum distance
			continue;
		}	}
}

/**
 * Finds markers and sets its local poses in a list of marker_poses_global
 * @Params Affine3f camera_pose
 */
void MarkerFinder::setMarkerPosesGlobal(Eigen::Affine3f cam_pose, float aruco_minimum_distance)
{// This funcion finds the marker pose global(map ref frame)
	double x=0,y=0,z=0;

	marker_poses_.clear();
	for(size_t i = 0; i < markers_.size(); i++)
	{
		Mat R = Mat::eye(3, 3, CV_32FC1); // Rotation Matrix
		Eigen::Affine3f P = Eigen::Affine3f::Identity(); // Aruco Pose
		
		Rodrigues(markers_[i].Rvec, R);
	
		P(0,0) = R.at<float>(0,0); P(0,1) = R.at<float>(0,1); P(0,2) = R.at<float>(0,2);
		P(1,0) = R.at<float>(1,0); P(1,1) = R.at<float>(1,1); P(1,2) = R.at<float>(1,2);
		P(2,0) = R.at<float>(2,0); P(2,1) = R.at<float>(2,1); P(2,2) = R.at<float>(2,2);
		P(0,3) = markers_[i].Tvec.at<float>(0,0); P(1,3) = markers_[i].Tvec.at<float>(1,0); P(2,3) = markers_[i].Tvec.at<float>(2,0);
		marker_poses_.push_back(cam_pose.inverse() * P);

		x = pow(P(0,3),2);
		y = pow(P(1,3),2);
		z = pow(P(2,3),2);

		///getting the absolute distance between camera and marker
		///if their distance is closer then aruco_minimum_distance save marker pose
		if(aruco_minimum_distance == -1){//infinite
			marker_poses_.push_back(cam_pose.inverse() *P );
			continue ;
		}
		if(sqrt(x + y + z) < aruco_minimum_distance){//aruco is closer than the minimum distance
			marker_poses_.push_back(cam_pose.inverse() * P);  //Find the pose point 3d Global ref frame
			continue;
		}
		if(sqrt(x +y +z) >= aruco_minimum_distance){//aruco is further than the minimum distance
			continue;
		}
	}
}

/**
 * Set marker point pose global related to a marker position
 * @Params Affine3f camera pose; float aruco distance
 */
void MarkerFinder::setMarkerPointPosesGlobal(Eigen::Affine3f cam_pose, float aruco_minimum_distance)
{/* This function save the marker pose where the robot need to go.
 It's the same aruco pose but with a value added in order to the robot always find a place inside of the map
 In Some situations the aruco marker can be detected outside of the map, since it is oftenly
 placed in a wall(Precision erros can place the aruco marker outside of the map)
 */  
	marker_point_poses_.clear();
	for(size_t i = 0; i < markers_.size(); i++)
	{
		Mat R = Mat::eye(3, 3, CV_32FC1); // Orientation 
		Eigen::Affine3f P = Eigen::Affine3f::Identity();// Marker pose
		Eigen::Vector4f F = Eigen::Vector4f(); // Distance between aruco and the 3D point we want
		Eigen::Vector4f V = Eigen::Vector4f(); // 3D point pose 
		double x=0,y=0,z=0;
		F(0,0) = 0.0;
		F(1,0) = 0.0;
		F(2,0) = 0.5;
		F(3,0) = 1.0;

		Rodrigues(markers_[i].Rvec, R);
	
		P(0,0) = R.at<float>(0,0); P(0,1) = R.at<float>(0,1); P(0,2) = R.at<float>(0,2);
		P(1,0) = R.at<float>(1,0); P(1,1) = R.at<float>(1,1); P(1,2) = R.at<float>(1,2);
		P(2,0) = R.at<float>(2,0); P(2,1) = R.at<float>(2,1); P(2,2) = R.at<float>(2,2);
		P(0,3) = markers_[i].Tvec.at<float>(0,0); P(1,3) = markers_[i].Tvec.at<float>(1,0); P(2,3) = markers_[i].Tvec.at<float>(2,0);

		x = pow(P(0,3),2);
		y = pow(P(1,3),2);
		z = pow(P(2,3),2);
		
		///getting the absolute distance between camera and marker
		///if their distance is closer then aruco_minimum_distance save marker pose
		if(aruco_minimum_distance == -1){//infinite
			V = P * F; //Find the point in the Aruco ref frame
			marker_point_poses_.push_back(cam_pose.inverse() *V );
			continue ;
		}
		if(sqrt(x + y + z) < aruco_minimum_distance){//aruco is closer than the minimum distance
			V = P * F; //Find the point in the Aruco ref frame
			marker_point_poses_.push_back(cam_pose.inverse() * V);  //Find the pose point 3d Global ref frame
			continue;
		}
		if(sqrt(x +y +z) >= aruco_minimum_distance){//aruco is further than the minimum distance
			continue;
		}
	}
}
/* Arucos dictionary
ARUCO, Original
ARUCO_MIP_25h7,
ARUCO_MIP_16h3,
ARUCO_MIP_36h12, Recommended
ARTAG,
ARTOOLKITPLUS,
ARTOOLKITPLUSBCH,
TAG16h5,TAG25h7,TAG25h9,TAG36h11,TAG36h10 
*/
MarkerFinder::MarkerFinder()
{//set dictionary
	marker_detector_.setDictionary("ARUCO_MIP_36h12", 0);
}

/**
 * Set marker Params 
 * @Params string parameters, float aruco size, string aruco dictionry
 */
void MarkerFinder::markerParam(string params, float size, string aruco_dic)
{//Load params 
	try{
		marker_detector_.setDictionary(aruco_dic,0);
		camera_params_.readFromXMLFile(params);
		marker_size_ = size;
	}
  	catch(char param[]){
    	cout << "An exception occurred. Exception Nr. "  << param<<'\n';
  	}
}

/**
 * Detect Aruco Markers
 * @Params cv::Mat image, Affine3f camera_pose, float aruco_minimum_distance
 */
void MarkerFinder::detectMarkers(const cv::Mat img, Eigen::Affine3f cam_pose, float aruco_minimum_distance)
{//Detect marker and calls setMarkerPointPosesGlobal
	markers_.clear();
	marker_detector_.detect(img, markers_, camera_params_, marker_size_);
	
	setMarkerPointPosesGlobal(cam_pose,aruco_minimum_distance);
}