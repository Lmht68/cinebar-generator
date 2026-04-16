#include <gtest/gtest.h>
#include <opencv2/opencv.hpp>
#include <atomic>

#include "../src/cinebar_generator.h"
#include "../src/types.h"

using namespace cinebar;

namespace
{
    cinebar_types::InputArgs DefaultArgs(int height = 10, int bar_w = 5)
    {
        cinebar_types::InputArgs args;
        args.height = height;
        args.bar_w = bar_w;
        return args;
    }
}

// ===================== Horizontal Barcode =====================

namespace cinebar
{
    // ===================== Horizontal =====================
    TEST(BuildHorizontalBarcodeTest, ThrowsOnEmptyColors)
    {
        std::vector<cv::Vec3b> colors;
        auto args = DefaultArgs();

        std::atomic<int> progress{0};

        EXPECT_THROW(
            BuildHorizontalBarcode(colors, args.height, args.bar_w, progress),
            std::runtime_error);
    }

    TEST(BuildHorizontalBarcodeTest, CorrectDimensions)
    {
        std::vector<cv::Vec3b> colors = {
            {0, 0, 255},
            {0, 255, 0},
            {255, 0, 0}};

        auto args = DefaultArgs(20, 4);

        std::atomic<int> progress{0};

        cv::Mat result = BuildHorizontalBarcode(colors, args.height, args.bar_w, progress);

        EXPECT_EQ(result.rows, 20);
        EXPECT_EQ(result.cols, 3 * 4);
        EXPECT_EQ(result.type(), CV_8UC3);
    }

    TEST(BuildHorizontalBarcodeTest, CorrectColorPlacement)
    {
        std::vector<cv::Vec3b> colors = {
            {10, 20, 30},
            {40, 50, 60}};

        auto args = DefaultArgs(5, 3);

        std::atomic<int> progress{0};

        cv::Mat result = BuildHorizontalBarcode(colors, args.height, args.bar_w, progress);

        for (int y = 0; y < result.rows; ++y)
        {
            for (int x = 0; x < 3; ++x)
            {
                EXPECT_EQ(result.at<cv::Vec3b>(y, x), colors[0]);
            }
            for (int x = 3; x < 6; ++x)
            {
                EXPECT_EQ(result.at<cv::Vec3b>(y, x), colors[1]);
            }
        }
    }

    // ===================== Stripes =====================

    TEST(BuildHorizontalBarcodeFromStripesTest, ThrowsOnEmptyStripes)
    {
        std::vector<cv::Mat> stripes;

        EXPECT_THROW(
            BuildHorizontalBarcodeFromStripes(stripes),
            std::runtime_error);
    }

    TEST(BuildHorizontalBarcodeFromStripesTest, ConcatenatesCorrectly)
    {
        cv::Mat stripe1(5, 2, CV_8UC3, cv::Scalar(0, 0, 255));
        cv::Mat stripe2(5, 3, CV_8UC3, cv::Scalar(0, 255, 0));

        std::vector<cv::Mat> stripes = {stripe1, stripe2};

        cv::Mat result = BuildHorizontalBarcodeFromStripes(stripes);

        EXPECT_EQ(result.rows, 5);
        EXPECT_EQ(result.cols, 5);
    }
}