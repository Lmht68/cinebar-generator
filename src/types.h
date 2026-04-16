#ifndef CINEBAR_TYPES_H_
#define CINEBAR_TYPES_H_

#include <opencv2/videoio.hpp>

#include <vector>
#include <map>
#include <string>
#include <optional>

namespace cinebar_types
{
    enum class Shape
    {
        Horizontal,
        Circular
    };

    inline const char *ToString(Shape shape)
    {
        switch (shape)
        {
        case Shape::Horizontal:
            return "Horizontal";
        case Shape::Circular:
            return "Circular";
        default:
            return "Unknown";
        };
    };

    const std::map<std::string, Shape> kArgShapeMap{
        {"horizontal", Shape::Horizontal},
        {"circular", Shape::Circular}};

    enum class Type
    {
        Color,
        Stripe
    };

    enum class Method
    {
        Avg,
        Smoothed,
        KMeans,
        Hist,
        HSV,
    };

    inline const char *ToString(Type type)
    {
        switch (type)
        {
        case Type::Color:
            return "Color";
        case Type::Stripe:
            return "Stripe";
        default:
            return "Unknown";
        }
    }

    inline const char *ToString(Method method)
    {
        switch (method)
        {
        case Method::Avg:
            return "Avg";
        case Method::Smoothed:
            return "Smoothed";
        case Method::KMeans:
            return "KMeans";
        case Method::Hist:
            return "Hist";
        case Method::HSV:
            return "HSV";
        default:
            return "Unknown";
        }
    }

    const std::map<std::string, Type> kArgTypeMap{
        {"color", Type::Color},
        {"stripe", Type::Stripe}};

    const std::map<std::string, Method> kArgMethodMap{
        {"avg", Method::Avg},
        {"smoothed", Method::Smoothed},
        {"kmeans", Method::KMeans},
        {"hist", Method::Hist},
        {"hsv", Method::HSV}};

    struct InputArgs
    {
        std::string input_video_path;
        std::string output_img_path;
        double interval = 0.0;
        size_t nframes = 0;
        size_t bar_w = 1;
        size_t width = 0;
        size_t height = 0;
        size_t start_frame = 0;
        size_t end_frame = 0;
        size_t segment_nframes = 0;
        Shape shape = Shape::Horizontal;
        bool trim = false;
        bool show_info = false;
        Type type = Type::Color;
        Method method = Method::Avg;
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
        size_t frame_count;
        size_t width;
        size_t height;
        std::optional<VideoBounds> bounds;
        BoxType box_type = BoxType::None;
    };
}

#endif
