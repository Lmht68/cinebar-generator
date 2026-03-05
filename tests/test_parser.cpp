#include "../src/parser.h"

#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

namespace fs = std::filesystem;

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

    app_parser::InputArgs Parse(const std::vector<std::string> &args)
    {
        std::vector<char *> argv;
        argv.reserve(args.size());

        for (const auto &s : args)
            argv.push_back(const_cast<char *>(s.c_str()));

        return app_parser::ParseArgs(
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
               "--nframes", "100"}),
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
                         "--nframes", "150"});

    EXPECT_EQ(result.nframes, 150);
}