#include <iostream>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

int main() {
	// Step 1: read images
	// This is an image in which the three channels are concatenated vertically
	Mat img = imread("images/emir.jpg", IMREAD_GRAYSCALE);

	// Find the width and height of the color image
	Size sz = img.size();
	int height = sz.height / 3;
	int width = sz.width;

	// Extract the three channels from the gray scale image
	vector<Mat>channels;
	channels.push_back(img(Rect(0, 0, width, height)));
	channels.push_back(img(Rect(0, height, width, height)));
	channels.push_back(img(Rect(0, 2 * height, width, height)));

	Mat blue = channels[0];
	Mat green = channels[1];
	Mat red = channels[2];

	Mat cropped;
	hconcat(channels, cropped);

	namedWindow("BGR", WINDOW_AUTOSIZE);
	imshow("BGR", cropped);
	waitKey();

	// Step 2: detect features
	int MAX_FEATURES = 5000;			// detect 5000 ORB features
	float GOOD_MATCH_PERCENT = 0.1f;	// pick top 10% ORB features

	// Detect ORB features and compute descriptors. Find the keypoints and descriptors for each channel using ORB and store them in respective variables
	// e.g. 'keyPointsBlue' and 'descriptorsBlue' are the keypoints and descriptors for the blue channel
	std::vector<KeyPoint> keypointsBlue, keypointsGreen, keypointsRed;
	Mat descriptorsBlue, descriptorsGreen, descriptorsRed;

	Ptr<Feature2D> orb = ORB::create(MAX_FEATURES);
	orb->detectAndCompute(blue, Mat(), keypointsBlue, descriptorsBlue);
	orb->detectAndCompute(green, Mat(), keypointsGreen, descriptorsGreen);
	orb->detectAndCompute(red, Mat(), keypointsRed, descriptorsRed);

	// Step 3: Match features
	Mat img2;
	drawKeypoints(blue, keypointsBlue, img2, Scalar(255, 0, 0), DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
	drawKeypoints(green, keypointsGreen, img2, Scalar(0, 255, 0), DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
	drawKeypoints(red, keypointsRed, img2, Scalar(0, 0, 255), DrawMatchesFlags::DRAW_RICH_KEYPOINTS);

	// Match features between blue and green channels
	std::vector<DMatch> matchesBlueGreen;
	Ptr<DescriptorMatcher> matcher = DescriptorMatcher::create("BruteForce-Hamming");
	matcher->match(descriptorsBlue, descriptorsGreen, matchesBlueGreen, Mat());

	// Sort matches by score
	std::sort(matchesBlueGreen.begin(), matchesBlueGreen.end());

	// Remove not so good matches
	int numGoodMatches = matchesBlueGreen.size() * GOOD_MATCH_PERCENT;
	matchesBlueGreen.erase(matchesBlueGreen.begin() + numGoodMatches, matchesBlueGreen.end());

	// Draw top matches
	Mat imMatchesBlueGreen;
	drawMatches(blue, keypointsBlue, green, keypointsGreen, matchesBlueGreen, imMatchesBlueGreen);

	namedWindow("matchesBlueGreen", WINDOW_AUTOSIZE);
	imshow("matchesBlueGreen", imMatchesBlueGreen);
	waitKey();

	// Match features between Red and Green channels
	std::vector<DMatch> matchesRedGreen;
	matcher->match(descriptorsRed, descriptorsGreen, matchesRedGreen, Mat());

	// Sort matches by score
	std::sort(matchesRedGreen.begin(), matchesRedGreen.end());

	// Remove not so good matches
	numGoodMatches = matchesRedGreen.size() * GOOD_MATCH_PERCENT;
	matchesRedGreen.erase(matchesRedGreen.begin() + numGoodMatches, matchesRedGreen.end());

	// Draw top matches
	Mat imMatchesRedGreen;
	drawMatches(red, keypointsRed, green, keypointsGreen, matchesRedGreen, imMatchesRedGreen);

	namedWindow("matchesRedGreen", WINDOW_AUTOSIZE);
	imshow("matchesRedGreen", imMatchesRedGreen);
	waitKey();

	// Step 4: calculate homography

	// Between blue and green channels
	// Extract location of good matches
	std::vector<Point2f> pointsBlue, pointsGreen;

	for (size_t i = 0; i < matchesBlueGreen.size(); i++)
	{
		pointsBlue.push_back(keypointsBlue[matchesBlueGreen[i].queryIdx].pt);
		pointsGreen.push_back(keypointsGreen[matchesBlueGreen[i].trainIdx].pt);
	}

	// Find homography
	Mat hBlueGreen = findHomography(pointsBlue, pointsGreen, RANSAC);

	// Between red and green channels
	// Extract location of good matches
	std::vector<Point2f> pointsRed;
	pointsGreen.clear();

	for (size_t i = 0; i < matchesRedGreen.size(); i++)
	{
		pointsRed.push_back(keypointsRed[matchesRedGreen[i].queryIdx].pt);
		pointsGreen.push_back(keypointsGreen[matchesRedGreen[i].trainIdx].pt);
	}

	// Find homography
	Mat hRedGreen = findHomography(pointsRed, pointsGreen, RANSAC);

	// Step 5: warp images
	// Use homography to find blueWarped and RedWarped images
	Mat blueWarped, redWarped;
	warpPerspective(blue, blueWarped, hBlueGreen, green.size());
	warpPerspective(red, redWarped, hRedGreen, green.size());

	// Step 6: merge channels
	Mat colorImage;
	vector<Mat> colorImageChannels{ blueWarped, green, redWarped };
	merge(colorImageChannels, colorImage);

	Mat originalImage;
	merge(channels, originalImage);

	Mat compare;
	hconcat(originalImage, colorImage, compare);
	namedWindow("comparison", WINDOW_AUTOSIZE);
	imshow("comparison", compare);
	waitKey();

	return 0;
}
