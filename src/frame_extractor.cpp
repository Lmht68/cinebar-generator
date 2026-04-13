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

    cv::Vec3b ExtractColorkMeans(const cv::Mat &frame)
    {
        cv::Mat small;
        cv::resize(frame, small, cv::Size(kResizeDim, kResizeDim), 0, 0, cv::INTER_AREA);
        // init centers (random pixels)
        std::vector<cv::Vec3f> centers(kNumkMean);
        std::vector<int> counts(kNumkMean, 0);

        cv::RNG rng(12345);
        for (int k = 0; k < kNumkMean; ++k)
        {
            int y = rng.uniform(0, small.rows);
            int x = rng.uniform(0, small.cols);
            centers[k] = small.at<cv::Vec3b>(y, x);
        }

        // streaming update
        int samples = 0;

        for (int y = 0; y < small.rows; ++y)
        {
            const cv::Vec3b *row = small.ptr<cv::Vec3b>(y);

            for (int x = 0; x < small.cols; ++x)
            {
                cv::Vec3f pixel = row[x];

                // find nearest center
                int best_k = 0;
                float best_dist = FLT_MAX;

                for (int k = 0; k < kNumkMean; ++k)
                {
                    cv::Vec3f diff = pixel - centers[k];
                    float dist = diff.dot(diff);

                    if (dist < best_dist)
                    {
                        best_dist = dist;
                        best_k = k;
                    }
                }

                // incremental update (running mean)
                counts[best_k]++;
                float lr = 1.0f / counts[best_k];
                centers[best_k] = centers[best_k] * (1.0f - lr) + pixel * lr;
                samples++;
                if (samples >= kMaxSamples)
                    break;
            }

            if (samples >= kMaxSamples)
                break;
        }

        // find dominant cluster
        auto dominant = std::distance(
            counts.begin(),
            std::max_element(counts.begin(), counts.end()));
        cv::Vec3f c = centers[dominant];
        return cv::Vec3b(cv::saturate_cast<uchar>(std::round(c[0])),
                         cv::saturate_cast<uchar>(std::round(c[1])),
                         cv::saturate_cast<uchar>(std::round(c[2])));
    }

    cv::Vec3b ExtractColorHistogram(const cv::Mat &frame)
    {
        cv::Mat small;
        cv::resize(frame, small, cv::Size(kResizeDim, kResizeDim), 0, 0, cv::INTER_AREA);
        std::vector<int> hist(kHistSize, 0);

        // build histogram
        for (int y = 0; y < small.rows; ++y)
        {
            const cv::Vec3b *row = small.ptr<cv::Vec3b>(y);

            for (int x = 0; x < small.cols; ++x)
            {
                const cv::Vec3b &p = row[x];
                int b = p[0] >> kShift;
                int g = p[1] >> kShift;
                int r = p[2] >> kShift;
                int idx = (r << 6) | (g << 3) | b; // packed index
                hist[idx]++;
            }
        }

        // find dominant bin
        auto max_idx = std::distance(hist.begin(), std::max_element(hist.begin(), hist.end()));
        // reconstruct color (center of bin)
        int r = (max_idx >> 6) & 7;
        int g = (max_idx >> 3) & 7;
        int b = (max_idx >> 0) & 7;
        // scale back to 0–255
        int step = 256 / kBinsPerChannel;

        return cv::Vec3b(
            b * step + step / 2,
            g * step + step / 2,
            r * step + step / 2);
    }

    cv::Vec3b ExtractDominantHue(const cv::Mat &frame)
    {
        cv::Mat hsv;
        cv::cvtColor(frame, hsv, cv::COLOR_BGR2HSV);
        const float *hist_ranges[] = {kHueRange};
        const int channels[] = {kHueChannel};

        // mask low saturation/value pixels
        cv::Mat mask;
        cv::inRange(
            hsv,
            cv::Scalar(kMinHue, kMinSaturation, kMinValue),
            cv::Scalar(kMaxHue, kMaxSaturation, kMaxValue),
            mask);
        // compute histogram
        cv::Mat hist;
        cv::calcHist(
            &hsv,
            1,
            channels,
            mask,
            hist,
            1,
            &kHueBins,
            hist_ranges);

        int max_idx[2] = {0, 0};
        cv::minMaxIdx(hist, nullptr, nullptr, nullptr, max_idx);
        int dominant_hue = max_idx[0];
        cv::Mat color_hsv(1, 1, CV_8UC3,
                          cv::Scalar(dominant_hue, 255, 255));
        cv::Mat color_bgr;
        cv::cvtColor(color_hsv, color_bgr, cv::COLOR_HSV2BGR);
        return color_bgr.at<cv::Vec3b>(0, 0);
    }
}