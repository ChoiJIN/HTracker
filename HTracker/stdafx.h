// stdafx.h : ���� ��������� ���� ��������� �ʴ�
// ǥ�� �ý��� ���� ���� �� ������Ʈ ���� ���� ������
// ��� �ִ� ���� �����Դϴ�.
//

#pragma once

#include "targetver.h"

#include <stdio.h>
#include <tchar.h>
#include <vector>
#include <algorithm>
#include <cmath>
#include <ctime>
#include <sstream>
#include <fstream>
#include <windows.h>


// TODO: ���α׷��� �ʿ��� �߰� ����� ���⿡�� �����մϴ�.
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>W
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/nonfree/features2d.hpp>
#include <opencv2/gpu/gpu.hpp>
#include <opencv2/video/tracking.hpp>
#include <opencv2/ocl/ocl.hpp>

using namespace std;
using namespace cv;

#define WINDOW_WIDTH 360	//360 768
#define WINDOW_HEIGHT 288	//288 432

//#define IMWRITE
//#define WRITE_DETECTED
//#define SEQUENCE
//#define HEAD
//#define MYDETECT
//#define VIDEOWRITER