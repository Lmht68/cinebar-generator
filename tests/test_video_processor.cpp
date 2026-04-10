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

    TEST(NframesFromIntervalTest, BasicValidCase)
    {
        size_t frame_count = 300;
        double fps = 30.0;
        double interval = 1.0;

        // duration = 10s → 10 / 1 = 10 frames
        size_t result = NframesFromInterval(frame_count, interval, fps);
        EXPECT_EQ(result, 10);
    }

    TEST(NframesFromIntervalTest, NonIntegerResultRoundsCorrectly)
    {
        size_t frame_count = 300;
        double fps = 30.0;
        double interval = 0.7;

        // duration = 10s → 10 / 0.7 ≈ 14.285 → rounds to 14
        size_t result = NframesFromInterval(frame_count, interval, fps);
        EXPECT_EQ(result, static_cast<size_t>(std::round(10.0 / 0.7)));
    }

    TEST(NframesFromIntervalTest, LargeValues)
    {
        size_t frame_count = 100000;
        double fps = 25.0;
        double interval = 2.0;

        double duration = frame_count / fps;
        size_t expected = static_cast<size_t>(std::round(duration / interval));

        EXPECT_EQ(NframesFromInterval(frame_count, interval, fps), expected);
    }

    TEST(NframesFromIntervalTest, IntervalExactlyOneFrame)
    {
        size_t frame_count = 60;
        double fps = 30.0;
        double interval = 1.0 / fps;

        // duration = 2s → 2 / (1/30) = 60
        size_t result = NframesFromInterval(frame_count, interval, fps);
        EXPECT_EQ(result, frame_count);
    }

    TEST(NframesFromIntervalTest, ThrowsIfIntervalZero)
    {
        EXPECT_THROW(
            NframesFromInterval(100, 0.0, 30.0),
            std::invalid_argument);
    }

    TEST(NframesFromIntervalTest, ThrowsIfIntervalNegative)
    {
        EXPECT_THROW(
            NframesFromInterval(100, -1.0, 30.0),
            std::invalid_argument);
    }

    TEST(NframesFromIntervalTest, ThrowsIfFpsZero)
    {
        EXPECT_THROW(
            NframesFromInterval(100, 1.0, 0.0),
            std::runtime_error);
    }

    TEST(NframesFromIntervalTest, ThrowsIfFpsNegative)
    {
        EXPECT_THROW(
            NframesFromInterval(100, 1.0, -30.0),
            std::runtime_error);
    }

    TEST(NframesFromIntervalTest, ThrowsIfIntervalTooSmall)
    {
        double fps = 30.0;
        double interval = (1.0 / fps) - 0.0001;

        EXPECT_THROW(
            NframesFromInterval(100, interval, fps),
            std::invalid_argument);
    }

    TEST(NframesFromIntervalTest, ThrowsIfFrameCountZero)
    {
        EXPECT_THROW(
            NframesFromInterval(0, 1.0, 30.0),
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

        auto bounds = DetectBounds(img);

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

        auto bounds = DetectBounds(img);

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

        auto bounds = DetectBounds(img);

        ASSERT_TRUE(bounds.has_value());
        EXPECT_NEAR(bounds->left, 0, 2);
        EXPECT_NEAR(bounds->top, 0, 2);
        EXPECT_NEAR(bounds->right, width - 1, 5);
        EXPECT_NEAR(bounds->bottom, height - 1, 5);
    }

    TEST(DetectBoundsTest, ReturnsNulloptIfEntireFrameBlack)
    {
        cv::Mat img(100, 200, CV_8UC1, cv::Scalar(0));
        auto bounds = DetectBounds(img);
        EXPECT_FALSE(bounds.has_value());
    }

    TEST(CropImageTest, CropsCorrectRegion)
    {
        cv::Mat img(100, 200, CV_8UC1, cv::Scalar(255));
        cinebar_types::VideoBounds bounds{10, 20, 110, 80};

        CropImage(img, bounds);

        EXPECT_EQ(img.cols, 100);
        EXPECT_EQ(img.rows, 60);
    }

    TEST(CropImageTest, PreservesPixels)
    {
        cv::Mat img(100, 200, CV_8UC1);

        for (int y = 0; y < img.rows; y++)
            for (int x = 0; x < img.cols; x++)
                img.at<uchar>(y, x) = x;

        cinebar_types::VideoBounds bounds{50, 10, 100, 60};
        CropImage(img, bounds);

        EXPECT_EQ(img.at<uchar>(0, 0), 50);
        EXPECT_EQ(img.at<uchar>(0, 10), 60);
    }

    TEST(DetectBoundsTest, SingleFrameLetterbox)
    {
        int width = 200;
        int height = 100;
        cv::Mat img(height, width, CV_8UC1, cv::Scalar(255));
        // add black bars
        img.rowRange(0, 10).setTo(0);
        img.rowRange(height - 10, height).setTo(0);

        auto bounds = DetectBounds(img);

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
        auto bounds = DetectBounds(img);

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