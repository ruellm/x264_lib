
#include <map>
#include <mutex>
#include "x264_adapter.h"
#include "h264lib.h"

static std::map<int, void*> contextList_;
static int lastViewerId_ = 0;
static std::mutex mutex_;
namespace x264 {
    void *GetContext(int encoderId) {
        auto instance = contextList_.find(encoderId);
        if (instance == contextList_.end())
            return NULL;

        return instance->second;
    }

    int CreateEncoder() {
        mutex_.lock();
        auto id = ++lastViewerId_;
        mutex_.unlock();

        auto context = h264_encoder_create();
        contextList_.insert(std::make_pair(id, context));
        return id;
    }

    bool InitializeEncoder(int encoderId, int width, int height, int bitrate, int framerate) {
        auto context = GetContext(encoderId);
        if (context == NULL)
            return false;

        return (bool) h264_encoder_initialize_live(context, width, height, bitrate, framerate);
    }

    int EncoderProcessFrame(int encoderId, uint8_t *buf, uint8_t *out, int *type) {
        auto context = GetContext(encoderId);
        if (context == NULL)
            return 0;

        return h264_encoder_process_frame(context, buf, out, type);
    }

    void DestroyEncoder(int encoderId) {
        auto context = GetContext(encoderId);
        if (context == NULL)
            return;

        h264_encoder_destroy(context);
    }

    void Shutdown() {
        auto iter = contextList_.begin();
        while (iter != contextList_.end()) {
            h264_encoder_destroy(iter->second);
            iter->second = nullptr;
            iter = contextList_.erase(iter);
        }
    }

    int GetFrameNumber(int encoderId) {
        auto context = GetContext(encoderId);
        if (context == NULL)
            return 0;

        return h264_encoder_get_frame_number(context);
    }

    void SetFrameNumber(int encoderId, int fn) {
        auto context = GetContext(encoderId);
        if (context == NULL)
            return;

        h264_encoder_set_frame_number(context, fn);
    }

    void SetDimsFromBitmapSize(int encoderId, int width, int height) {
        auto context = GetContext(encoderId);
        if (context == NULL)
            return;

        h264_encoder_set_dims_from_bitmap_size(context, width, height);
    }

    int InitEncoder(int encoderId) {
        auto context = GetContext(encoderId);
        if (context == NULL)
            return 0;

        return h264_encoder_initialize(context);
    }

    int EncoderProcessFrameRGB(int encoderId, uint8_t *buf, uint8_t *out, int *type) {
        auto context = GetContext(encoderId);
        if (context == NULL)
            return 0;

        return h264_encoder_process_frame_rgb(context, buf, out, type);
    }

    void SetHighBandwidthRateControls(int encoderId, bool t) {
        auto context = GetContext(encoderId);
        if (context == NULL)
            return;

        h264_encoder_set_high_bandwidth_rate_control(context, (int) t);
    }

    void SetMotionEstimatorHighQuality(int encoderId, bool t) {
        auto context = GetContext(encoderId);
        if (context == NULL)
            return;

        h264_encoder_set_motion_estimator_high_quality(context, (int) t);
    }

    void SetFrameRate(int encoderId, float fr) {
        auto context = GetContext(encoderId);
        if (context == NULL)
            return;

        h264_encoder_set_frame_rate(context, fr);
    }

    void SetIFrameInterval(int encoderId, int i) {
        auto context = GetContext(encoderId);
        if (context == NULL)
            return;

        h264_encoder_set_iframe_interval(context, i);
    }

    void SetIDR_Interval(int encoderId, int i) {
        auto context = GetContext(encoderId);
        if (context == NULL)
            return;

        h264_encoder_set_idr_interval(context, i);
    }

    void ForceIDRFrame(int encoderId) {
        auto context = GetContext(encoderId);
        if (context == NULL)
            return;
        h264_encoder_force_idr_frame(context);
    }

    void SetKeyQuant(int encoderId, int i) {
        auto context = GetContext(encoderId);
        if (context == NULL)
            return;
        h264_encoder_set_key_quant(context, i);
        h264_encoder_set_quant(context, i + 10);
    }
}