//
// Created by Ruell Magpayo on 10/14/20.
//

#ifndef X264_WEBASM_X264_ADAPTER_H
#define X264_WEBASM_X264_ADAPTER_H

namespace x264 {
    int  CreateEncoder();

    bool  InitializeEncoder(int encoderId, int width, int height, int bitrate, int framerate);

    int  EncoderProcessFrame(int encoderId, uint8_t* buf, uint8_t* out, int* type);

    void  DestroyEncoder(int encoderId);

    void  Shutdown();

    int  GetFrameNumber(int encoderId);

    void  SetFrameNumber(int encoderId, int fn);

    void  SetDimsFromBitmapSize(int encoderId, int width, int height);

    int  InitEncoder(int encoderId);

    int  EncoderProcessFrameRGB(int encoderId, uint8_t* buf, uint8_t* out, int* type);

    void  SetHighBandwidthRateControls(int encoderId, bool t);

    void  SetMotionEstimatorHighQuality(int encoderId, bool t);

    void  SetFrameRate(int encoderId, float fr);

    void  SetIFrameInterval(int encoderId,  int i);

    void  SetIDR_Interval(int encoderId,  int i);

    void  ForceIDRFrame(int encoderId);

    void  SetKeyQuant(int encoderId, int i);

}
#endif //X264_WEBASM_X264_ADAPTER_H
