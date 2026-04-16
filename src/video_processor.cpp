#include "video_processor.h"
#include "frame_extractor.h"

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

        int frame_count = video.get(cv::CAP_PROP_FRAME_COUNT);
        double fps = video.get(cv::CAP_PROP_FPS);
        double duration = frame_count / fps;
        auto size = std::filesystem::file_size(video_path);
        int width = video.get(cv::CAP_PROP_FRAME_WIDTH);
        int height = video.get(cv::CAP_PROP_FRAME_HEIGHT);

        return {std::move(video), fps, duration, size, frame_count, width, height};
    }

    int NframesFromInterval(const int frame_count,
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
        return static_cast<int>(std::round(duration / interval));
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

    void DetectVideoBoxType(cinebar_types::VideoInfo &video_info)
    {
        cinebar_types::VideoBounds bounds;
        bool has_bounds = !DetermineVideoBounds(video_info, bounds);
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

    template <auto Extractor>
    std::vector<cv::Vec3b> ExtractColors(
        const cinebar_types::InputArgs &args,
        cinebar_types::VideoInfo &video_info,
        std::atomic<int> &progress_current)
    {
        cv::VideoCapture &cap = video_info.capture;

        if (!cap.isOpened())
            throw std::runtime_error("video_processor: Failed to open video");

        cap.set(cv::CAP_PROP_POS_FRAMES, static_cast<double>(args.start_frame));
        const int step = (args.segment_nframes / args.nframes);
        std::vector<cv::Vec3b> colors;
        colors.reserve(args.nframes);
        cv::Mat frame;

        if (args.trim && video_info.bounds)
        {
            for (int i = 0; i < args.nframes; ++i)
            {
                if (!cap.read(frame))
                    break;
                CropImage(frame, *video_info.bounds);
                colors.push_back(Extractor(frame));
                progress_current = i + 1;
                for (int j = 0; j < step - 1; ++j)
                    cap.grab();
            }
        }
        else
        {
            for (int i = 0; i < args.nframes; ++i)
            {
                if (!cap.read(frame))
                    break;
                colors.push_back(Extractor(frame));
                progress_current = i + 1;
                for (int j = 0; j < step - 1; ++j)
                    cap.grab();
            }
        }

        return colors;
    }

    template <auto Extractor>
    std::vector<cv::Mat> ExtractStripes(
        const cinebar_types::InputArgs &args,
        cinebar_types::VideoInfo &video_info,
        std::atomic<int> &progress_current)
    {
        cv::VideoCapture &cap = video_info.capture;
        const bool do_trim = args.trim && video_info.bounds;

        if (!cap.isOpened())
            throw std::runtime_error("video_processor: Failed to open video");

        cap.set(cv::CAP_PROP_POS_FRAMES, static_cast<double>(args.start_frame));
        const int step = (args.segment_nframes / args.nframes);
        std::vector<cv::Mat> stripes;
        stripes.reserve(args.nframes);
        cv::Mat frame;

        if (args.trim && video_info.bounds)
        {
            for (int i = 0; i < args.nframes; ++i)
            {
                if (!cap.read(frame))
                    break;
                CropImage(frame, *video_info.bounds);
                stripes.push_back(Extractor(frame, args.bar_w));
                progress_current = i + 1;
                for (int j = 0; j < step - 1; ++j)
                    cap.grab();
            }
        }
        else
        {
            for (int i = 0; i < args.nframes; ++i)
            {
                if (!cap.read(frame))
                    break;
                stripes.push_back(Extractor(frame, args.bar_w));
                progress_current = i + 1;
                for (int j = 0; j < step - 1; ++j)
                    cap.grab();
            }
        }

        return stripes;
    }

    std::vector<cv::Vec3b> ExtractColorsDispatch(
        const cinebar_types::InputArgs &args,
        cinebar_types::VideoInfo &video_info,
        std::atomic<int> &progress_current)
    {
        switch (args.method)
        {
        case cinebar_types::Method::Avg:
            return ExtractColors<app_frame_extractor::ExtractColorMean>(
                args, video_info, progress_current);

        case cinebar_types::Method::Smoothed:
            return ExtractColors<app_frame_extractor::ExtractSmoothedColor>(
                args, video_info, progress_current);

        case cinebar_types::Method::KMeans:
            return ExtractColors<app_frame_extractor::ExtractColorkMeans>(
                args, video_info, progress_current);

        case cinebar_types::Method::Hist:
            return ExtractColors<app_frame_extractor::ExtractColorHistogram>(
                args, video_info, progress_current);

        case cinebar_types::Method::HSV:
            return ExtractColors<app_frame_extractor::ExtractDominantHue>(
                args, video_info, progress_current);

        default:
            throw std::invalid_argument("Invalid extraction method");
        }
    }

    std::vector<cv::Mat> ExtractStripesDispatch(const cinebar_types::InputArgs &args,
                                                cinebar_types::VideoInfo &video_info,
                                                std::atomic<int> &progress_current)
    {
        switch (args.method)
        {
        case cinebar_types::Method::Avg:
            return ExtractStripes<app_frame_extractor::ExtractMeanFrameStripe>(
                args, video_info, progress_current);

        case cinebar_types::Method::Smoothed:
            return ExtractStripes<app_frame_extractor::ExtractSmoothedFrameStripe>(
                args, video_info, progress_current);

        default:
            throw std::invalid_argument("Invalid extraction method");
        }
    }
}
