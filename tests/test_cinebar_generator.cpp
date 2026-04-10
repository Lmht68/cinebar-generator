#include <gtest/gtest.h>
#include <opencv2/opencv.hpp>

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

TEST(BuildHorizontalBarcodeTest, ThrowsOnEmptyColors)
{
    std::vector<cv::Vec3b> colors;
    auto args = DefaultArgs();

    EXPECT_THROW(
        BuildHorizontalBarcode(colors, args),
        std::runtime_error);
}

TEST(BuildHorizontalBarcodeTest, CorrectDimensions)
{
    std::vector<cv::Vec3b> colors = {
        {0, 0, 255},
        {0, 255, 0},
        {255, 0, 0}};

    auto args = DefaultArgs(20, 4);

    cv::Mat result = BuildHorizontalBarcode(colors, args);

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

    cv::Mat result = BuildHorizontalBarcode(colors, args);

    // Check first stripe
    for (int y = 0; y < result.rows; ++y)
    {
        for (int x = 0; x < 3; ++x)
        {
            EXPECT_EQ(result.at<cv::Vec3b>(y, x), colors[0]);
        }
    }

    // Check second stripe
    for (int y = 0; y < result.rows; ++y)
    {
        for (int x = 3; x < 6; ++x)
        {
            EXPECT_EQ(result.at<cv::Vec3b>(y, x), colors[1]);
        }
    }
}

TEST(BuildHorizontalBarcodeTest, ProgressCallbackCalledCorrectly)
{
    std::vector<cv::Vec3b> colors = {
        {0, 0, 0},
        {1, 1, 1},
        {2, 2, 2}};

    auto args = DefaultArgs();

    int call_count = 0;
    std::vector<size_t> progress_values;

    auto callback = [&](size_t current, size_t total)
    {
        call_count++;
        progress_values.push_back(current);
        EXPECT_EQ(total, colors.size());
    };

    BuildHorizontalBarcode(colors, args, callback);

    EXPECT_EQ(call_count, colors.size());

    for (size_t i = 0; i < progress_values.size(); ++i)
    {
        EXPECT_EQ(progress_values[i], i + 1);
    }
}

TEST(BuildHorizontalBarcodeTest, NoCallbackDoesNotCrash)
{
    std::vector<cv::Vec3b> colors = {
        {0, 0, 0},
        {255, 255, 255}};

    auto args = DefaultArgs();

    EXPECT_NO_THROW(BuildHorizontalBarcode(colors, args, nullptr));
}

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

TEST(BuildHorizontalBarcodeFromStripesTest, StartAndFinishCallbacksCalled)
{
    cv::Mat stripe(5, 2, CV_8UC3, cv::Scalar(0, 0, 255));
    std::vector<cv::Mat> stripes = {stripe};

    bool start_called = false;
    bool finish_called = false;

    auto on_start = [&]()
    { start_called = true; };
    auto on_finish = [&]()
    { finish_called = true; };

    BuildHorizontalBarcodeFromStripes(stripes, on_start, on_finish);

    EXPECT_TRUE(start_called);
    EXPECT_TRUE(finish_called);
}

TEST(BuildHorizontalBarcodeFromStripesTest, NullCallbacksDoNotCrash)
{
    cv::Mat stripe(5, 2, CV_8UC3, cv::Scalar(0, 0, 255));
    std::vector<cv::Mat> stripes = {stripe};

    EXPECT_NO_THROW(
        BuildHorizontalBarcodeFromStripes(stripes, nullptr, nullptr));
}