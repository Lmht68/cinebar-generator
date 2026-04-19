#ifndef CINEBAR_TYPES_H_
#define CINEBAR_TYPES_H_

#include <opencv2/videoio.hpp>

#include <vector>
#include <map>
#include <string>
#include <optional>

/**
 * @file types.h
 * @brief Shared data types and enum mappings
 */

namespace cinebar_types
{
    /**
     * @brief Layout used when rendering the final barcode image.
     */
    enum class Shape
    {
        Horizontal,
        Circular
    };

    /**
     * @brief Converts a barcode shape enum to a human-readable name.
     *
     * @param shape Shape value to stringify.
     * @return Constant string describing the shape.
     */
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

    /**
     * @brief Maps command-line shape tokens to enum values.
     */
    const std::map<std::string, Shape> kArgShapeMap{
        {"horizontal", Shape::Horizontal},
        {"circular", Shape::Circular}};

    /**
     * @brief Output representation extracted from sampled frames.
     */
    enum class Type
    {
        Color,
        Stripe
    };

    /**
     * @brief Strategy used to reduce a frame into a representative color or stripe.
     */
    enum class Method
    {
        Avg,
        Smoothed,
        KMeans,
        Hist,
        HSV,
    };

    /**
     * @brief Converts a barcode type enum to a human-readable name.
     *
     * @param type Type value to stringify.
     * @return Constant string describing the type.
     */
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

    /**
     * @brief Converts an extraction method enum to a human-readable name.
     *
     * @param method Method value to stringify.
     * @return Constant string describing the method.
     */
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

    /**
     * @brief Maps command-line barcode type tokens to enum values.
     */
    const std::map<std::string, Type> kArgTypeMap{
        {"color", Type::Color},
        {"stripe", Type::Stripe}};

    /**
     * @brief Maps command-line extraction method tokens to enum values.
     */
    const std::map<std::string, Method> kArgMethodMap{
        {"avg", Method::Avg},
        {"smoothed", Method::Smoothed},
        {"kmeans", Method::KMeans},
        {"hist", Method::Hist},
        {"hsv", Method::HSV}};

    /**
     * @brief Parsed command-line options and derived processing settings.
     */
    struct InputArgs
    {
        /** @brief Path to the source video file. */
        std::string input_video_path;
        /** @brief Path where the generated barcode image will be written. */
        std::string output_img_path;
        /** @brief Sampling interval in seconds between extracted frames. */
        double interval = 0.0;
        /** @brief Number of frames to sample from the selected segment. */
        int nframes = 0;
        /** @brief Width in pixels of each stripe in horizontal barcode mode. */
        int bar_w = 1;
        /** @brief Output image width in pixels. */
        int width = 0;
        /** @brief Output image height in pixels. */
        int height = 0;
        /** @brief First frame index included in the sampling segment. */
        int start_frame = 0;
        /** @brief Last frame index included in the sampling segment. */
        int end_frame = 0;
        /** @brief Number of frames inside the selected segment. */
        int segment_nframes = 0;
        /** @brief Output barcode layout. */
        Shape shape = Shape::Horizontal;
        /** @brief Enables cropping of detected letterbox or pillarbox bars. */
        bool trim = false;
        /** @brief Displays application information instead of processing a file. */
        bool show_info = false;
        /** @brief Output representation extracted from each sample. */
        Type type = Type::Color;
        /** @brief Color extraction algorithm used for each sample. */
        Method method = Method::Avg;
    };

    /**
     * @brief Bounds of the visible video content after removing black bars.
     */
    struct VideoBounds
    {
        /** @brief Inclusive left crop boundary in pixels. */
        int left;
        /** @brief Inclusive top crop boundary in pixels. */
        int top;
        /** @brief Inclusive right crop boundary in pixels. */
        int right;
        /** @brief Inclusive bottom crop boundary in pixels. */
        int bottom;
    };

    /**
     * @brief Type of black-bar boxing detected around the video content.
     */
    enum class BoxType
    {
        None,
        Letterbox,
        Pillarbox,
        Windowbox
    };

    /**
     * @brief Converts a detected boxing type to a descriptive message.
     *
     * @param type Boxing classification to stringify.
     * @return Constant string describing the detected box type.
     */
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

    /**
     * @brief Loaded video metadata and reusable capture state.
     */
    struct VideoInfo
    {
        /** @brief OpenCV capture handle for the input video. */
        cv::VideoCapture capture;
        /** @brief Source video frame rate in frames per second. */
        double fps;
        /** @brief Source video duration in seconds. */
        double duration;
        /** @brief Source file size in bytes. */
        std::uintmax_t size;
        /** @brief Total number of frames in the source video. */
        int frame_count;
        /** @brief Source frame width in pixels. */
        int width;
        /** @brief Source frame height in pixels. */
        int height;
        /** @brief Optional visible-content bounds after black-bar detection. */
        std::optional<VideoBounds> bounds;
        /** @brief Detected boxing classification for the input video. */
        BoxType box_type = BoxType::None;
    };
}

#endif
