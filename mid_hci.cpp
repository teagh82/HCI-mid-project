#include <opencv2/opencv.hpp>
#include <iostream>

using namespace cv;
using namespace std;

// global 변수로 선언
Mat image, flower;
Mat roi;
Mat clone;	// 복사본
int mx1, my1, mx2, my2;	// 마우스로 지정한 사각형의 좌표
bool cropping = false;	// 사각형 선택 중임을 나타내는 플래그 변수
bool check = false;
int px[4],py[4];	// 마우스로 지정한 네 점의 좌표
bool pp = false;	// 2번 실행 위한 변수

// 1번 양선형 보간법 위한 함수들
// Linear Interpolation
float Lerp(float s, float e, float t) {
	return s + (e - s) * t;
}

// BiLinear Interpolation
float Blerp(float c00, float c10, float c01, float c11, float tx, float ty) {
	return Lerp(Lerp(c00, c10, tx), Lerp(c01, c11, tx), ty);
}

// 새로운 컬러 화소값 계산
float GetPixel(Mat img, int x, int y, int ch)
{
	if (x > 0 && y > 0 && x < img.cols && y < img.rows)
		return (float)(img.at<Vec3b>(y, x)[ch]);
	else
		return 0.0;
}

Mat n1() {
	Mat dst1 = clone.clone();
	roi = clone(Rect(mx1, my1, mx2 - mx1, my2 - my1));
	printf("1번 좌표 (%d,%d) (%d,%d)\n", mx1,my1,mx2,my2);
	Mat roi1 = Mat::zeros(Size(roi.cols * 2, roi.rows * 2), roi.type());

	float c00[3], c10[3], c01[3], c11[3];
	int value[3];

	for (int y = 0; y < roi1.rows; y++)
		for (int x = 0; x < roi1.cols; x++) {
			for (int ch = 0; ch < 3; ch++) {	// 컬러 이미지이므로 채널 3개 적용
				float gx = ((float)x) / 2.0;	//주변 화소 인덱스 계산
				float gy = ((float)y) / 2.0;
				int gxi = (int)gx;
				int gyi = (int)gy;

				c00[ch] = GetPixel(roi, gxi, gyi, ch);  // 주변 화소값 대입
				c10[ch] = GetPixel(roi, gxi + 1, gyi, ch);
				c01[ch] = GetPixel(roi, gxi, gyi + 1, ch);
				c11[ch] = GetPixel(roi, gxi + 1, gyi + 1, ch);

				value[ch] = (int)Blerp(c00[ch], c10[ch], c01[ch], c11[ch], gx - gxi, gy - gyi);

				roi1.at<Vec3b>(y, x)[ch] = value[ch];
			}
		}

	// 예외 처리
	int rx = mx1 - roi.cols / 2;
	int ry = my1 - roi.rows / 2;
	int rw = roi1.cols;
	int rh = roi1.rows;
	
	int tmp = rx + roi1.cols;
	if (tmp > image.cols) {
		rx = rx - (tmp - image.cols);
		cout << "사각형 영역 잘못 지정(오른쪽으로 너무 붙음)\n";
	}

	if ((mx1 - roi.cols / 2) < 0) {
		rx = 0;
		cout << "사각형 영역 잘못 지정 (왼쪽으로 너무 붙음)\n";
	}

	int tmp2 = ry + roi1.rows;
	if (tmp2 > image.rows) {
		ry = ry - (tmp2 - image.rows);
		cout << "사각형 영역 잘못 지정(아래로 너무 붙음)\n";
	}

	if ((my1 - roi.rows / 2) < 0) {
		ry = 0;
		cout << "사각형 영역 잘못 지정 (위로 너무 붙음)\n";
	}
	
	Mat roi2(dst1, Rect(rx, ry, rw, rh));
	//Mat roi2(dst1, Rect(mx1 - roi.cols/2, my1 - roi.rows/2, roi1.cols, roi1.rows));
	roi1.copyTo(roi2);

	return dst1;
}

void n2() {
	Point2f inputp[4];
	inputp[0] = Point2f(px[0], py[0]);	// 왼쪽 위
	inputp[1] = Point2f(px[1], py[1]);	// 왼쪽 밑
	inputp[2] = Point2f(px[3], py[3]);	// 오른쪽 위
	inputp[3] = Point2f(px[2], py[2]);	// 오른쪽 밑

	Point2f outputp[4];
	outputp[0] = Point2f(0, 0);						// 왼쪽 위
	outputp[1] = Point2f(0, image.rows);			// 왼쪽 밑
	outputp[2] = Point2f(image.cols, 0);			// 오른쪽 위
	outputp[3] = Point2f(image.cols, image.rows);	// 오른쪽 밑

	Mat h = getPerspectiveTransform(inputp, outputp); // Perspective 행렬로 변환
	Mat out;
	warpPerspective(clone, out, h, image.size());

	imshow("Perspective Transformation", out);		// 이미지 출력
}

Mat n3(Mat img) {
	resize(img, img, Size(), 0.5, 0.5);
	Size dsize = Size(img.cols, img.rows);

	Point center = Point(img.cols / 2.0, img.rows / 2.0);	//회전 중심
	Mat M = getRotationMatrix2D(center, 45, 1.0);	//회전 행렬 계산	
	warpAffine(img, img, M, dsize, INTER_LINEAR);	//회전 실행

	return img;
}

void n4() {
	int rows = image.rows;
	int cols = image.cols;
	Mat dst_warping1 = clone.clone();
	Mat dst_warping2 = clone.clone();
	Mat dst_warping3 = clone.clone();

	for (int i = 0; i < rows; i++) {  // y 방향
		for (int j = 0; j < cols; j++) { // x 방향
			int offset_x = 0;
			int offset_y = (int)(25.0 * sin(2 * 3.14 * j / image.rows));  // sine wave
			dst_warping1.at<Vec3b>(i, j) = image.at<Vec3b>((i + offset_y) % rows, j);
		}
	}

	for (int i = 0; i < rows; i++) {  // y 방향
		for (int j = 0; j < cols; j++) { // x 방향
			int offset_x = 0;
			int offset_y = (int)(25.0 * sin(2 * 3.14 * j / 90));  // sine wave
			dst_warping2.at<Vec3b>(i, j) = image.at<Vec3b>((i + offset_y) % rows, j);
		}
	}

	for (int i = 0; i < rows; i++) {  // y 방향
		for (int j = 0; j < cols; j++) { // x 방향
			int offset_x = 0;
			int offset_y = (int)(50.0 * sin(2 * 3.14 * j / 90));  // sine wave
			dst_warping3.at<Vec3b>(i, j) = image.at<Vec3b>((i + offset_y) % rows, j);
		}
	}

	imshow("Warping1", dst_warping1);
	imshow("Warping2", dst_warping2);
	imshow("Warping3", dst_warping3);
}

void n5() {
	int rows = image.rows;
	int cols = image.cols;
	Mat warping_ad = clone.clone();

	for (int i = 0; i < rows; i++) {  // y 방향
		for (int j = 0; j < cols; j++) { // x 방향
			int offset_x = 0;
			int offset_y = (int)(25.0 * sin(2 * 3.14 * (j+i) / 90));  // sine wave
			warping_ad.at<Vec3b>(i, j) = image.at<Vec3b>((i + offset_y) % rows, j);
		}
	}

	imshow("Warping Advanced", warping_ad);
}

void n6() {
	Mat dst6, mask = image.clone();

	GaussianBlur(clone, dst6, Size(15, 15), 0, 0);

	// 검은 원이 그려지지 않은 부분을 white로 만들어줌
	for (int r = 0; r < image.rows; r++) {
		for (int c = 0; c < image.cols; c++) {
			for (int ch = 0; ch < 3; ch++) {
				if (image.at<Vec3b>(r, c)[ch] != 0)
					mask.at<Vec3b>(r, c)[ch] = 255;
				else
					mask.at<Vec3b>(r, c)[ch] = 0;
			}
		}
	}
	bitwise_and(dst6, mask, dst6);	// white인 부분을 블러 처리된 사진으로 해줌

	// 블러 처리 안된 검은 원 부분을 원본 사진으로
	for (int r = 0; r < image.rows; r++) {
		for (int c = 0; c < image.cols; c++) {
			for (int ch = 0; ch < 3; ch++) {
				if (dst6.at<Vec3b>(r, c)[ch] == 0)
					dst6.at<Vec3b>(r, c)[ch] = clone.at<Vec3b>(r, c)[ch];
			}
		}
	}
	imshow("Gaussian filter", dst6);
}

void n7() {
	vector<Mat> bgr_planes;	// 영상(Mat)들의 벡터 bgr_planes[0], [1], [2]
	split(flower, bgr_planes);	// 입력 영상을 색상별로 분리한다. 

	int histSize = 256;	// 히스토그램 막대 그래프의 개수
	float range[] = { 0, 256 }; 	// 화소값 범위
	const float* histRange = { range };
	bool uniform = true, accumulate = false;

	// 영상에서 각 화소 값의 출현 개수를 구한다.
	Mat b_hist, g_hist, r_hist;
	calcHist(&bgr_planes[0], 1, 0, Mat(), b_hist, 1, &histSize, &histRange, uniform, accumulate);
	calcHist(&bgr_planes[1], 1, 0, Mat(), g_hist, 1, &histSize, &histRange, uniform, accumulate);
	calcHist(&bgr_planes[2], 1, 0, Mat(), r_hist, 1, &histSize, &histRange, uniform, accumulate);

	// 막대 그래프가 그려지는 영상을 생성한다. 
	int hist_w = 512, hist_h = 400;
	int bin_w = cvRound((double)hist_w / histSize); // 상자의 폭 512/256=2
	Mat histImage(hist_h, hist_w, CV_8UC3, Scalar(0, 0, 0)); // black

	// 값들이 영상을 벗어나지 않도록 정규화한다. 
	normalize(b_hist, b_hist, 0, histImage.rows, NORM_MINMAX, -1, Mat());
	normalize(g_hist, g_hist, 0, histImage.rows, NORM_MINMAX, -1, Mat());
	normalize(r_hist, r_hist, 0, histImage.rows, NORM_MINMAX, -1, Mat());

	// 히스토그램의 값을 막대로 그린다. 
	for (int i = 0; i < 255; i++) {
		line(histImage, Point(bin_w * (i), hist_h),
			Point(bin_w * (i), hist_h - b_hist.at<float>(i)),
			Scalar(255, 0, 0));  // blue
		line(histImage, Point(bin_w * (i), hist_h),
			Point(bin_w * (i), hist_h - g_hist.at<float>(i)),
			Scalar(0, 255, 0));  // green
		line(histImage, Point(bin_w * (i), hist_h),
			Point(bin_w * (i), hist_h - r_hist.at<float>(i)),
			Scalar(0, 0, 255));  // red
	}
	imshow("flower", flower);
	imshow("histogram", histImage);

	Mat threshold_image;
	cvtColor(flower, flower, cv::COLOR_BGR2GRAY);	// 전경과 배경 분리를 위해 흑백으로 만들어줌
	threshold(flower, threshold_image, 150, 255, THRESH_BINARY);
	cvtColor(threshold_image, threshold_image, cv::COLOR_GRAY2BGR);		// 흑백으로 만들었던 것을 다시 컬러로 만들어줌 

	Mat dst7 = clone.clone();
	bitwise_and(clone, threshold_image, dst7);      // (source, mask, target)
	
	imshow("Segmentation", threshold_image);
	imshow("Segmentation MyPhoto", dst7);
}

int i = 0;	// 2번 마우스 클릭으로 그려진 사각형 표시 위한 변수
// 마우스 이벤트가 발생하면 호출되는 콜백 함수 
void onMouse(int event, int x, int y, int flags, void* param) {
	// 1번 마우스 입력
	if (event == EVENT_LBUTTONDOWN) {	 // 마우스의 왼쪽 버튼을 누르면
		mx1 = x; 			 // 사각형의 좌측 상단 좌표 저장
		my1 = y;
		cropping = true;
	}
	else if (event == EVENT_LBUTTONUP) { // 마우스의 왼쪽 버튼에서 손을 떼면
		mx2 = x; 			  // 사각형의 우측 하단 좌표 저장
		my2 = y;
		cropping = false;
		rectangle(image, Rect(mx1, my1, mx2 - mx1, my2 - my1), Scalar(0, 255, 255), 1.5);
		imshow("MyPhoto", image);		// 이미지 출력
		check = true;
	}

	// 2번 마우스 입력
	if (pp) {
		if (event == EVENT_LBUTTONDOWN) {	 // 마우스의 왼쪽 버튼을 누르면
			px[i] = x; 			 
			py[i] = y;
			cout << "("<< px[i] << ", " << py[i] << ")\n";
			if (i > 0)
				line(image, Point(px[i - 1], py[i - 1]), Point(px[i], py[i]), Scalar(255, 0, 0), 1.5);
			if (i == 3) {
				line(image, Point(px[0], py[0]), Point(px[i], py[i]), Scalar(255, 0, 0), 1.5);
				n2();
			}
			imshow("MyPhoto", image);
			i++;
		}
	}

	// 6번 마우스 입력
	if (event == EVENT_RBUTTONDOWN) {
		mx1 = x;
		my1 = y;
		cropping = true;
	}
	else if (event == EVENT_RBUTTONUP) {
		mx2 = x;
		my2 = y;
		cropping = false;
		circle(image, Point(mx1, my1), mx2 - mx1, Scalar(0, 0, 0), -1);
		imshow("MyPhoto", image);		// 이미지 출력
	}
}

int main() {				
	image = imread("kihyeon.jpg", IMREAD_COLOR);	// 이미지를 읽음
	if (image.empty()) {
		cout << "kihyeon 영상을 읽을 수 없음" << endl;
		return -1;
	}
	
	resize(image, image, Size(400, 400));
	clone = image.clone();	// 복사본을 만들어둠
	imshow("MyPhoto", image);
	
	flower = imread("flower.png", IMREAD_COLOR);	// 이미지를 읽음
	if (flower.empty()) {
		cout << "flower 영상을 읽을 수 없음" << endl;
		return -1;
	}
	resize(flower, flower, Size(400, 400));

	setMouseCallback("MyPhoto", onMouse, &image);	// 콜백 함수 등록

	while (1) {
		int key = waitKeyEx();	// 사용자로부터 키를 기다림

		if (key == 'q') break;	// 사용자가 ‘q'를 누르면 종료
		else if (key == 'i') {	// 1번
			int key = waitKey(100);
			if (check) {	  // 왼쪽 마우스를 UP하면 잘라낸 이미지 보여줌
				imshow("Bilinear Interpolation", n1());		// 이미지 출력
				check = false;
			}
		}
		else if (key == 'p') {	// 2번
			cout << "2번 좌표\n";
			pp = true;
		}
		else if (key == 's') {	// 3번
			Mat dst3 = clone.clone();
			dst3 = n3(dst3);
			imshow("Scale & Rotation", dst3);
		}
		else if (key == 'w') {	// 4번
			n4();
		}
		else if (key == 'a') {	// 5번
			n5();
		}
		else if (key == 'b') {	// 6번
			n6();
		}
		else if (key == 'g') {	// 7번
			n7();
		}
	}

	waitKey(0);						// 키보드 입력을 기다림
	return 0;
}