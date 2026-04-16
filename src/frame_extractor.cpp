#include "frame_extractor.h"
#include "types.h"

namespace app_frame_extractor
{
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

    inline float dist2(const cv::Vec3f &a, const cv::Vec3f &b)
    {
        cv::Vec3f d = a - b;
        return d.dot(d);
    }

    cv::Vec3b ExtractColorkMeans(const cv::Mat &frame)
    {
        cv::Mat small;
        cv::resize(frame, small, cv::Size(kResizeDim, kResizeDim), 0, 0, cv::INTER_AREA);

        std::vector<cv::Vec3f> centers(kNumkMean);
        std::vector<int> counts(kNumkMean, 0);
        cv::RNG rng(12345);

        for (int k = 0; k < kNumkMean; ++k)
        {
            centers[k] = small.at<cv::Vec3b>(
                rng.uniform(0, small.rows),
                rng.uniform(0, small.cols));
        }

        int samples = 0;

        for (int y = 0; y < small.rows && samples < kMaxSamples; ++y)
        {
            const cv::Vec3b *row = small.ptr<cv::Vec3b>(y);

            for (int x = 0; x < small.cols && samples < kMaxSamples; ++x)
            {
                cv::Vec3f pixel = row[x];

                int best_k = 0;
                float best_dist = FLT_MAX;

                for (int k = 0; k < kNumkMean; ++k)
                {
                    float d = dist2(pixel, centers[k]);
                    if (d < best_dist)
                    {
                        best_dist = d;
                        best_k = k;
                    }
                }

                counts[best_k]++;
                float lr = 1.0f / counts[best_k];
                centers[best_k] = centers[best_k] * (1.0f - lr) + pixel * lr;

                ++samples;
            }
        }

        int dominant = static_cast<int>(std::max_element(counts.begin(), counts.end()) - counts.begin());

        return cv::Vec3b(
            cv::saturate_cast<uchar>(centers[dominant][0]),
            cv::saturate_cast<uchar>(centers[dominant][1]),
            cv::saturate_cast<uchar>(centers[dominant][2]));
    }

    cv::Vec3b ExtractColorHistogram(const cv::Mat &frame)
    {
        cv::Mat small;
        cv::resize(frame, small, cv::Size(kResizeDim, kResizeDim), 0, 0, cv::INTER_AREA);

        std::vector<int> hist(kHistSize, 0);

        for (int y = 0; y < small.rows; ++y)
        {
            const cv::Vec3b *row = small.ptr<cv::Vec3b>(y);

            for (int x = 0; x < small.cols; ++x)
            {
                const cv::Vec3b &p = row[x];

                int idx =
                    ((p[2] >> kShift) << 6) |
                    ((p[1] >> kShift) << 3) |
                    ((p[0] >> kShift));

                hist[idx]++;
            }
        }

        int max_idx = static_cast<int>(std::max_element(hist.begin(), hist.end()) - hist.begin());
        int step = 256 / kBinsPerChannel;

        return cv::Vec3b(
            (max_idx & 7) * step + step / 2,
            ((max_idx >> 3) & 7) * step + step / 2,
            ((max_idx >> 6) & 7) * step + step / 2);
    }
    cv::Vec3b ExtractDominantHue(const cv::Mat &frame)
    {
        cv::Mat hsv;
        cv::cvtColor(frame, hsv, cv::COLOR_BGR2HSV);
        cv::Mat mask;
        cv::inRange(
            hsv,
            cv::Scalar(kMinHue, kMinSaturation, kMinValue),
            cv::Scalar(kMaxHue, kMaxSaturation, kMaxValue),
            mask);
        cv::Mat hist;
        const int channels[] = {0};
        const float *ranges[] = {kHueRange};

        cv::calcHist(&hsv, 1, channels, mask, hist, 1, &kHueBins, ranges);

        // Find max bin using STL
        const float *hist_ptr = hist.ptr<float>(0);
        int dominant_bin = static_cast<int>(
            std::max_element(hist_ptr, hist_ptr + kHueBins) - hist_ptr);
        // Convert bin → actual hue (0–179)
        int hue = static_cast<int>((dominant_bin + 0.5f) * 180.0f / kHueBins);
        // Build HSV color and convert back to BGR
        cv::Vec3b hsv_color(
            static_cast<uchar>(hue),
            255,
            255);
        cv::Mat tmp(1, 1, CV_8UC3, hsv_color);
        cv::cvtColor(tmp, tmp, cv::COLOR_HSV2BGR);
        return tmp.at<cv::Vec3b>(0, 0);
    }
}