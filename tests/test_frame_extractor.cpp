#include <gtest/gtest.h>
#include <opencv2/opencv.hpp>
#include "frame_extractor.h"

using namespace app_frame_extractor;

// Helper: create a solid color frame
cv::Mat CreateSolidColorFrame(const cv::Vec3b &color, int width = 10, int height = 10)
{
    cv::Mat frame(height, width, CV_8UC3, color);
    return frame;
}

TEST(FrameExtractorTests, ExtractSmoothedColor_Solid)
{
    cv::Vec3b color(50, 100, 150);
    cv::Mat frame = CreateSolidColorFrame(color, 20, 30);
    cv::Vec3b result = ExtractSmoothedColor(frame);
    EXPECT_EQ(result, color);
}

TEST(FrameExtractorTests, ExtractColorMean_Solid)
{
    cv::Vec3b color(10, 20, 30);
    cv::Mat frame = CreateSolidColorFrame(color, 15, 15);
    cv::Vec3b result = ExtractColorMean(frame);
    EXPECT_EQ(result, color);
}

TEST(FrameExtractorTests, ExtractColorkMeans_Solid)
{
    cv::Vec3b color(200, 50, 100);
    cv::Mat frame = CreateSolidColorFrame(color, 10, 10);
    cv::Vec3b result = ExtractColorkMeans(frame);
    EXPECT_EQ(result, color);
}

TEST(FrameExtractorTests, ExtractDominantHue_SolidRed)
{
    // Red in BGR = (0,0,255)
    cv::Vec3b red(0, 0, 255);
    cv::Mat frame = CreateSolidColorFrame(red, 20, 20);
    cv::Vec3b result = ExtractDominantHue(frame);
    // Hue 0 corresponds to red → result should be red-ish
    EXPECT_NEAR(result[0], red[0], 5);
    EXPECT_NEAR(result[1], red[1], 5);
    EXPECT_NEAR(result[2], red[2], 5);
}

TEST(FrameExtractorTests, ExtractFrameStripe_Size)
{
    cv::Vec3b color(100, 150, 200);
    cv::Mat frame = CreateSolidColorFrame(color, 10, 20);
    int width = 5;
    cv::Mat stripe = ExtractMeanFrameStripe(frame, width);
    EXPECT_EQ(stripe.rows, frame.rows);
    EXPECT_EQ(stripe.cols, width);

    for (int r = 0; r < stripe.rows; ++r)
        for (int c = 0; c < stripe.cols; ++c)
            EXPECT_EQ(stripe.at<cv::Vec3b>(r, c), color);
}
