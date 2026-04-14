#include "frame_extractor.h"
#include "types.h"

namespace app_frame_extractor
{
    cv::Mat ExtractFrameStripe(const cv::Mat &frame, size_t width)
    {
        cv::Mat stripe;
        cv::resize(frame, stripe, cv::Size(static_cast<int>(width), frame.rows), 0, 0, cv::INTER_AREA);
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

        int dominant = 0;
        cv::minMaxIdx(hist, nullptr, nullptr, nullptr, &dominant);
        // direct conversion without Mat allocation
        cv::Vec3b hsv_color(dominant, 255, 255);
        cv::Vec3b bgr;
        cv::Mat tmp(1, 1, CV_8UC3, hsv_color);
        cv::cvtColor(tmp, tmp, cv::COLOR_HSV2BGR);

        return tmp.at<cv::Vec3b>(0, 0);
    }
}