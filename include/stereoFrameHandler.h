/**
* This file is part of GF-PL-SLAM.
*
* Copyright (C) 2019 Yipu Zhao <yipu dot zhao at gatech dot edu>
* (Georgia Institute of Technology)
* For more information see
* <https://sites.google.com/site/zhaoyipu/good-feature-visual-slam>
*
* GF-PL-SLAM is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* GF-PL-SLAM is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with GF-PL-SLAM. If not, see <http://www.gnu.org/licenses/>.
*/

/*****************************************************************************
**   PL-SLAM: stereo visual SLAM with points and line segment features  	**
******************************************************************************
**																			**
**	Copyright(c) 2017, Ruben Gomez-Ojeda, University of Malaga              **
**	Copyright(c) 2017, MAPIR group, University of Malaga					**
**																			**
**  This program is free software: you can redistribute it and/or modify	**
**  it under the terms of the GNU General Public License (version 3) as		**
**	published by the Free Software Foundation.								**
**																			**
**  This program is distributed in the hope that it will be useful, but		**
**	WITHOUT ANY WARRANTY; without even the implied warranty of				**
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the			**
**  GNU General Public License for more details.							**
**																			**
**  You should have received a copy of the GNU General Public License		**
**  along with this program.  If not, see <http://www.gnu.org/licenses/>.	**
**																			**
*****************************************************************************/

#pragma once
#include <stereoFrame.h>
#include <stereoFeatures.h>

typedef Matrix<double,6,6> Matrix6d;
typedef Matrix<double,6,1> Vector6d;


// enable visualization modules of PL-SLAM
#define DO_VIZ

// apply box filter to input image, to simulate motion blur
// #define SIMU_MOTION_BLUR


class StereoFrame;

namespace StVO {

class StereoFrameHandler
{

public:

    EIGEN_MAKE_ALIGNED_OPERATOR_NEW;

    StereoFrameHandler( PinholeStereoCamera* cam_ );
    ~StereoFrameHandler();

    void initialize( const Mat & img_l_, const Mat & img_r_,
                     const int idx_, const double time_stamp_);
    void updateFrame();
    void updateFrame_ECCV18( const Matrix4d T_base );
    void insertStereoPair(const Mat & img_l_, const Mat & img_r_,
                          const int idx_, const double time_stamp_);
    //
    void crossFrameMatching_Brute();
    void crossFrameMatching_Hybrid();
    void crossFrameMatching_Proj();
    //
    void optimizePose();
    void optimizePose(Matrix4d DT_ini);
    double lineSegmentOverlap( Vector2d spl_obs, Vector2d epl_obs,
                               Vector2d spl_proj, Vector2d epl_proj  );

    //
    /* parameter setup / update */
    void setLeftCamExtrinCalib(const double cam_rx,
                               const double cam_ry,
                               const double cam_rz,
                               const double cam_tx,
                               const double cam_ty,
                               const double cam_tz);
    void predictFramePose();
    void setFramePose(const cv::Matx44f Tfw_curr);
    //
    /* line uncertainty modeling */
    void estimateProjUncertainty_greedy();
    void estimateProjUncertainty_descent(const double stepCutRatio,
                                         const double rngCutRatio[2]);
    void estimateProjUncertainty_submodular(const double stepCutRatio,
                                            const double rngCutRatio[2]);
    //
    void getPoseCovMatrixOnLine(const int fst_idx,
                                const int lst_idx,
                                const Matrix<double, 6, 1> rigframe_T_world,
                                const list<StVO::LineFeature*>::iterator it,
                                Matrix<double, 6, 6> & cov_pose_inv);

    void getPoseCovOnLine(const Matrix4d DT_inv,
                          const Vector2d J_loss,
                          const StVO::LineFeature * line,
                          const double cutRatio[2],
                          Matrix<double, 6, 6> & cov_pose);

    void getPoseInfoOnLine(const Matrix4d DT_inv,
                           const Vector2d J_loss,
                           const StVO::LineFeature * line,
                           const double cutRatio[2],
                           Matrix<double, 6, 6> & info_pose);

    void getPoseInfoPoint(const Matrix4d DT_inv,
                          const StVO::PointFeature * point,
                          Matrix<double, 6, 6> & info_pose);

    void updateEndPointByRatio(StVO::LineFeature * line);

    void greedySolveLineCut_P3L(const Matrix4d DT_inv,
                                const double stepCutRatio,
                                const double rngCutRatio[2],
    vector<StVO::LineFeature *> line_P3L);

    //
    /* viz functions */
    void plotLine3D( bool save_screen_shot );
    void plotMatchedLines( bool vizUncertainty, bool save_screen_shot );
    void plotMatchedPointLine(bool save_screen_shot);
    void appendPlotCutLine(bool save_screen_shot);
    void appendPlotOptLine(bool save_screen_shot);


    cv::viz::Viz3d * viz_3D_lines;
    cv::Mat canvas_match_frames;

    // adaptative fast
    int orb_fast_th;

    // slam-specific functions
    bool needNewKF();
    void currFrameIsKF();

    /* variables used in cross-frame line matching */
    list<PointFeature*> matched_pt;
    list<LineFeature*>  matched_ls;

    StereoFrame* prev_frame;
    StereoFrame* curr_frame;
    PinholeStereoCamera* cam;

    ORB_SLAM2::ORBextractor * mpORBextractor_l;
    ORB_SLAM2::ORBextractor * mpORBextractor_r;

    Matrix<double, 6, 1> left_cam_T_rigframe;
    double NUM_PTS_SAMPLE = 50.0; // 20.0;

    int  n_inliers, n_inliers_pt, n_inliers_ls;

    int numFrameLoss = 0;
    int numFrameSinceKeyframe = 0;

    // slam-specific variables
    bool     prev_f_iskf;
    double   entropy_first_prevKF;
    Matrix4d T_prevKF;
    Matrix6d cov_prevKF_currF;

    //
    /* variables used in debug and figure generation */
    bool save_screen_shot = true; // false; //
    bool print_frame_info = false; // true; //

    vector< std::pair<double, Matrix4d> > T_allF;

private:

    void removeOutliers( Matrix4d DT );
    void gaussNewtonOptimization(Matrix4d &DT, Matrix6d &DT_cov, double &err_, int max_iters);
    void optimizeFunctions(Matrix4d DT, Matrix6d &H, Vector6d &g, double &e);

    Vector6d prior_inc;
    Matrix6d prior_cov;
    std::vector<Vector2d> line_projection_end;
    std::vector<Vector2d> line_projection_start;

};

}
