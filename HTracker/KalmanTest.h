#pragma once
#include "stdafx.h"

class KalmanTest
{
public:
	KalmanTest(void);
	void init();
	void init(double, double);	// Process Noise Cov, Measurement Noise Cov
	void setStartPoint(Point p);
	void predict();
	void correction(cv::Point);
	void correction(cv::Rect);
	void draw(Mat image);

	~KalmanTest(void);

public:
	KalmanFilter KF;				//����4, ����2
	cv::Mat_<float> state;			//���°� (x, y, Vx, Vy) 
	cv::Mat processNoise;			//������ ���°�4
	cv::Mat_<float> measurement;	//����2
	cv::Mat prediction;
	cv::Mat estimated;
	cv::Point predictPt,
		measPt,
		statePt;
	int width, height;				//size of window(detected)

};

