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

    std::vector<cv::Vec3b> MakeSolidColors(size_t n, cv::Vec3b color)
    {
        return std::vector<cv::Vec3b>(n, color);
    }

    std::vector<cv::Vec3b> MakeGradient(size_t n)
    {
        std::vector<cv::Vec3b> colors;
        colors.reserve(n);

        for (size_t i = 0; i < n; ++i)
        {
            colors.emplace_back(
                static_cast<uchar>(i % 256),
                static_cast<uchar>((i * 2) % 256),
                static_cast<uchar>((i * 3) % 256));
        }
        return colors;
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

    // ===================== Circular =====================

    TEST(CircularBarcode, ReturnsValidImage)
    {
        std::atomic<int> progress{0};
        auto colors = MakeSolidColors(100, {10, 20, 30});
        int img_size = 256;

        cv::Mat result = BuildCircularBarcode(colors, img_size, progress);

        ASSERT_FALSE(result.empty());
        EXPECT_EQ(result.rows, img_size);
        EXPECT_EQ(result.cols, img_size);
        EXPECT_EQ(result.type(), CV_8UC4);
    }

    TEST(CircularBarcode, EmptyInputReturnsEmpty)
    {
        std::atomic<int> progress{0};
        std::vector<cv::Vec3b> colors;

        cv::Mat result = BuildCircularBarcode(colors, 256, progress);

        EXPECT_TRUE(result.empty());
    }

    TEST(CircularBarcode, SingleColorFillsCenter)
    {
        std::atomic<int> progress{0};
        std::vector<cv::Vec3b> colors = {cv::Vec3b(0, 0, 255)};
        int img_size = 128;

        cv::Mat result = BuildCircularBarcode(colors, img_size, progress);

        cv::Vec4b center = result.at<cv::Vec4b>(img_size / 2, img_size / 2);

        EXPECT_EQ(center[0], 0);
        EXPECT_EQ(center[1], 0);
        EXPECT_EQ(center[2], 255);
        EXPECT_EQ(center[3], 255);
    }

    TEST(CircularBarcode, MultipleColorsProduceDifferentRings)
    {
        std::atomic<int> progress{0};
        std::vector<cv::Vec3b> colors = {
            {255, 0, 0},
            {0, 255, 0},
            {0, 0, 255}};
        int img_size = 200;

        cv::Mat result = BuildCircularBarcode(colors, img_size, progress);

        int center = img_size / 2;

        cv::Vec4b inner = result.at<cv::Vec4b>(center, center + 10);
        cv::Vec4b middle = result.at<cv::Vec4b>(center, center + 40);
        cv::Vec4b outer = result.at<cv::Vec4b>(center, center + 70);

        EXPECT_NE(inner, middle);
        EXPECT_NE(middle, outer);
    }

    TEST(CircularBarcode, RingsExpandOutward)
    {
        std::atomic<int> progress{0};
        auto colors = MakeGradient(10);
        int img_size = 200;

        cv::Mat result = BuildCircularBarcode(colors, img_size, progress);

        int center = img_size / 2;
        cv::Vec4b near_center = result.at<cv::Vec4b>(center, center + 5);
        cv::Vec4b far_edge = result.at<cv::Vec4b>(center, center + 80);

        EXPECT_NE(near_center, far_edge);
    }

    TEST(CircularBarcode, SmallImage)
    {
        std::atomic<int> progress{0};
        auto colors = MakeGradient(5);
        int img_size = 16;

        cv::Mat result = BuildCircularBarcode(colors, img_size, progress);

        ASSERT_FALSE(result.empty());
        EXPECT_EQ(result.rows, img_size);
        EXPECT_EQ(result.cols, img_size);
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