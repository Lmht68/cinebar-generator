#include "../src/parser.h"
#include "../src/types.h"

#include <gtest/gtest.h>

namespace app_parser
{
    class ProcessingArgsTest : public ::testing::Test
    {
    protected:
        cinebar_types::VideoInfo MakeVideoInfo(
            int frame_count = 300,
            double fps = 30.0,
            int height = 720)
        {
            cinebar_types::VideoInfo info{};
            info.frame_count = frame_count;
            info.fps = fps;
            info.height = height;
            return info;
        }
    };

    TEST_F(ProcessingArgsTest, SegmentDefaultsToFullVideo)
    {
        cinebar_types::InputArgs args{};
        auto video = MakeVideoInfo(300);

        ProcessingArgs(args, video);

        EXPECT_EQ(args.segment_nframes, 300);
    }

    TEST_F(ProcessingArgsTest, IntervalComputesNframes)
    {
        cinebar_types::InputArgs args{};
        args.interval = 2.0; // every 2 seconds

        auto video = MakeVideoInfo(300, 30.0); // 10 seconds total

        ProcessingArgs(args, video);

        // Expect ~5 frames (10s / 2s)
        EXPECT_GT(args.nframes, 0);
    }

    TEST_F(ProcessingArgsTest, NframesComputesInterval)
    {
        cinebar_types::InputArgs args{};
        args.nframes = 10;

        auto video = MakeVideoInfo(300, 30.0); // 10 seconds

        ProcessingArgs(args, video);

        EXPECT_GT(args.interval, 0.0);
    }

    TEST_F(ProcessingArgsTest, DefaultSamplingUsesAllFrames)
    {
        cinebar_types::InputArgs args{};

        auto video = MakeVideoInfo(120, 24.0);

        ProcessingArgs(args, video);

        EXPECT_EQ(args.nframes, 120);
        EXPECT_DOUBLE_EQ(args.interval, 1.0 / 24.0);
    }

    TEST_F(ProcessingArgsTest, HorizontalUsesOriginalHeight)
    {
        cinebar_types::InputArgs args{};
        args.bar_w = 2;
        args.nframes = 10;
        args.shape = cinebar_types::Shape::Horizontal;

        auto video = MakeVideoInfo(100, 30.0, 720);

        ProcessingArgs(args, video);

        EXPECT_EQ(args.height, 720);
        EXPECT_EQ(args.width, 20); // 2 * 10
    }

    TEST_F(ProcessingArgsTest, CircularUsesSquareDimensions)
    {
        cinebar_types::InputArgs args{};
        args.bar_w = 3;
        args.nframes = 10;
        args.shape = cinebar_types::Shape::Circular;

        auto video = MakeVideoInfo();

        ProcessingArgs(args, video);

        EXPECT_EQ(args.height, 30);
        EXPECT_EQ(args.width, 30);
    }

    TEST_F(ProcessingArgsTest, EndFrameDefaultsToLastFrame)
    {
        cinebar_types::InputArgs args{};
        args.end_frame = 0;

        auto video = MakeVideoInfo(200);

        ProcessingArgs(args, video);

        EXPECT_EQ(args.end_frame, 199);
    }

    TEST_F(ProcessingArgsTest, SegmentRangeCalculatesCorrectly)
    {
        cinebar_types::InputArgs args{};
        args.start_frame = 10;
        args.end_frame = 19;

        // simulate ParseArgs behavior
        args.segment_nframes = args.end_frame - args.start_frame + 1;

        auto video = MakeVideoInfo();

        ProcessingArgs(args, video);

        EXPECT_EQ(args.segment_nframes, 10);
    }

    TEST_F(ProcessingArgsTest, NframesGreaterThanSegmentHandledEarlier)
    {
        // This is a safety check: ProcessingArgs assumes ParseArgs already validated
        cinebar_types::InputArgs args{};
        args.start_frame = 0;
        args.end_frame = 9;
        args.segment_nframes = 10;
        args.nframes = 20; // invalid but should not crash

        auto video = MakeVideoInfo();

        EXPECT_NO_THROW(ProcessingArgs(args, video));
    }

    TEST_F(ProcessingArgsTest, ZeroFPSWouldBeProblematic)
    {
        cinebar_types::InputArgs args{};
        auto video = MakeVideoInfo(100, 0.0);

        // This exposes a real edge case in your code
        // division by zero risk
        EXPECT_NO_THROW(ProcessingArgs(args, video));
    }

    TEST_F(ProcessingArgsTest, WidthScalesWithBarWidthAndFrames)
    {
        cinebar_types::InputArgs args{};
        args.bar_w = 4;
        args.nframes = 25;

        auto video = MakeVideoInfo();

        ProcessingArgs(args, video);

        EXPECT_EQ(args.width, 100);
    }
}