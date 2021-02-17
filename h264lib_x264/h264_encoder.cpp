#include <x264.h>
#include <stdio.h>
#include <string.h>
#include <vector>
#include "h264lib.h"
#include "x264_155.h"
#include "../types.h"

static void x264_log(void* p, int level, const char* fmt, va_list args)
{

    const char* psz_prefix;
    switch (level)
    {
        case X264_LOG_ERROR: psz_prefix = "error"; break;
        case X264_LOG_WARNING: psz_prefix = "warning"; break;
        case X264_LOG_INFO: psz_prefix = "info"; break;
        case X264_LOG_DEBUG: psz_prefix = "debug"; break;
        default: psz_prefix = "unknown"; break;
    }
  //  fprintf(stderr, "x264 [%s]: ", psz_prefix);
  //  vfprintf(stderr, fmt, args);
}


struct X264ContextImpl {
    x264_param_t param;
    x264_picture_t pic;
    x264_t* h;

    int m_srcWidth;
    int m_srcHeight;

    int m_iframeInterval;
    int m_idrInterval;
    int m_currentFrame;
    int m_keyQuant;
    int m_quant;

    int border_padding;

    bool abruptFrameChange;

    ////////////

    X264ContextImpl();
    int initEncoder();
    int initEncoder(int width, int height, int bitrate, int framerate);
    void destroyEncoder();
    int GetBorderPadding();
    int GetWidth();
    int GetHeight();
    void SetDimsFromBitmapSize(uint32_t width, uint32_t height);
    int processFrameRGB(void* imageData, uint8_t* out, int* type);
    int processFrame(void* imageData, uint8_t* out, int* type);
    void SetIDR_Interval(int interval);
    void SetIFrameInterval(int interval);
    void SetKeyQuant(int code);
    void SetQuant(int code);
    void ForceIDRFrame();
    int getFrameNumber();
    void setFrameNumber(int fn);

    int encode_nals(uint8_t* buf, int size, x264_nal_t* nals, int nnal);
    int encode_nals_live(uint8_t* buf, int size, x264_nal_t* nals, int nnal);
};


X264ContextImpl::X264ContextImpl()
{
    int result;

    x264_param_default(&param);

    param.i_frame_total = 0;
    param.i_level_idc = 0x29;
    //	param.i_log_level = X264_LOG_DEBUG;
    param.i_log_level = 0;
    param.pf_log = x264_log;

    param.rc.i_rc_method = X264_RC_CQP;

    param.analyse.inter = 0;
    param.b_deblocking_filter = true;

    param.analyse.b_transform_8x8 = 0;
    param.i_bframe = 0;
    param.i_bframe_adaptive = 0;
    param.b_interlaced = 0;
    param.i_cqm_preset = X264_CQM_FLAT;
    param.b_cabac = 0;

    param.b_repeat_headers = 1;
    param.b_aud = 0;
    param.i_sps_id = 0;

    m_iframeInterval = 12;
    m_idrInterval = 2;
    m_currentFrame = 0;
    m_keyQuant = 32;
    m_quant = 32;

    param.rc.i_qp_constant = m_keyQuant;

    border_padding = 0;

    abruptFrameChange = false;
}

void X264ContextImpl::destroyEncoder()
{
    x264_encoder_close(h);
    x264_picture_clean(&pic);
}

int X264ContextImpl::initEncoder()
{
    param.i_keyint_max = m_iframeInterval * m_idrInterval;
    param.i_keyint_min = m_iframeInterval * m_idrInterval;

    /*pX264DLLModule->x264_param_apply_profile( &param, "high" );*/

    /// might be able to leverage X264_CSP_VFLIP flag
    x264_picture_alloc(&pic, X264_CSP_I420, param.i_width, param.i_height);

    h = x264_encoder_open(&param);
    if (!h)
    {
        // No assertions!  Some layers may not be possible due to odd width or height.
        // We will skip these layers and mark them as defunct for the affected hardware.
        // assert(0 && "failed x264_encoder_open");
        return 0;
    }

    return 1;
}

int X264ContextImpl::initEncoder(int width, int height, int bitrate, int framerate)
{
    int result;
    border_padding = 0;

    x264_param_default(&param);
    result = x264_param_default_preset(
            &param, "ultrafast", "fastdecode,zerolatency");
    if (result != 0)
        return 0;

    param.i_frame_total = 0;
    param.i_level_idc = 0x29;
    param.i_log_level = 0;
    param.pf_log = x264_log;

    param.rc.i_rc_method = X264_RC_ABR;
    param.i_nal_hrd = X264_NAL_HRD_VBR;

    param.i_fps_num = framerate;
    param.i_fps_den = 1;
    param.rc.i_bitrate = bitrate;
    param.rc.i_vbv_buffer_size = param.rc.i_bitrate;
    param.rc.i_vbv_max_bitrate = param.rc.i_bitrate;

    param.analyse.inter = 0;
    param.b_deblocking_filter = true;

    param.analyse.b_transform_8x8 = 0;
    param.i_bframe = 0;
    param.i_bframe_adaptive = 0;
    param.b_interlaced = 0;
    param.i_cqm_preset = X264_CQM_FLAT;
    param.b_cabac = 0;
    param.b_repeat_headers = 1;
    param.b_aud = 0;
    param.i_sps_id = 0;

    param.b_annexb = 0;

    param.b_sliced_threads = 0;
    param.i_threads = 1;

    m_iframeInterval = framerate;
    m_idrInterval = 1;
    m_currentFrame = 0;
    abruptFrameChange = false;

    param.i_keyint_max = m_iframeInterval * m_idrInterval;
    param.i_keyint_min = m_iframeInterval * m_idrInterval;
    param.i_width = width;
    param.i_height = height;
    m_srcWidth = width;
    m_srcHeight = height;

    h = x264_encoder_open(&param);
    if (!h)
    {
        return 0;
    }

    x264_picture_alloc(&pic, X264_CSP_I420, param.i_width, param.i_height);

    return 1;
}


int X264ContextImpl::GetWidth()
{
    return m_srcWidth;
}

int X264ContextImpl::GetHeight()
{
    return m_srcHeight;
}

int X264ContextImpl::GetBorderPadding()
{
    return border_padding;
}

void X264ContextImpl::SetDimsFromBitmapSize(uint32_t width, uint32_t height)
{
    param.i_width = width + (2 * GetBorderPadding());
    param.i_height = height + (2 * GetBorderPadding());
    m_srcWidth = width;
    m_srcHeight = height;
}

void X264ContextImpl::SetIDR_Interval(int interval)
{
    m_idrInterval = interval;
}

void X264ContextImpl::SetIFrameInterval(int interval)
{
    m_iframeInterval = interval;
}

void X264ContextImpl::SetKeyQuant(int code)
{
    m_keyQuant = code;
    param.rc.i_qp_constant = m_keyQuant;
}

void X264ContextImpl::SetQuant(int code)
{
    m_quant = code;
}

int X264ContextImpl::getFrameNumber()
{
    return m_currentFrame;
}

void X264ContextImpl::setFrameNumber(int fn)
{
    m_currentFrame = fn; // external to x264 anyway.  Not critical but keeps iframes in
    // sync.
}

void X264ContextImpl::ForceIDRFrame()
{
    abruptFrameChange = true;
}


int my_live_x264_nal_encode(void* p_data, int* pi_data, int b_annexeb, x264_nal_t* nal)
{
    uint8_t* dst = (uint8_t*)p_data;
    uint8_t* src = nal->p_payload;
    src += 5;
    uint8_t* end = &nal->p_payload[nal->i_payload];
    int i_count = 0;

    if (b_annexeb)
    {
        *dst++ = 0x00;
        *dst++ = 0x00;
        *dst++ = 0x00;
        *dst++ = 0x01;
    }

    *dst++ = (0x00 << 7) | (nal->i_ref_idc << 5) | nal->i_type;

    while (src < end)
    {
        *dst++ = *src++;
    }

    *pi_data = dst - (uint8_t*)p_data;

    return *pi_data;
}

int my_x264_nal_encode(void* p_data, int* pi_data, int b_annexeb, x264_nal_t* nal)
{
    uint8_t* dst = (uint8_t*)p_data;
    uint8_t* src = nal->p_payload;
#if 129 <= DESIRED_X264_BUILD
    src += 5;
#endif
    uint8_t* end = &nal->p_payload[nal->i_payload];
    int i_count = 0;

    /* FIXME this code doesn't check overflow */

    if (b_annexeb)
    {
        /* long nal start code (we always use long ones)*/
        *dst++ = 0x00;
        *dst++ = 0x00;
        *dst++ = 0x00;
        *dst++ = 0x01;
    }

    /* nal header */
    *dst++ = (0x00 << 7) | (nal->i_ref_idc << 5) | nal->i_type;

    while (src < end)
    {
        if (i_count == 2 && *src <= 0x03)
        {
#if 129 <= DESIRED_X264_BUILD
            *dst++ = 0x03;
#endif
            i_count = 0;
        }
        if (*src == 0)
            i_count++;
        else
            i_count = 0;
        *dst++ = *src++;
    }
    *pi_data = dst - (uint8_t*)p_data;

    return *pi_data;
}

static int StripEmulationBytes(uint8_t* nalUnit, unsigned int& nalUnitSize)
{

    uint8_t* resultPtr = nalUnit;
    uint8_t* startingPtr = NULL;

    int decrementor = 0;

    for (startingPtr = nalUnit; (startingPtr - nalUnit) < nalUnitSize; ++startingPtr)
    {
        if ((startingPtr - nalUnit) < (nalUnitSize - 3) && *startingPtr == 0 &&
            *(startingPtr + 1) == 0 && *(startingPtr + 2) == 3 &&
            *(startingPtr + 3) > 0x03)
        {

            // if ( *(startingPtr+3) == 1 ) {
            //      *(startingPtr+3) = 9;
            //}

            *resultPtr = *startingPtr;
            resultPtr++;
            startingPtr++;
            *resultPtr = *startingPtr;
            resultPtr++;
            startingPtr++;
            decrementor++;
        }
        else
        {
            *resultPtr = *startingPtr;
            resultPtr++;
        }
    }

    nalUnitSize -= decrementor;

    return decrementor;
}

int X264ContextImpl::encode_nals_live(uint8_t* buf, int size, x264_nal_t* nals, int nnal)
{
    uint8_t* p = buf;
    int i;
    for (i = 0; i < nnal; i++)
    {
        x264_nal_t* nal = nals + i;
        unsigned int s = 0;
        if (nal->i_type != NAL_SEI)
        {
            if (param.b_annexb)
            {
                s = my_live_x264_nal_encode(p, &size, 1, nal);
            }
            else
            {
                s = my_live_x264_nal_encode(p, &size, 1, nal);
            }
        }
        if (s < 0)
        {
            return -1;
        }
        p += s;
    }
    return p - buf;
}

int X264ContextImpl::encode_nals(uint8_t* buf, int size, x264_nal_t* nals, int nnal)
{
    // OutputDebugString("****************************************\n");
    uint8_t* p = buf;
    int i;
    for (i = 0; i < nnal; i++)
    {
        x264_nal_t* nal = nals + i;
        unsigned int s = 0;
#if 67 == DESIRED_X264_BUILD
        unsigned int s = pX264DLLModule->x264_nal_encode(p, &size, 1, nal);
#elif 129 <= DESIRED_X264_BUILD
        unsigned int s = 0;
        // strip out initial SEI packet
        if (nal->i_type != NAL_SEI)
        {
            if (param.b_annexb)
            {
                memcpy(p, nal->p_payload, nal->i_payload);
                s = nal->i_payload;
                // StripEmulationBytes(p, s);
            }
            else
            {
                s = my_x264_nal_encode(p, &size, 0, nal);
            }
        }
#endif
        switch (nal->i_type)
        {
            /* Sequence Parameter Set & Program Parameter Set go in the
             * mp4 header so skip them here
             */
            case NAL_SPS:
                // OutputDebugString("NAL - SPS\n");
                break;
            case NAL_PPS:
                // OutputDebugString("NAL - PPS\n");
                break;
            case NAL_SLICE:
                // OutputDebugString("NAL - SLICE\n");
                break;
            case NAL_SLICE_IDR:
                // OutputDebugString("NAL - SLICE_IDR\n");
                break;
            case NAL_SEI:
                // OutputDebugString("NAL - SEI\n");
                break;
            default: break;
        }
        if (s < 0)
        {
            return -1;
        }
        p += s;
    }
    return p - buf;
}

// based off
// http://stackoverflow.com/questions/14018666/converting-rgb-image-to-yuv-using-c-programming

static void rgb2yuv420p(uint8_t* rgb,
                        uint8_t* y,
                        uint8_t* u,
                        uint8_t* v,
                        unsigned int width,
                        unsigned int height)
{
    unsigned int i = 0;
    unsigned int numpixels = width * height;
    unsigned int ui = 0;
    unsigned int vi = 0;
    unsigned int s = 0;
    const unsigned int colors = 3;

// assuming BGR input
#define sR (uint8_t)(rgb[s + 2])
#define sG (uint8_t)(rgb[s + 1])
#define sB (uint8_t)(rgb[s + 0])

    // assuming RGB input
    //#define sR (uint8_t)(rgb[s+0])
    //#define sG (uint8_t)(rgb[s+1])
    //#define sB (uint8_t)(rgb[s+2])

    for (int j = 0; j < height; j++)
        for (int k = 0; k < width; k++)
        {
            y[i] = (uint8_t)((66 * sR + 129 * sG + 25 * sB + 128) >> 8) + 16;

            if (0 == j % 2 && 0 == k % 2)
            {
                u[ui++] = (uint8_t)((-38 * sR - 74 * sG + 112 * sB + 128) >> 8) + 128;
                v[vi++] = (uint8_t)((112 * sR - 94 * sG - 18 * sB + 128) >> 8) + 128;
            }
            i++;
            s += colors;
        }
}

static void byte2yuv420p(uint8_t* byte,
                         uint8_t* y,
                         uint8_t* u,
                         uint8_t* v,
                         unsigned int width,
                         unsigned int height)
{
    memcpy(y, byte, width * height);
    memcpy(v, byte + width * height, width * height / 4);
    memcpy(u, byte + width * height + width * height / 4, width * height / 4);
}

int X264ContextImpl::processFrame(void* in, uint8_t* out, int* type)
{
    byte2yuv420p((uint8_t*)in,
                 (uint8_t*)pic.img.plane[0],
                 (uint8_t*)pic.img.plane[1],
                 (uint8_t*)pic.img.plane[2],
                 m_srcWidth,
                 m_srcHeight);

    if (0 == (m_currentFrame % (m_iframeInterval * m_idrInterval)) || abruptFrameChange)
    {
        abruptFrameChange = false;
        pic.i_type = X264_TYPE_IDR;
        if (type)
            *type = 0;
    }
    else if (0 == (m_currentFrame % (m_iframeInterval)))
    {
        pic.i_type = X264_TYPE_I;
        if (type)
            *type = 1;
    }
    else
    {
        pic.i_type = X264_TYPE_P;
        if (type)
            *type = 2;
    }

    pic.i_qpplus1 = 0;

    int i_nal = 0;
    x264_picture_t pic_out;
    x264_nal_t* nal;

    int ret = x264_encoder_encode(h, &nal, &i_nal, &pic, &pic_out);
    if (ret <= 0)
    {
        return 0;
    }

    int encode_size = encode_nals_live(out, m_srcWidth * m_srcHeight * 3, nal, i_nal);
    // int encode_size = encode_nals(out, m_srcWidth * m_srcHeight * 3, nal, i_nal);

    m_currentFrame++;
    return encode_size;
}

static void expandCopyRGB(uint8_t* destBits,
                          uint8_t* srcBits,
                          unsigned long srcWidth,
                          unsigned long srcHeight,
                          unsigned long destWidth,
                          unsigned long destHeight)
{
    destBits += 3 * destWidth * (destHeight - srcHeight) / 2;
    unsigned long cbWidthBorderSkip = 3 * (destWidth - srcWidth) / 2;
    for (int y = 0; y < srcHeight; ++y)
    {
        destBits += cbWidthBorderSkip;
        for (int x = 0; x < srcWidth; ++x)
        {
            destBits[0] = srcBits[0];
            destBits[1] = srcBits[1];
            destBits[2] = srcBits[2];
            srcBits += 3;
            destBits += 3;
        }
        destBits += cbWidthBorderSkip;
    }
}

int X264ContextImpl::processFrameRGB(void* imageData, uint8_t* out, int* type)
{

    x264_picture_alloc(
            &pic, X264_CSP_I420, param.i_width, param.i_height);

    const int borderSize = GetBorderPadding();
    if (borderSize)
    {
        std::vector<uint8_t> tempData(param.i_width * param.i_height * 3);
        expandCopyRGB(&tempData[0],
                      (uint8_t*)imageData,
                      m_srcWidth,
                      m_srcHeight,
                      param.i_width,
                      param.i_height);
        rgb2yuv420p((uint8_t*)&tempData[0],
                    (uint8_t*)pic.img.plane[0],
                    (uint8_t*)pic.img.plane[1],
                    (uint8_t*)pic.img.plane[2],
                    param.i_width,
                    param.i_height);
    }
    else
    {
        rgb2yuv420p((uint8_t*)imageData,
                    (uint8_t*)pic.img.plane[0],
                    (uint8_t*)pic.img.plane[1],
                    (uint8_t*)pic.img.plane[2],
                    m_srcWidth,
                    m_srcHeight);
    }
    /////
    /// force slice type
    if (0 == (m_currentFrame % (m_iframeInterval * m_idrInterval)) || abruptFrameChange)
    {
        abruptFrameChange = false;
        pic.i_type = X264_TYPE_IDR;
        if (type)
            *type = 0;
    }
    else if (0 == (m_currentFrame % (m_iframeInterval)))
    {
        pic.i_type = X264_TYPE_I;
        if (type)
            *type = 1;
    }
    else
    {
        pic.i_type = X264_TYPE_P;
        if (type)
            *type = 2;
    }
    //	pic.i_type = X264_TYPE_AUTO;

    if (X264_TYPE_P == pic.i_type)
    {
        pic.i_qpplus1 = m_quant + 1;
    }
    else
    {
        pic.i_qpplus1 = m_keyQuant + 1;
    }

    int i_nal = 0;
    x264_picture_t pic_out;
    x264_nal_t* nal;

    int ret = x264_encoder_encode(h, &nal, &i_nal, &pic, &pic_out);
#if 67 == DESIRED_X264_BUILD
    if (ret < 0)
    {
        return 0;
    }
#elif 129 <= DESIRED_X264_BUILD
    if (ret <= 0)
    {
        return 0;
    }
#endif
    int encode_size = encode_nals(out, m_srcWidth * m_srcHeight * 3, nal, i_nal);

    m_currentFrame++;
    return encode_size;
}

extern "C"
{
    void*  h264_encoder_create()
    {
        return reinterpret_cast<void*>(new X264ContextImpl());
    }

    void  h264_encoder_destroy(void* pContext)
    {
        delete reinterpret_cast<X264ContextImpl*>(pContext);
    }

    int  h264_encoder_get_frame_number(void* pContext)
    {
        X264ContextImpl* pImpl = reinterpret_cast<X264ContextImpl*>(pContext);
        return pImpl->getFrameNumber();
    }

    void  h264_encoder_set_frame_number(void* pContext, int fn)
    {
        X264ContextImpl* pImpl = reinterpret_cast<X264ContextImpl*>(pContext);
        pImpl->setFrameNumber(fn);
        pImpl->abruptFrameChange = true;
    }

    void  h264_encoder_set_dims_from_bitmap_size(void* pContext,
                                                        int width,
                                                        int height)
    {
        X264ContextImpl* pImpl = reinterpret_cast<X264ContextImpl*>(pContext);
        pImpl->SetDimsFromBitmapSize(width, height);
    }

    int  h264_encoder_initialize(void* pContext)
    {
        X264ContextImpl* pImpl = reinterpret_cast<X264ContextImpl*>(pContext);
        return pImpl->initEncoder();
    }

    int  h264_encoder_initialize_live(void* pContext, int w, int h, int br, int fr)
    {
        X264ContextImpl* pImpl = reinterpret_cast<X264ContextImpl*>(pContext);
        return pImpl->initEncoder(w, h, br, fr);
    }

    int  h264_encoder_initialize_highres(void* pContext, int res)
    {
        X264ContextImpl* pImpl = reinterpret_cast<X264ContextImpl*>(pContext);
        return pImpl->initEncoder();
    }

    void  h264_encoder_deinitialize(void* pContext)
    {
        //((h264Enc*)pContext)->destroyEncoder();
        // pImpl->destroyEncoder();
        X264ContextImpl* pImpl = reinterpret_cast<X264ContextImpl*>(pContext);
        pImpl->destroyEncoder();
    }

    int  h264_encoder_process_frame(void* pContext,
                                           void* pYUV,
                                           void* pBits,
                                           int* type)
    {
        X264ContextImpl* pImpl = reinterpret_cast<X264ContextImpl*>(pContext);
        return pImpl->processFrame(pYUV, (uint8_t*)pBits, type);
    }

    int  h264_encoder_process_frame_rgb(void* pContext,
                                               void* pRGB,
                                               void* pBits,
                                               int* type)
    {
        X264ContextImpl* pImpl = reinterpret_cast<X264ContextImpl*>(pContext);
        return pImpl->processFrameRGB(pRGB, (uint8_t*)pBits, type);
    }

    void  h264_encoder_set_high_bandwidth_rate_control(void* pContext, int flag)
    {
        //	((h264Enc*)pContext)->SetHighBandwidthRateControls(flag);
    }

    void  h264_encoder_set_motion_estimator_high_quality(void* pContext, int flag)
    {
        //	((h264Enc*)pContext)->SetMotionEstimatorHighQuality(flag);
    }

    void  h264_encoder_set_frame_rate(void* pContext, float rate)
    {
        X264ContextImpl* pImpl = reinterpret_cast<X264ContextImpl*>(pContext);
        // pImpl->m_iframeInterval = (int) rate;
    }

    void  h264_encoder_set_iframe_interval(void* pContext, int interval)
    {
        X264ContextImpl* pImpl = reinterpret_cast<X264ContextImpl*>(pContext);
        pImpl->SetIFrameInterval(interval);
    }

    void  h264_encoder_set_idr_interval(void* pContext, int interval)
    {
        X264ContextImpl* pImpl = reinterpret_cast<X264ContextImpl*>(pContext);
        pImpl->SetIDR_Interval(interval);
    }

    void  h264_encoder_force_idr_frame(void* pContext)
    {
        X264ContextImpl* pImpl = reinterpret_cast<X264ContextImpl*>(pContext);
        pImpl->ForceIDRFrame();
    }

    void  h264_encoder_set_key_quant(void* pContext, int code)
    {
        X264ContextImpl* pImpl = reinterpret_cast<X264ContextImpl*>(pContext);
        pImpl->SetKeyQuant(code);
    }

    void  h264_encoder_set_quant(void* pContext, int code)
    {
        X264ContextImpl* pImpl = reinterpret_cast<X264ContextImpl*>(pContext);
        pImpl->SetQuant(code);
    }
}
