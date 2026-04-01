#ifndef CINEBAR_TYPES_H_
#define CINEBAR_TYPES_H_

#include <opencv2/videoio.hpp>

#include <vector>
#include <map>
#include <string>
#include <optional>

namespace cinebar_types
{
    enum class BarcodeShape
    {
        Horizontal,
        Circular
    };

    inline const char *ToString(BarcodeShape shape)
    {
        switch (shape)
        {
        case BarcodeShape::Horizontal:
            return "Horizontal";
        case BarcodeShape::Circular:
            return "Circular";
        default:
            return "Unknown";
        };
    };

    const std::map<std::string, BarcodeShape> kArgShapeMap{
        {"horizontal", BarcodeShape::Horizontal},
        {"circular", BarcodeShape::Circular}};

    enum class Method
    {
        Avg,
        Smoothed,
        KMeans,
        HSV,
        Stripe,
    };

    inline const char *ToString(Method method)
    {
        switch (method)
        {
        case Method::Avg:
            return "Average";
        case Method::Smoothed:
            return "Smoothed";
        case Method::KMeans:
            return "K-Means";
        case Method::HSV:
            return "HSV";
        case Method::Stripe:
            return "Stripe";
        default:
            return "Unknown";
        }
    }

    const std::map<std::string, Method> kArgMethodMap{
        {"avg", Method::Avg},
        {"smoothed", Method::Smoothed},
        {"kmeans", Method::KMeans},
        {"hsv", Method::HSV},
        {"stripe", Method::Stripe}};

    struct InputArgs
    {
        std::string input_video_path;
        std::string output_img_path;
        double interval = 0.0;
        int nframes = 0;
        int bar_w = 1;
        int width = 0;
        int height = 0;
        int start_frame = 0;
        int end_frame = -1; // -1 means till the end of the video
        BarcodeShape shape = BarcodeShape::Horizontal;
        bool trim = false;
        bool show_info = false;
        Method method = Method::Avg;
        int workers = 1;
        int bound_top = 0;
        int bound_bottom = 0;
        int bound_left = 0;
        int bound_right = 0;
        bool worker_mode = false; // Internal flag to indicate running in worker mode (not exposed to users)
        int worker_id = 0;        // Internal worker ID for worker mode (not exposed to users)
    };

    // Represents the detected bounds of the actual video content, excluding letterboxing
    struct VideoBounds
    {
        int left;
        int top;
        int right;
        int bottom;
    };

    enum class BoxType
    {
        None,
        Letterbox,
        Pillarbox,
        Windowbox
    };

    inline const char *ToString(BoxType type)
    {
        switch (type)
        {
        case BoxType::None:
            return "No letterboxing or pillarboxing detected";
        case BoxType::Letterbox:
            return "Letterboxing detected";
        case BoxType::Pillarbox:
            return "Pillarboxing detected";
        case BoxType::Windowbox:
            return "Windowboxing detected";
        default:
            return "Unknown";
        }
    }

    struct VideoInfo
    {
        cv::VideoCapture capture;
        double fps;
        double duration;
        std::uintmax_t size;
        int frame_count;
        int width;
        int height;
        std::optional<VideoBounds> bounds;
        BoxType box_type = BoxType::None;
    };
}

#endif