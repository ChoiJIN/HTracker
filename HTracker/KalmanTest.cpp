#include "stdafx.h"
#include "KalmanTest.h"


KalmanTest::KalmanTest(void)
{
}


KalmanTest::~KalmanTest(void)
{
}

void KalmanTest::init()
{ // Should be modified.
	KF = KalmanFilter(4, 2, 0);//����4, ����2
	state = Mat_<float>(4, 1); //���°� (x, y, Vx, Vy) 
	processNoise = Mat(4, 1, CV_32F);//������ ���°�4
	measurement = Mat_<float>(2,1); //����2

	measurement.setTo(Scalar(0));

	KF.statePre.at<float>(0) = 0;
	KF.statePre.at<float>(1) = 0;
	KF.statePre.at<float>(2) = 0;
	KF.statePre.at<float>(3) = 0;
	
	//���� ���
	KF.transitionMatrix = *(Mat_<float>(4, 4) << 1,0,1,0,   0,1,0,1,  0,0,1,0,  0,0,0,1);
	//������ķ� �ʱ�ȭ

	//�������
	setIdentity(KF.measurementMatrix);
	//���μ��� ���� ���л�, Ŭ���� �������� ���� ����
	setIdentity(KF.processNoiseCov, Scalar::all(1e-4));//1e-4, ������� �ӵ�,Ŭ���� ����
	//���� ���� ���л�, �������� �������� ���� ����
	setIdentity(KF.measurementNoiseCov, Scalar::all(1e-1 ));//1e-1, ������� �ӵ�, �������� ����
	//���� ���� ���л�
	setIdentity(KF.errorCovPost, Scalar::all( .1 ) );//.1


	randn(KF.statePost, Scalar::all(0), Scalar::all(0.1));
}

void KalmanTest::init( double pnc, double mnc )
{
	KF = KalmanFilter(4, 2, 0);//����4, ����2
	state = Mat_<float>(4, 1); //���°� (x, y, Vx, Vy) 
	processNoise = Mat(4, 1, CV_32F);//������ ���°�4
	measurement = Mat_<float>(2,1); //����2

	measurement.setTo(Scalar(0));

	KF.statePre.at<float>(0) = 0;
	KF.statePre.at<float>(1) = 0;
	KF.statePre.at<float>(2) = 0;
	KF.statePre.at<float>(3) = 0;

	//���� ���
	KF.transitionMatrix = *(Mat_<float>(4, 4) << 1,0,1,0,   0,1,0,1,  0,0,1,0,  0,0,0,1);
	//������ķ� �ʱ�ȭ

	//�������
	setIdentity(KF.measurementMatrix);
	//���μ��� ���� ���л�, Ŭ���� �������� ���� ����
	setIdentity(KF.processNoiseCov, Scalar::all(pnc));//1e-4, ������� �ӵ�,Ŭ���� ����
	//���� ���� ���л�, �������� �������� ���� ����
	setIdentity(KF.measurementNoiseCov, Scalar::all(mnc));//1e-1, ������� �ӵ�, �������� ����
	//���� ���� ���л�
	setIdentity(KF.errorCovPost, Scalar::all( .1 ) );//.1


	randn(KF.statePost, Scalar::all(0), Scalar::all(0.1));
}

void KalmanTest::setStartPoint( Point p )
{
	KF.statePre.at<float>(0) = p.x;
	KF.statePre.at<float>(1) = p.y;
	correction(p);
}

void KalmanTest::predict()
{
	prediction = KF.predict();//���� : ���� �ð��ܰ迡 ���� ����
	predictPt = Point(prediction.at<float>(0), prediction.at<float>(1));
	
	//cout<<"������: "<<predictPt<<endl;
}

void KalmanTest::correction(cv::Point actual)
{
	measurement(0) = actual.x;
	measurement(1) = actual.y;
	measPt = Point(measurement(0),measurement(1));

	estimated = KF.correct(measurement);
	statePt = Point(estimated.at<float>(0), estimated.at<float>(1));
}

void KalmanTest::correction(cv::Rect actual)
{
	measurement(0) = actual.x;
	measurement(1) = actual.y;
	measPt = Point(measurement(0),measurement(1));
	this->width = actual.width;
	this->height = actual.height;

	estimated = KF.correct(measurement);
	statePt = Point(estimated.at<float>(0), estimated.at<float>(1));
}

void KalmanTest::draw( Mat image )
{
	rectangle(image, predictPt, predictPt+Point(width, height), Scalar(255, 0, 0), 3);	// ����� ��� ����� ��� �簢�� �׸�
}
