#include "frame_extractor.h"

namespace app_frame_extractor
{
    cv::Mat ExtractColorStripe(const cv::Mat &frame, int width)
    {
        cv::Mat stripe;
        cv::resize(frame, stripe, cv::Size(width, frame.rows), 0, 0, cv::INTER_AREA);
        return stripe;
    }

    cv::Vec3b ExtractSmoothedColor(const cv::Mat &frame)
    {
        cv::Mat smoothed_frm;
        cv::resize(frame, smoothed_frm, cv::Size(1, 1), 0, 0, cv::INTER_AREA);
        return smoothed_frm.at<cv::Vec3b>(0, 0); // return the color
    }

    cv::Vec3b ExtractColorMean(const cv::Mat &frame)
    {
        cv::Scalar mean = cv::mean(frame);
        return cv::Vec3b(
            static_cast<uchar>(mean[0]),
            static_cast<uchar>(mean[1]),
            static_cast<uchar>(mean[2]));
    }

    cv::Vec3b ExtractColorkMeans(const cv::Mat &frame)
    {
        cv::Mat data;
        frame.convertTo(data, CV_32F);
        data = data.reshape(1, frame.rows * frame.cols); // Nx3
        cv::Mat labels, centers;
        cv::kmeans(
            data,
            app_frame_extractor::kNumkMean,
            labels,
            cv::TermCriteria(cv::TermCriteria::EPS + cv::TermCriteria::MAX_ITER, app_frame_extractor::kTermCriteriaMaxIter, app_frame_extractor::kTermCriteriaEpsilon),
            app_frame_extractor::kKMeansMaxAttempts,
            cv::KMEANS_PP_CENTERS,
            centers);

        // Count pixels per cluster
        std::vector<int> counts(app_frame_extractor::kNumkMean, 0);
        for (int i = 0; i < labels.rows; ++i)
            counts[labels.at<int>(i, 0)]++;

        int dominant_idx = static_cast<int>(std::distance(counts.begin(),
                                                          std::max_element(counts.begin(), counts.end())));
        cv::Vec3f c = centers.row(dominant_idx);
        return cv::Vec3b((uchar)c[0], (uchar)c[1], (uchar)c[2]);
    }

    cv::Vec3b ExtractDominantHue(const cv::Mat &frame)
    {
        cv::Mat hsv;
        cv::cvtColor(frame, hsv, cv::COLOR_BGR2HSV);

        int hist_size = 180;
        float range[] = {0, 180};
        const float *hist_ranges[] = {range};
        int channels[] = {0};

        cv::Mat hist;
        cv::calcHist(&hsv, 1, channels, cv::Mat(), hist, 1, &hist_size, hist_ranges);
        int max_idx[2] = {0, 0};
        cv::minMaxIdx(hist, nullptr, nullptr, nullptr, max_idx);
        int dominant_hue = max_idx[0];
        cv::Mat color_hsv(1, 1, CV_8UC3, cv::Scalar(dominant_hue, 255, 255));

        cv::Mat color_bgr;
        cv::cvtColor(color_hsv, color_bgr, cv::COLOR_HSV2BGR);

        return color_bgr.at<cv::Vec3b>(0, 0);
    }
}