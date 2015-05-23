//  [8/23/2014 Hajin]

#include "stdafx.h"
#include "HOGtest.h"
#include "FeatureTracker.h"
#include "TrackerGroup.h"
#include "KltTracker.h"

//BRISK *preLoadedBrisk = new BRISK(70, 4, 1.0f);
BRISK *preLoadedBrisk = new BRISK(70, 4, 1.0f);

int count2=0;



void overlayImage(const cv::Mat &background, const cv::Mat &foreground, 
	cv::Mat &output, cv::Point2i location)
{
	background.copyTo(output);


	// start at the row indicated by location, or at row 0 if location.y is negative.
	for(int y = std::max(location.y , 0); y < background.rows; ++y)
	{
		int fY = y - location.y; // because of the translation

		// we are done of we have processed all rows of the foreground image.
		if(fY >= foreground.rows)
			break;

		// start at the column indicated by location, 

		// or at column 0 if location.x is negative.
		for(int x = std::max(location.x, 0); x < background.cols; ++x)
		{
			int fX = x - location.x; // because of the translation.

			// we are done with this row if the column is outside of the foreground image.
			if(fX >= foreground.cols)
				break;

			// determine the opacity of the foregrond pixel, using its fourth (alpha) channel.
			double opacity =
				((double)foreground.data[fY * foreground.step + fX * foreground.channels() + 3])

				/ 255.;


			// and now combine the background and foreground pixel, using the opacity, 

			// but only if opacity > 0.
			for(int c = 0; opacity > 0 && c < output.channels(); ++c)
			{
				unsigned char foregroundPx =
					foreground.data[fY * foreground.step + fX * foreground.channels() + c];
				unsigned char backgroundPx =
					background.data[y * background.step + x * background.channels() + c];
				output.data[y*output.step + output.channels()*x + c] =
					backgroundPx * (1.-opacity) + foregroundPx * opacity;
			}
		}
	}
}
int write_count=0;
void imageWrite(char* dir, Mat img, Size size) {
#ifdef WRITE_DETECTED
// 	double k;
// 	if(img.cols*2>img.rows){
// 		k = (128.0/img.rows);
// 		resize(img, img, Size((int)(img.cols*k), 128));
// 	}
// 	else {
// 		k = (64.0/img.cols);
// 		resize(img, img, Size(64, (int)(img.rows*k)));
// 	}
#endif

	Mat img2;
	if(img.cols >= size.width && img.rows >= size.height)
		img2 = img(Rect(0, 0, size.width, size.height));	// crop

	char filename[30];
	sprintf(filename, "%s/fp11 %d.jpg", dir, write_count);
	imwrite(filename, img2);
	write_count++;
}

void wtoc(char * Dest, const WCHAR * Source)
{
    int i = 0;

    while(Source[i] != '\0')
    {
        Dest[i] = (char)Source[i];
        ++i;
    }

	Dest[i] = '\0';
}
int executeAllFile(TCHAR *c,int argc, vector<string>& fileNames)
{
	WIN32_FIND_DATA ffd;
	HANDLE hFind;

	hFind = FindFirstFile(c, &ffd);
	if (hFind == INVALID_HANDLE_VALUE) {
		printf ("FindFirstFile failed (%d)\n", GetLastError());
		return 1;
	} 

	do {
		if (!(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
			char fname[256];

			wtoc(fname, ffd.cFileName);

			cout<<fname<<endl;
			fileNames.push_back(fname);
		}
	} while (FindNextFile(hFind, &ffd) != 0);

	return 0;
}

void myDetectMultiScale(HOGDescriptor* hog, Mat img, vector<Rect>& found, double hitThreshold,
	int topFoot, int bottomFoot, double ratio, int realHeightAtTop, int acceptableDetla);

int main()
{
	
#ifdef SEQUENCE
	vector<string> filenames;
	executeAllFile( _T(".\\seq\\*"), 0, filenames);
#else
	//VideoCapture cap("D:/ChungAngDrone/dataset/mitsubishi.avi");
	//VideoCapture cap("D:/ChungAngDrone/dataset/campus4-c2.avi");
	VideoCapture cap("D:/ChungAngDrone/dataset/campusmulti.avi");
	//VideoCapture cap("D:/ChungAngDrone/dataset/cau/04.mp4");
	//VideoCapture cap("D:/�߾ӵ����/dataset/cau/media_20141028_144547/frame(%d).jpg");
	if(!cap.isOpened()) { // check if we succeeded
		printf("Error opening video!\n");
		return EXIT_FAILURE;
	}
	const char filename[30] = "result.avi";
#ifdef VIDEOWRITER
	VideoWriter writer;
	writer.open("result.avi", -1, 10, Size(768, 576));
#endif
#endif
	
	char ch=0;
	Mat captured, hogimg, kalmanimg, blurred, grounded;
	Mat map_background = imread("data/map_background3.jpg");
	Mat ground = imread("data/ground.png", IMREAD_UNCHANGED);
	Mat trans = imread("data/transparent2.png", IMREAD_UNCHANGED);
	Mat asdf;

	/************************************************************************/
	/* HOG, Kalman, Matcher init                                            */
	/************************************************************************/
	HOGtest hog;
	hog.init();
	HOGDescriptor hogd;
	vector<Rect> found;
	TrackerGroup trackers;
	trackers.setThreshold();

	/***********************************************************************/
	/* Loop variables                                                      */
	/***********************************************************************/
	bool start_detect=true, start_track=true, start_match=false, start_save=true;
	int frame=0;
	int count=0;
	int detectingRate=8;	// 1 detect per 4 frame
	int matchingCount=0;		// 3 match per 4 frame
	
	/************************************************************************/
	/* Start loop                                                           */
	/************************************************************************/
	char dir[40];
	while(ch != 'c')
	{
		/********************************/
		/* capture from video           */
		/********************************/
		
#ifdef SEQUENCE
		if(count<filenames.size()) {
			sprintf(dir, "seq/%s", filenames[count].c_str());
			captured = imread(dir);
		}
		else
			captured.release();
#else
		cap>>captured;
#endif
		if(captured.empty()) {
			cout<<"no image to be captured"<<endl;
			break;
		}
		
 		
		//resize(captured, captured, Size(640, 360)); // for cau dataset
  		//GaussianBlur( captured, blurred, Size( 7, 7 ), 0, 0 );	// it costs approximately 0.02sec (hajin laptop)

		hogimg = captured.clone();

		/************************************************************************/
		/* 1. Detect                                                            */
		/************************************************************************/
		if(start_detect && count%detectingRate<1) {
			//cout<<"hog"<<endl;
			
#ifdef MYDETECT
			myDetectMultiScale(&hogd, blurred, found, 0.0, 500, 676, 2.3, 70, 0);  
#else
			hog.detect(captured);// hog uses blurred image
#endif // MYDETECT

#ifdef WRITE_DETECTED
			if(!hog.found_filtered.empty()) {
				for(int i=0; i<hog.found_filtered.size(); i++) {
					if(hog.found_filtered[i].x + 32 < hogimg.cols &&
						hog.found_filtered[i].y + 32 < hogimg.rows)
						imageWrite("fp",hogimg(hog.found_filtered[i]), Size(32, 32));
				}
			}
#endif
		}
		hog.show(hogimg);
#ifdef MYDETECT
		for (int i = 0; i < found.size(); i++)
			rectangle(hogimg, found[i], Scalar(0, 255, 0), 2);
#endif // MYDETECT


		//trackers.drawMap(map_background);

		//imshow("trans", trans);
		//overlayImage(hogimg, trans, hogimg, Point(0, 0));

		/************************************************************************/
		/*                                                                      */
		/************************************************************************/
// 		resize(captured, captured, Size(captured.cols*2, captured.rows*2));
// 		for(int i=0; i<hog.found_filtered.size(); i++) {
// 			hog.found_filtered[i] = Rect(hog.found_filtered[i].x*2, hog.found_filtered[i].y*2,hog.found_filtered[i].width*2, hog.found_filtered[i].height*2);
// 		}
		kalmanimg = captured.clone();
		
		/************************************************************************/
		/* 2. Match and Track                                                   */
		/************************************************************************/
 		trackers.match(kalmanimg, hog.found_filtered);
 		trackers.draw(kalmanimg);

		/************************************************************************/
		/* Show                                                                 */
		/************************************************************************/
		imshow("hog", hogimg);
		imshow("kalman", kalmanimg);

		/************************************************************************/
		/* imwrite                                                              */
		/************************************************************************/
#ifdef IMWRITE
		char filename[30];
		sprintf(filename, "frame/frame%d.jpg", count);
		imwrite(filename, hogimg);
#endif
#ifdef VIDEOWRITER
		writer << kalmanimg;
#endif


		/********************************/
		/* Other                        */
		/********************************/
		ch = waitKey(10);
		count++;
		
		/********************************/
		/* Debug                        */
		/********************************/
#if 0
		if(start_detect) cout<<"1true ";
		if(start_match)  cout<<"2true ";
		if(start_track)  cout<<"3true"<<endl;
#elif 0
		if(ch == 'r')			// push 'r' when you want reset the person
			start_save = true;
#endif
	}

#ifdef VIDEOWRITER
	writer.release();
#endif

	return 0;
}

void myDetectMultiScale(HOGDescriptor* hog, Mat img, vector<Rect>& found, double hitThreshold,
	int topFoot, int bottomFoot, double ratio, int realHeightAtTop, int acceptableDetla) {

	// scaling�� image level ���� ���� ��� //

	// 1. nlevel ����
	int nlevels = cvFloor((log(ratio)) / (log(1.05))) + 1;	 // ��� scale�� ����Ұ���
	int yStride = (bottomFoot - topFoot) / nlevels;
	//int nlevels = 15;
	//int yStride = 6;


	// 2. �� level�� scale �Է�
	double scale = 1.;
	int level = 0;
	double factor = 1.05;
	vector<double> levelScale;
	for (level = 0; level < nlevels; level++)
	{
		levelScale.push_back(scale);	// ���⿡�� scale�� �ɷ��� ���� ���..
		scale *= factor;
	}
	level = std::max(level, 1);

	vector<Rect> allCandidates;
	Size winStride = Size(8, 8);
	Size padding = Size(0, 0);
	// 	Mutex mtx;
	// 
	// 	parallel_for_(Range(0, (int)levelScale.size()), 
	// 		HOGInvoker(hog, img, hitThreshold, winStride, padding, &levelScale[0], &allCandidates, &mtx,
	// 		topFoot, bottomFoot, ratio, realHeightAtTop, yStride));

	int i, i1 = 0, i2 = levelScale.size() - 1;

	// Mat�� ���۸� �̸� Ȯ���� �ð��� ���δ�.
	double maxScale = levelScale[i2];	// �ִ� scale factor
	Size maxSz(img.cols, cvCeil(realHeightAtTop*maxScale));	// �ִ� Size
	Mat smallerImgBuf(maxSz, img.type());

	vector<Point> locations;
	vector<double> hitsWeights;

	int presentFoot = topFoot;

	// level�� ���� ��ŭ �ݺ��Ѵ�.
	for (i = i1; i < i2; i++)
	{
		double scale = levelScale[i];
		int presentHeight = cvRound(realHeightAtTop*scale);
		cout << "presentHeight: " << presentHeight << endl;
		double presentFactor = presentHeight / 128.0;
		int presentHead = presentFoot - presentHeight;
		if (presentHead < 0) cout << "Ű�� ������ �ǹ���" << endl;

		Size sz(cvRound(img.cols / presentFactor), cvRound(presentHeight / presentFactor));	// probable size of img
		Mat smallerImg(sz, img.type(), smallerImgBuf.data);			// �Ƹ� �̰͵� ���� Ȯ����

		if (sz == img.size())	// scale factor�� 1 �� ��
			smallerImg = Mat(sz, img.type(), img.data, img.step);	// step�� �̹��� ä��(3-channel��) ����
		else {	// �������� resize
			//rectangle(img, Rect(0, presentHead, img.cols, presentHeight), Scalar((20*i)%255, (50*i)%255, (90*i)%255), 1);
			resize(img(Rect(0, presentHead, img.cols, presentHeight)), smallerImg, sz);
			imshow("before", img(Rect(0, presentHead, img.cols, presentHeight)));
			imshow("after", smallerImg);
			cout << smallerImg.cols << " " << smallerImg.rows << endl;
			waitKey(10);
		}
		// smallerImg�� ������ ����� �κ��̴�!
		
		hog->detect(smallerImg, locations, hitsWeights, hitThreshold, winStride, padding);

		for (int i = 0; i<locations.size(); i++)
		{
			found.push_back(
				Rect(
				Point(locations[i].x*presentFactor, locations[i].y + presentHead),
				Size(64 * presentFactor, 128 * presentFactor)
				)
				);
		}

		groupRectangles(found, 0.0, 0.2);

		presentFoot += yStride;
		if (presentFoot > bottomFoot) presentFoot = bottomFoot;
	}


	// groupRectangles();	// is needed

	// 		for(int j=0; j<temp_found.size(); j++) {
	// 			found.push_back( Rect(temp_found[j], temp_found[j]+Point(personHeight/2 ,personHeight)) );
	// 		}
}