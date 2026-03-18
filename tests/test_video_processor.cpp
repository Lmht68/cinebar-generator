#include "../src/video_processor.h"

#include <gtest/gtest.h>
#include <opencv2/opencv.hpp>
#include <opencv2/core/utils/logger.hpp>

namespace app_video_processor
{
    class OpenCVLogSilencer
    {
    public:
        OpenCVLogSilencer()
        {
            cv::utils::logging::setLogLevel(cv::utils::logging::LOG_LEVEL_SILENT);
        }
    };

    static OpenCVLogSilencer silence;

    TEST(VideoProcessorTest, LoadVideoInfo_NonExistentFile)
    {
        EXPECT_THROW(
            LoadVideoInfo("nonexistent_file.mp4"),
            std::runtime_error);
    }

    TEST(VideoProcessorTest, GetFrameCount_InvalidInterval)
    {
        EXPECT_THROW(
            GetFrameCountFromInterval(100, 30.0, 0.0),
            std::invalid_argument);
    }

    TEST(VideoProcessorTest, GetFrameCount_InvalidFPS)
    {
        EXPECT_THROW(
            GetFrameCountFromInterval(100, 0.0, 1.0),
            std::runtime_error);
    }

    TEST(VideoProcessorTest, GetFrameCount_IntervalTooSmall)
    {
        // frame_count = 3000; arbitrary valid
        // fps = 30.0;         30 FPS
        // interval = 0.01;    too small (100 FPS sampling)

        EXPECT_THROW(
            GetFrameCountFromInterval(3000, 30.0, 0.01),
            std::invalid_argument);
    }

    TEST(VideoProcessorTest, GetFrameCount_InvalidFrameCount)
    {
        EXPECT_THROW(
            GetFrameCountFromInterval(0, 30.0, 1.0),
            std::runtime_error);
    }

    TEST(VideoProcessorTest, GetFrameCount_ComputesCorrectValue)
    {
        // 300 frames at 30 fps = 10 seconds
        // interval = 2 seconds
        // expected = round(10 / 2) = 5
        int result = GetFrameCountFromInterval(300, 30.0, 2.0);
        EXPECT_EQ(result, 5);
    }

    TEST(VideoProcessorTest, GetFrameCount_RoundsCorrectly)
    {
        // 100 frames at 30 fps = 3.3333 sec
        // interval = 1 sec
        // 3.3333 -> round -> 3
        int result = GetFrameCountFromInterval(100, 30.0, 1.0);
        EXPECT_EQ(result, 3);
    }

    TEST(VideoProcessorTest, InvalidInterval)
    {
        VideoInfo info{
            {},   // capture (not used in this function)
            300,  // frame_count
            30.0, // fps
            1920,
            1080};

        EXPECT_THROW(
            NframesFromInterval(info, 0.0),
            std::invalid_argument);
    }

    TEST(VideoProcessorTest, InvalidFrameCount)
    {
        VideoInfo info{
            {},
            0, // invalid frame_count
            30.0,
            1920,
            1080};

        EXPECT_THROW(
            NframesFromInterval(info, 1.0),
            std::runtime_error);
    }

    TEST(DetectBoundsTest, DetectsLetterboxTopBottom)
    {
        int width = 200;
        int height = 100;
        cv::Mat img(height, width, CV_8UC1, cv::Scalar(255));
        // add black bars
        img.rowRange(0, 10).setTo(0);
        img.rowRange(height - 10, height).setTo(0);

        auto bounds = DetectBounds(img, 20, 0.9);

        ASSERT_TRUE(bounds.has_value());
        EXPECT_NEAR(bounds->top, 10, 5);
        EXPECT_NEAR(bounds->bottom, 90, 5);
        EXPECT_NEAR(bounds->left, 0, 2);
        EXPECT_NEAR(bounds->right, width - 1, 5);
    }

    TEST(DetectBoundsTest, DetectPillarboxLeftRight)
    {
        int width = 200;
        int height = 100;
        cv::Mat img(height, width, CV_8UC1, cv::Scalar(255));
        img.colRange(0, 20).setTo(0);
        img.colRange(width - 20, width).setTo(0);

        auto bounds = DetectBounds(img, 20, 0.9);

        ASSERT_TRUE(bounds.has_value());
        EXPECT_NEAR(bounds->left, 20, 5);
        EXPECT_NEAR(bounds->right, width - 20, 5);
        EXPECT_NEAR(bounds->top, 0, 2);
        EXPECT_NEAR(bounds->bottom, height - 1, 5);
    }

    TEST(DetectBoundsTest, ReturnsFullFrameIfNoBlackBars)
    {
        int width = 200;
        int height = 100;
        cv::Mat img(height, width, CV_8UC1, cv::Scalar(255));

        auto bounds = DetectBounds(img, 20, 0.9);

        ASSERT_TRUE(bounds.has_value());
        EXPECT_NEAR(bounds->left, 0, 2);
        EXPECT_NEAR(bounds->top, 0, 2);
        EXPECT_NEAR(bounds->right, width - 1, 5);
        EXPECT_NEAR(bounds->bottom, height - 1, 5);
    }

    TEST(DetectBoundsTest, ReturnsNulloptIfEntireFrameBlack)
    {
        cv::Mat img(100, 200, CV_8UC1, cv::Scalar(0));
        auto bounds = DetectBounds(img, 20, 0.9);
        EXPECT_FALSE(bounds.has_value());
    }

    TEST(CropImageTest, CropsCorrectRegion)
    {
        cv::Mat img(100, 200, CV_8UC1, cv::Scalar(255));
        VideoBounds bounds{10, 20, 110, 80};

        cv::Mat cropped = CropImage(img, bounds);

        EXPECT_EQ(cropped.cols, 100);
        EXPECT_EQ(cropped.rows, 60);
    }

    TEST(CropImageTest, PreservesPixels)
    {
        cv::Mat img(100, 200, CV_8UC1);

        for (int y = 0; y < img.rows; y++)
            for (int x = 0; x < img.cols; x++)
                img.at<uchar>(y, x) = x;

        VideoBounds bounds{50, 10, 100, 60};
        cv::Mat cropped = CropImage(img, bounds);

        EXPECT_EQ(cropped.at<uchar>(0, 0), 50);
        EXPECT_EQ(cropped.at<uchar>(0, 10), 60);
    }

    TEST(DetectBoundsTest, SingleFrameLetterbox)
    {
        int width = 200;
        int height = 100;
        cv::Mat img(height, width, CV_8UC1, cv::Scalar(255));
        // add black bars
        img.rowRange(0, 10).setTo(0);
        img.rowRange(height - 10, height).setTo(0);

        auto bounds = DetectBounds(img, 20, 0.9);

        ASSERT_TRUE(bounds.has_value());
        EXPECT_NEAR(bounds->top, 10, 5);
        EXPECT_NEAR(bounds->bottom, 90, 5);
        EXPECT_NEAR(bounds->left, 0, 2);
        EXPECT_NEAR(bounds->right, width - 1, 5);
    }

    TEST(VideoBoundsLogicTest, RejectsTinyCrop)
    {
        int width = 200;
        int height = 100;
        cv::Mat img(height, width, CV_8UC1, cv::Scalar(255));

        // Use DetectBounds to get full-frame bounds (no black bars)
        auto bounds = DetectBounds(img, 20, 0.9);

        ASSERT_TRUE(bounds.has_value());

        int crop_w = bounds->right - bounds->left;
        int crop_h = bounds->bottom - bounds->top;
        // Mimic DetermineVideoBounds min-crop logic
        bool can_crop = !(crop_w > width * kMinCropRatio &&
                          crop_h > height * kMinCropRatio);

        // Expect can_crop = true only if crop is not too large
        // For this full-frame case, crop_h < height * kMinCropRatio → can_crop = true
        EXPECT_TRUE(can_crop);
    }
}