#include "video_processor.h"
#include "utility.h"

#include <opencv2/opencv.hpp>

#include <vector>

namespace app_video_processor
{
    // --- Video Info
    cinebar_types::VideoInfo LoadVideoInfo(const std::string &video_path)
    {
        cv::VideoCapture video(video_path, cv::CAP_FFMPEG);

        if (!video.isOpened())
            throw std::runtime_error("video_processor: Input video not found: " + video_path);

        size_t frame_count = static_cast<size_t>(video.get(cv::CAP_PROP_FRAME_COUNT));
        double fps = video.get(cv::CAP_PROP_FPS);
        double duration = frame_count / fps;
        auto size = std::filesystem::file_size(video_path);
        size_t width = static_cast<size_t>(video.get(cv::CAP_PROP_FRAME_WIDTH));
        size_t height = static_cast<size_t>(video.get(cv::CAP_PROP_FRAME_HEIGHT));

        return {std::move(video), fps, duration, size, frame_count, width, height};
    }

    size_t NframesFromInterval(const size_t frame_count,
                               const double interval,
                               const double fps)
    {
        if (interval <= 0.0)
            throw std::invalid_argument("video_processor: Interval must be greater than 0");
        if (fps <= 0.0)
            throw std::runtime_error("video_processor: Invalid FPS value");
        if (interval < 1.0 / fps)
            throw std::invalid_argument("video_processor: Interval too small for video FPS");
        if (frame_count <= 0.0)
            throw std::runtime_error("video_processor: Invalid frame count value");

        double duration = frame_count / fps;
        return static_cast<size_t>(std::round(duration / interval));
    }
    // ---

    // --- Handle Letterbox: black bars on top and bottom, Pillarbox: black bars on left and right
    std::optional<cinebar_types::VideoBounds> DetectBounds(const cv::Mat &frame_grayed)
    {
        // downscale for faster processing
        cv::Mat frame_downscaled;
        cv::resize(frame_grayed, frame_downscaled, cv::Size(), kDownScaleFactor, kDownScaleFactor, cv::INTER_AREA);

        int h_downscaled = frame_downscaled.rows;
        int w_downscaled = frame_downscaled.cols;

        std::vector<double> row_black_ratio(h_downscaled, 0.0);
        std::vector<double> col_black_ratio(w_downscaled, 0.0);

        // count black pixels
        for (int y = 0; y < h_downscaled; y++)
        {
            for (int x = 0; x < w_downscaled; x++)
            {
                if (frame_downscaled.at<uchar>(y, x) < kDefaultThreshold)
                {
                    row_black_ratio[y] += 1;
                    col_black_ratio[x] += 1;
                }
            }
        }

        for (int y = 0; y < h_downscaled; y++)
            row_black_ratio[y] /= w_downscaled;

        for (int x = 0; x < w_downscaled; x++)
            col_black_ratio[x] /= h_downscaled;

        std::vector<bool> black_rows(h_downscaled);
        std::vector<bool> black_cols(w_downscaled);

        for (int i = 0; i < h_downscaled; i++)
            black_rows[i] = row_black_ratio[i] > kDefaultMinBlackRatio;

        for (int i = 0; i < w_downscaled; i++)
            black_cols[i] = col_black_ratio[i] > kDefaultMinBlackRatio;

        int top = 0;
        while (top < h_downscaled && black_rows[top])
            top++;

        int bottom = h_downscaled - 1;
        while (bottom >= 0 && black_rows[bottom])
            bottom--;

        int left = 0;
        while (left < w_downscaled && black_cols[left])
            left++;

        int right = w_downscaled - 1;
        while (right >= 0 && black_cols[right])
            right--;

        if (left >= right || top >= bottom)
            return std::nullopt;

        cinebar_types::VideoBounds bounds{
            static_cast<int>(left / kDownScaleFactor),
            static_cast<int>(top / kDownScaleFactor),
            static_cast<int>(right / kDownScaleFactor),
            static_cast<int>(bottom / kDownScaleFactor)};

        return bounds;
    }

    void CropImage(cv::Mat &frame, const cinebar_types::VideoBounds &bounds)
    {
        cv::Rect roi(
            bounds.left,
            bounds.top,
            bounds.right - bounds.left,
            bounds.bottom - bounds.top);

        frame = frame(roi);
    }

    bool DetermineVideoBounds(cinebar_types::VideoInfo &video_info,
                              cinebar_types::VideoBounds &bounds)
    {
        cv::VideoCapture &cap = video_info.capture;
        double interval = static_cast<double>(video_info.frame_count) / kDefaultSampleFrames;
        std::vector<cinebar_types::VideoBounds> detections;
        cv::Mat frame;

        for (int i = 0; i < kDefaultSampleFrames; i++)
        {
            int frame_id = static_cast<int>((i + 0.5) * interval);
            cap.set(cv::CAP_PROP_POS_FRAMES, frame_id);
            if (!cap.read(frame))
                continue;
            cv::Mat gray;
            cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
            auto detected = DetectBounds(gray);

            if (detected)
                detections.push_back(*detected);
        }

        if (detections.empty())
            return false;

        std::vector<int> lefts, tops, rights, bottoms;

        for (auto &b : detections)
        {
            lefts.push_back(b.left);
            tops.push_back(b.top);
            rights.push_back(b.right);
            bottoms.push_back(b.bottom);
        }

        auto median = [](std::vector<int> &v)
        {
            std::sort(v.begin(), v.end());
            return v[v.size() / 2];
        };

        cinebar_types::VideoBounds median_bounds{
            median(lefts),
            median(tops),
            median(rights),
            median(bottoms)};

        int crop_w = median_bounds.right - median_bounds.left;
        int crop_h = median_bounds.bottom - median_bounds.top;

        if (crop_w > video_info.width * kMinCropRatio &&
            crop_h > video_info.height * kMinCropRatio)
            return false;

        bounds = median_bounds;
        return true;
    }

    void DetectVideoBoxType(cinebar_types::VideoInfo &video_info,
                            ProgressCbk on_start,
                            ProgressCbk on_finish)
    {
        if (on_start)
            on_start();

        cinebar_types::VideoBounds bounds;
        bool has_bounds = !DetermineVideoBounds(video_info, bounds);

        if (on_finish)
            on_finish();

        if (!has_bounds)
            return;

        video_info.bounds = bounds;
        int crop_w = bounds.right - bounds.left;
        int crop_h = bounds.bottom - bounds.top;
        bool letterbox = crop_h < video_info.height;
        bool pillarbox = crop_w < video_info.width;

        if (letterbox && pillarbox)
            video_info.box_type = cinebar_types::BoxType::Windowbox;
        else if (letterbox)
            video_info.box_type = cinebar_types::BoxType::Letterbox;
        else if (pillarbox)
            video_info.box_type = cinebar_types::BoxType::Pillarbox;
        else
            video_info.box_type = cinebar_types::BoxType::None;
    }
    // ---

    std::vector<cv::Vec3b> ExtractColors(
        const cinebar_types::InputArgs &args,
        cinebar_types::VideoInfo &video_info,
        const app_frame_extractor::ColorFunc &extractor,
        ProgressUpdateCbk on_progress,
        ProgressCbk on_cancel)
    {
        cv::VideoCapture &cap = video_info.capture;
        const bool do_trim = args.trim && video_info.bounds;

        if (!cap.isOpened())
            throw std::runtime_error("video_processor: Failed to open video");

        cap.set(cv::CAP_PROP_POS_FRAMES, static_cast<double>(args.start_frame));
        const size_t segment_length = args.end_frame - args.start_frame + 1;
        const size_t step = segment_length / args.nframes;
        std::vector<cv::Vec3b> colors;
        colors.reserve(args.nframes);
        cv::Mat frame;
        size_t current = args.start_frame;
        size_t frame_counter = 0;

        while (current <= args.end_frame && frame_counter < args.nframes)
        {
            if (!cap.read(frame))
            {
                if (on_cancel)
                    on_cancel();
                throw std::runtime_error("video_processor: Failed to read frame ");
            }

            if (do_trim)
                CropImage(frame, *video_info.bounds);

            colors.push_back(extractor(frame));
            ++current;
            // update progress bar
            ++frame_counter;
            if (on_progress)
                on_progress(frame_counter, args.nframes);

            for (int i = 0; i < step - 1 && current <= args.end_frame; ++i)
            {
                if (!cap.grab())
                {
                    if (on_cancel)
                        on_cancel();
                    throw std::runtime_error("video_processor: Failed to grab frame ");
                }

                ++current;
            }
        }

        return colors;
    }

    std::vector<cv::Mat> ExtractStripes(
        const cinebar_types::InputArgs &args,
        cinebar_types::VideoInfo &video_info,
        ProgressUpdateCbk on_progress,
        ProgressCbk on_cancel)
    {
        cv::VideoCapture &cap = video_info.capture;
        const bool do_trim = args.trim && video_info.bounds;

        if (!cap.isOpened())
            throw std::runtime_error("video_processor: Failed to open video");

        cap.set(cv::CAP_PROP_POS_FRAMES, static_cast<double>(args.start_frame));
        const size_t segment_length = args.end_frame - args.start_frame + 1;
        const size_t step = segment_length / args.nframes;
        std::vector<cv::Mat> stripes;
        stripes.reserve(args.nframes);
        auto extractor = app_frame_extractor::getStripeFunction();
        cv::Mat frame;
        size_t current = args.start_frame;
        size_t frame_counter = 0;

        while (current <= args.end_frame && frame_counter < args.nframes)
        {
            if (!cap.read(frame))
            {
                if (on_cancel)
                    on_cancel();
                throw std::runtime_error("video_processor: Failed to read frame ");
            }

            if (do_trim)
                CropImage(frame, *video_info.bounds);

            stripes.push_back(extractor(frame, args.bar_w));
            ++current;
            // update progress bar
            ++frame_counter;
            if (on_progress)
                on_progress(frame_counter, args.nframes);

            // skip (step - 1) frames
            for (int i = 0; i < step - 1 && current <= args.end_frame; ++i)
            {
                if (!cap.grab())
                {
                    if (on_cancel)
                        on_cancel();
                    throw std::runtime_error("video_processor: Failed to grab frame ");
                }

                ++current;
            }
        }

        return stripes;
    }
}