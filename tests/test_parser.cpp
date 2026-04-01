#include "../src/parser.h"
#include "../src/types.h"

#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

namespace fs = std::filesystem;

namespace app_parser
{
    class ParserTest : public ::testing::Test
    {
    protected:
        fs::path temp_video;

        void SetUp() override
        {
            // create a dummy file so CLI::ExistingFile passes
            temp_video = "test_video.mp4";
            std::ofstream(temp_video.string()).close();
        }

        void TearDown() override
        {
            if (fs::exists(temp_video))
                fs::remove(temp_video);
        }

        cinebar_types::InputArgs Parse(const std::vector<std::string> &args)
        {
            std::vector<char *> argv;
            argv.reserve(args.size());

            for (const auto &s : args)
                argv.push_back(const_cast<char *>(s.c_str()));

            return ParseArgs(
                static_cast<int>(argv.size()),
                argv.data());
        }
    };

    TEST_F(ParserTest, VersionDoesNotRequireInput)
    {
        auto result = Parse({"cinebar", "--version"});

        EXPECT_TRUE(result.show_info);
    }

    TEST_F(ParserTest, MissingInputThrows)
    {
        EXPECT_THROW(
            Parse({"cinebar"}),
            CLI::RequiredError);
    }

    TEST_F(ParserTest, IntervalAndNframesConflict)
    {
        EXPECT_THROW(
            Parse({"cinebar",
                   temp_video.string(),
                   "--interval", "1",
                   "--frames", "100"}),
            CLI::ValidationError);
    }

    TEST_F(ParserTest, DefaultOutputFilenameGenerated)
    {
        auto result = Parse({"cinebar",
                             temp_video.string()});

        EXPECT_EQ(result.output_img_path, "test_video.png");
    }

    TEST_F(ParserTest, OutputOptionOverridesDefault)
    {
        auto result = Parse({"cinebar",
                             temp_video.string(),
                             "-o", "custom.png"});

        EXPECT_EQ(result.output_img_path, "custom.png");
    }

    TEST_F(ParserTest, IntervalOnlyWorks)
    {
        auto result = Parse({"cinebar",
                             temp_video.string(),
                             "--interval", "2"});

        EXPECT_DOUBLE_EQ(result.interval, 2.0);
    }

    TEST_F(ParserTest, NframesOnlyWorks)
    {
        auto result = Parse({"cinebar",
                             temp_video.string(),
                             "--frames", "150"});

        EXPECT_EQ(result.nframes, 150);
    }

    TEST_F(ParserTest, MethodOptionParsesCorrectly)
    {
        // Test each valid method string
        std::vector<std::pair<std::string, cinebar_types::Method>> test_cases = {
            {"avg", cinebar_types::Method::Avg},
            {"smoothed", cinebar_types::Method::Smoothed},
            {"kmeans", cinebar_types::Method::KMeans},
            {"hsv", cinebar_types::Method::HSV},
            {"stripe", cinebar_types::Method::Stripe}};

        for (const auto &[str, expected] : test_cases)
        {
            auto result = Parse({"cinebar",
                                 temp_video.string(),
                                 "--method", str});

            EXPECT_EQ(result.method, expected) << "Failed for method string: " << str;
        }
    }

    TEST_F(ParserTest, InvalidMethodThrows)
    {
        EXPECT_THROW(
            Parse({"cinebar",
                   temp_video.string(),
                   "--method", "invalid_method"}),
            CLI::ValidationError);
    }
}