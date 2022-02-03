#include <iostream>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

int MAX_FEATURES = 5000;			// detect 5000 ORB features
float GOOD_MATCH_PERCENT = 0.1f;	// pick top 10% ORB features
bool fDrawMatches = true;			// a flag for displaying matches

// Read and cut image into RGB channels
vector<Mat> readImg(const string& path) {
	Mat img = imread("images/emir.jpg", IMREAD_GRAYSCALE);

	// Find the width and height of the color image
	Size sz = img.size();
	int height = sz.height / 3;
	int width = sz.width;

	// Extract the three channels from the gray scale image
	vector<Mat> channels;
	channels.push_back(img(Rect(0, 0, width, height)));
	channels.push_back(img(Rect(0, height, width, height)));
	channels.push_back(img(Rect(0, 2 * height, width, height)));

	Mat cropped;
	hconcat(channels, cropped);

	namedWindow("BGR", WINDOW_AUTOSIZE);
	imshow("BGR", cropped);
	waitKey();

	return channels;
}

// Detect ORB features. Find the keypoints and descriptors for RGB channel using ORB and store them in respective variables
void detectFeatures(const vector<Mat>& channels, vector<vector<KeyPoint>>& keypointsList, vector<Mat>& descriptorsList) {
	Ptr<Feature2D> orb = ORB::create(MAX_FEATURES);
	orb->detectAndCompute(channels[0], Mat(), keypointsList[0], descriptorsList[0]);	// blue channel
	orb->detectAndCompute(channels[1], Mat(), keypointsList[1], descriptorsList[1]);	// green channel
	orb->detectAndCompute(channels[2], Mat(), keypointsList[2], descriptorsList[2]);	// red channel
}

void matchFeatures(const Mat& descriptors1, const Mat& descriptors2, vector<DMatch>& matches) {
	Ptr<DescriptorMatcher> matcher = DescriptorMatcher::create("BruteForce-Hamming");
	matcher->match(descriptors1, descriptors2, matches, Mat());

	// Sort matches by hamming distance (the smaller the distance, the more similar the match)
	sort(matches.begin(), matches.end());

	// Remove not so good matches
	int numGoodMatches = matches.size() * GOOD_MATCH_PERCENT;
	matches.erase(matches.begin() + numGoodMatches, matches.end());
}

void computeHomography(const vector<DMatch>& matches, const vector<KeyPoint>& keypointsSrc, const vector<KeyPoint>& keypointsDst, Mat& homography) {
	vector<Point2f> goodPtsSrc, goodPtsDst;

	// Extract locations of good matches
	for (size_t i = 0; i < matches.size(); i++)
	{
		goodPtsSrc.push_back(keypointsSrc[matches[i].queryIdx].pt);
		goodPtsDst.push_back(keypointsDst[matches[i].trainIdx].pt);
	}

	homography = findHomography(goodPtsSrc, goodPtsDst, RANSAC);
}

void displayTopMatches(const vector<Mat>& channels, vector<vector<KeyPoint>>& keypointsList, vector<DMatch>& matchesBG, vector<DMatch>& matchesRG) {
	Mat imgMatchesBG, imgMatchesRG;

	drawMatches(channels[0], keypointsList[0], channels[1], keypointsList[1], matchesBG, imgMatchesBG);	// blue-green matches
	drawMatches(channels[2], keypointsList[2], channels[1], keypointsList[1], matchesRG, imgMatchesRG);	// red-green matches

	namedWindow("BlueGreenMatches", WINDOW_AUTOSIZE);
	imshow("BlueGreenMatches", imgMatchesBG);

	namedWindow("RedGreenMatches", WINDOW_AUTOSIZE);
	imshow("RedGreenMatches", imgMatchesRG);

	waitKey();
}

void displayResults(const vector<Mat>& channels, const Mat& warpB, const Mat& warpR) {
	// Merge aligned channels to a single color image
	Mat colorImage;
	vector<Mat> colorImageChannels{ warpB, channels[1], warpR };
	merge(colorImageChannels, colorImage);

	// Direct superimposition without alignment
	Mat originalImage;
	merge(channels, originalImage);

	// Compare results
	Mat compare;
	hconcat(originalImage, colorImage, compare);
	namedWindow("comparison", WINDOW_AUTOSIZE);
	imshow("comparison", compare);

	waitKey();
}

int main() {
	// Step 1: read the image in which the three channels are concatenated vertically
	vector<Mat> channels = readImg("images/emir.jpg");

	// Step 2: detect features
	vector<vector<KeyPoint>> keypointsList(3);
	vector<Mat> descriptorsList(3);
	detectFeatures(channels, keypointsList, descriptorsList);

	// Step 3: Match features
	vector<DMatch> matchesBG, matchesRG;
	matchFeatures(descriptorsList[0], descriptorsList[1], matchesBG);	// between blue and green channels
	matchFeatures(descriptorsList[2], descriptorsList[1], matchesRG);	// between red and green channels

	// Display top matches (optional)
	if (fDrawMatches)
		displayTopMatches(channels, keypointsList, matchesBG, matchesRG);

	// Step 4: Compute homography
	Mat homographyBG, homographyRG;
	computeHomography(matchesBG, keypointsList[0], keypointsList[1], homographyBG); // between blue and green channels
	computeHomography(matchesRG, keypointsList[2], keypointsList[1], homographyRG);	// between red and green channels

	// Step 5: Use homography to find aligned blue and red channels 
	Mat warpB, warpR;
	warpPerspective(channels[0], warpB, homographyBG, channels[1].size());
	warpPerspective(channels[2], warpR, homographyRG, channels[1].size());

	// Step 6: Display results
	displayResults(channels, warpB, warpR);

	return 0;
}
