#include <opencv2/opencv.hpp>
#include <iostream>

using namespace cv;
using namespace std;

// global ������ ����
Mat image, flower;
Mat roi;
Mat clone;	// ���纻
int mx1, my1, mx2, my2;	// ���콺�� ������ �簢���� ��ǥ
bool cropping = false;	// �簢�� ���� ������ ��Ÿ���� �÷��� ����
bool check = false;
int px[4],py[4];	// ���콺�� ������ �� ���� ��ǥ
bool pp = false;	// 2�� ���� ���� ����

// 1�� �缱�� ������ ���� �Լ���
// Linear Interpolation
float Lerp(float s, float e, float t) {
	return s + (e - s) * t;
}

// BiLinear Interpolation
float Blerp(float c00, float c10, float c01, float c11, float tx, float ty) {
	return Lerp(Lerp(c00, c10, tx), Lerp(c01, c11, tx), ty);
}

// ���ο� �÷� ȭ�Ұ� ���
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
	printf("1�� ��ǥ (%d,%d) (%d,%d)\n", mx1,my1,mx2,my2);
	Mat roi1 = Mat::zeros(Size(roi.cols * 2, roi.rows * 2), roi.type());

	float c00[3], c10[3], c01[3], c11[3];
	int value[3];

	for (int y = 0; y < roi1.rows; y++)
		for (int x = 0; x < roi1.cols; x++) {
			for (int ch = 0; ch < 3; ch++) {	// �÷� �̹����̹Ƿ� ä�� 3�� ����
				float gx = ((float)x) / 2.0;	//�ֺ� ȭ�� �ε��� ���
				float gy = ((float)y) / 2.0;
				int gxi = (int)gx;
				int gyi = (int)gy;

				c00[ch] = GetPixel(roi, gxi, gyi, ch);  // �ֺ� ȭ�Ұ� ����
				c10[ch] = GetPixel(roi, gxi + 1, gyi, ch);
				c01[ch] = GetPixel(roi, gxi, gyi + 1, ch);
				c11[ch] = GetPixel(roi, gxi + 1, gyi + 1, ch);

				value[ch] = (int)Blerp(c00[ch], c10[ch], c01[ch], c11[ch], gx - gxi, gy - gyi);

				roi1.at<Vec3b>(y, x)[ch] = value[ch];
			}
		}

	// ���� ó��
	int rx = mx1 - roi.cols / 2;
	int ry = my1 - roi.rows / 2;
	int rw = roi1.cols;
	int rh = roi1.rows;
	
	int tmp = rx + roi1.cols;
	if (tmp > image.cols) {
		rx = rx - (tmp - image.cols);
		cout << "�簢�� ���� �߸� ����(���������� �ʹ� ����)\n";
	}

	if ((mx1 - roi.cols / 2) < 0) {
		rx = 0;
		cout << "�簢�� ���� �߸� ���� (�������� �ʹ� ����)\n";
	}

	int tmp2 = ry + roi1.rows;
	if (tmp2 > image.rows) {
		ry = ry - (tmp2 - image.rows);
		cout << "�簢�� ���� �߸� ����(�Ʒ��� �ʹ� ����)\n";
	}

	if ((my1 - roi.rows / 2) < 0) {
		ry = 0;
		cout << "�簢�� ���� �߸� ���� (���� �ʹ� ����)\n";
	}
	
	Mat roi2(dst1, Rect(rx, ry, rw, rh));
	//Mat roi2(dst1, Rect(mx1 - roi.cols/2, my1 - roi.rows/2, roi1.cols, roi1.rows));
	roi1.copyTo(roi2);

	return dst1;
}

void n2() {
	Point2f inputp[4];
	inputp[0] = Point2f(px[0], py[0]);	// ���� ��
	inputp[1] = Point2f(px[1], py[1]);	// ���� ��
	inputp[2] = Point2f(px[3], py[3]);	// ������ ��
	inputp[3] = Point2f(px[2], py[2]);	// ������ ��

	Point2f outputp[4];
	outputp[0] = Point2f(0, 0);						// ���� ��
	outputp[1] = Point2f(0, image.rows);			// ���� ��
	outputp[2] = Point2f(image.cols, 0);			// ������ ��
	outputp[3] = Point2f(image.cols, image.rows);	// ������ ��

	Mat h = getPerspectiveTransform(inputp, outputp); // Perspective ��ķ� ��ȯ
	Mat out;
	warpPerspective(clone, out, h, image.size());

	imshow("Perspective Transformation", out);		// �̹��� ���
}

Mat n3(Mat img) {
	resize(img, img, Size(), 0.5, 0.5);
	Size dsize = Size(img.cols, img.rows);

	Point center = Point(img.cols / 2.0, img.rows / 2.0);	//ȸ�� �߽�
	Mat M = getRotationMatrix2D(center, 45, 1.0);	//ȸ�� ��� ���	
	warpAffine(img, img, M, dsize, INTER_LINEAR);	//ȸ�� ����

	return img;
}

void n4() {
	int rows = image.rows;
	int cols = image.cols;
	Mat dst_warping1 = clone.clone();
	Mat dst_warping2 = clone.clone();
	Mat dst_warping3 = clone.clone();

	for (int i = 0; i < rows; i++) {  // y ����
		for (int j = 0; j < cols; j++) { // x ����
			int offset_x = 0;
			int offset_y = (int)(25.0 * sin(2 * 3.14 * j / image.rows));  // sine wave
			dst_warping1.at<Vec3b>(i, j) = image.at<Vec3b>((i + offset_y) % rows, j);
		}
	}

	for (int i = 0; i < rows; i++) {  // y ����
		for (int j = 0; j < cols; j++) { // x ����
			int offset_x = 0;
			int offset_y = (int)(25.0 * sin(2 * 3.14 * j / 90));  // sine wave
			dst_warping2.at<Vec3b>(i, j) = image.at<Vec3b>((i + offset_y) % rows, j);
		}
	}

	for (int i = 0; i < rows; i++) {  // y ����
		for (int j = 0; j < cols; j++) { // x ����
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

	for (int i = 0; i < rows; i++) {  // y ����
		for (int j = 0; j < cols; j++) { // x ����
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

	// ���� ���� �׷����� ���� �κ��� white�� �������
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
	bitwise_and(dst6, mask, dst6);	// white�� �κ��� �� ó���� �������� ����

	// �� ó�� �ȵ� ���� �� �κ��� ���� ��������
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
	vector<Mat> bgr_planes;	// ����(Mat)���� ���� bgr_planes[0], [1], [2]
	split(flower, bgr_planes);	// �Է� ������ ���󺰷� �и��Ѵ�. 

	int histSize = 256;	// ������׷� ���� �׷����� ����
	float range[] = { 0, 256 }; 	// ȭ�Ұ� ����
	const float* histRange = { range };
	bool uniform = true, accumulate = false;

	// ���󿡼� �� ȭ�� ���� ���� ������ ���Ѵ�.
	Mat b_hist, g_hist, r_hist;
	calcHist(&bgr_planes[0], 1, 0, Mat(), b_hist, 1, &histSize, &histRange, uniform, accumulate);
	calcHist(&bgr_planes[1], 1, 0, Mat(), g_hist, 1, &histSize, &histRange, uniform, accumulate);
	calcHist(&bgr_planes[2], 1, 0, Mat(), r_hist, 1, &histSize, &histRange, uniform, accumulate);

	// ���� �׷����� �׷����� ������ �����Ѵ�. 
	int hist_w = 512, hist_h = 400;
	int bin_w = cvRound((double)hist_w / histSize); // ������ �� 512/256=2
	Mat histImage(hist_h, hist_w, CV_8UC3, Scalar(0, 0, 0)); // black

	// ������ ������ ����� �ʵ��� ����ȭ�Ѵ�. 
	normalize(b_hist, b_hist, 0, histImage.rows, NORM_MINMAX, -1, Mat());
	normalize(g_hist, g_hist, 0, histImage.rows, NORM_MINMAX, -1, Mat());
	normalize(r_hist, r_hist, 0, histImage.rows, NORM_MINMAX, -1, Mat());

	// ������׷��� ���� ����� �׸���. 
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
	cvtColor(flower, flower, cv::COLOR_BGR2GRAY);	// ����� ��� �и��� ���� ������� �������
	threshold(flower, threshold_image, 150, 255, THRESH_BINARY);
	cvtColor(threshold_image, threshold_image, cv::COLOR_GRAY2BGR);		// ������� ������� ���� �ٽ� �÷��� ������� 

	Mat dst7 = clone.clone();
	bitwise_and(clone, threshold_image, dst7);      // (source, mask, target)
	
	imshow("Segmentation", threshold_image);
	imshow("Segmentation MyPhoto", dst7);
}

int i = 0;	// 2�� ���콺 Ŭ������ �׷��� �簢�� ǥ�� ���� ����
// ���콺 �̺�Ʈ�� �߻��ϸ� ȣ��Ǵ� �ݹ� �Լ� 
void onMouse(int event, int x, int y, int flags, void* param) {
	// 1�� ���콺 �Է�
	if (event == EVENT_LBUTTONDOWN) {	 // ���콺�� ���� ��ư�� ������
		mx1 = x; 			 // �簢���� ���� ��� ��ǥ ����
		my1 = y;
		cropping = true;
	}
	else if (event == EVENT_LBUTTONUP) { // ���콺�� ���� ��ư���� ���� ����
		mx2 = x; 			  // �簢���� ���� �ϴ� ��ǥ ����
		my2 = y;
		cropping = false;
		rectangle(image, Rect(mx1, my1, mx2 - mx1, my2 - my1), Scalar(0, 255, 255), 1.5);
		imshow("MyPhoto", image);		// �̹��� ���
		check = true;
	}

	// 2�� ���콺 �Է�
	if (pp) {
		if (event == EVENT_LBUTTONDOWN) {	 // ���콺�� ���� ��ư�� ������
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

	// 6�� ���콺 �Է�
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
		imshow("MyPhoto", image);		// �̹��� ���
	}
}

int main() {				
	image = imread("kihyeon.jpg", IMREAD_COLOR);	// �̹����� ����
	if (image.empty()) {
		cout << "kihyeon ������ ���� �� ����" << endl;
		return -1;
	}
	
	resize(image, image, Size(400, 400));
	clone = image.clone();	// ���纻�� ������
	imshow("MyPhoto", image);
	
	flower = imread("flower.png", IMREAD_COLOR);	// �̹����� ����
	if (flower.empty()) {
		cout << "flower ������ ���� �� ����" << endl;
		return -1;
	}
	resize(flower, flower, Size(400, 400));

	setMouseCallback("MyPhoto", onMouse, &image);	// �ݹ� �Լ� ���

	while (1) {
		int key = waitKeyEx();	// ����ڷκ��� Ű�� ��ٸ�

		if (key == 'q') break;	// ����ڰ� ��q'�� ������ ����
		else if (key == 'i') {	// 1��
			int key = waitKey(100);
			if (check) {	  // ���� ���콺�� UP�ϸ� �߶� �̹��� ������
				imshow("Bilinear Interpolation", n1());		// �̹��� ���
				check = false;
			}
		}
		else if (key == 'p') {	// 2��
			cout << "2�� ��ǥ\n";
			pp = true;
		}
		else if (key == 's') {	// 3��
			Mat dst3 = clone.clone();
			dst3 = n3(dst3);
			imshow("Scale & Rotation", dst3);
		}
		else if (key == 'w') {	// 4��
			n4();
		}
		else if (key == 'a') {	// 5��
			n5();
		}
		else if (key == 'b') {	// 6��
			n6();
		}
		else if (key == 'g') {	// 7��
			n7();
		}
	}

	waitKey(0);						// Ű���� �Է��� ��ٸ�
	return 0;
}