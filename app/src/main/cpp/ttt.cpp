#include <jni.h>
#include <cstdio>
#include <string>
#include <android/log.h>

extern "C" {
//编码
#include "libavcodec/avcodec.h"
//封装格式处理
#include "libavformat/avformat.h"
//像素处理
#include "libswscale/swscale.h"
}

#define FFLOGI(FORMAT, ...) __android_log_print(ANDROID_LOG_INFO,"ffmpeg",FORMAT,##__VA_ARGS__);
#define FFLOGE(FORMAT, ...) __android_log_print(ANDROID_LOG_ERROR,"ffmpeg",FORMAT,##__VA_ARGS__);

extern "C" JNIEXPORT jint JNICALL
Java_com_zhangteng_myapplication_MainActivity_FFmpegTest(JNIEnv *env, jobject obj, jstring input,
                                                         jstring output) {
    //获取输入输出文件名
    const char *inputc = env->GetStringUTFChars((jstring) input, 0);
    const char *outputc = env->GetStringUTFChars((jstring) output, 0);
    //注册组件
    av_register_all();
    //打开文件
    AVFormatContext *avFormatContext = avformat_alloc_context();
    if (avformat_open_input(&avFormatContext, inputc, NULL, NULL) != 0) {
        FFLOGE("%s", "无法打开视频");
        return 0;
    }
    //找到流信息
    if (avformat_find_stream_info(avFormatContext, NULL) < 0) {
        FFLOGE("%s", "无法解析视频")
        return 1;
    }
    //查找视频流
    int i = 0;
    int video_stream_idx = -1;
    for (; i < avFormatContext->nb_streams; ++i) {
        if (avFormatContext->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
            video_stream_idx = i;
        }
    }
    //获取解码器
    AVCodecContext *avCodecContext = avFormatContext->streams[video_stream_idx]->codec;
    AVCodec *avCodec = avcodec_find_decoder(avCodecContext->codec_id);
    if (avCodec == NULL) {
        FFLOGE("%s", "无法解码")
        return 2;
    }
    //打开解码器
    if (avcodec_open2(avCodecContext, avCodec, NULL) < 0) {
        FFLOGE("%s", "解码器出错")
        return 3;
    }
    //编码数据
    AVPacket *avPacket;
    av_init_packet(avPacket);
    //像素数据
    AVFrame *avFrame = av_frame_alloc();
    AVFrame *yuvAVFrame = av_frame_alloc();

    //只有指定了AVFrame的像素格式、画面大小才能真正分配内存
    //缓冲区分配内存
    uint8_t *out_buffer = (uint8_t *) av_malloc(
            avpicture_get_size(AV_PIX_FMT_YUV420P, avCodecContext->width, avCodecContext->height));
    //初始化缓冲区
    avpicture_fill((AVPicture *) yuvAVFrame, out_buffer, AV_PIX_FMT_YUV420P, avCodecContext->width,
                   avCodecContext->height);

    //用于转码（缩放）的参数，转之前的宽高，转之后的宽高，格式等
    struct SwsContext *sws_ctx = sws_getContext(avCodecContext->width, avCodecContext->height,
                                                avCodecContext->pix_fmt,
                                                avCodecContext->width, avCodecContext->height,
                                                AV_PIX_FMT_YUV420P,
                                                SWS_BILINEAR, NULL, NULL, NULL);
    int len, got_frame;
    int frame_count = 0;
    FILE *outFile = fopen(outputc, "wb");
    if (outFile == NULL) {
        FFLOGE("%s", "目标文件为NULL");
        return 4;
    }
    while (av_read_frame(avFormatContext, avPacket) >= 0) {
        //只要视频压缩数据（根据流的索引位置判断）
        if (avPacket->stream_index == video_stream_idx) {
            len = avcodec_decode_video2(avCodecContext, avFrame, &got_frame, avPacket);
            if (len < 0) {
                FFLOGE("%s", "解码错误");
                return 5;
            }
            if (got_frame) {
                sws_scale(sws_ctx, avFrame->data, avFrame->linesize, 0, avFrame->height,
                          yuvAVFrame->data, yuvAVFrame->linesize);
                int y_size = avCodecContext->width * avCodecContext->height;
                fwrite(yuvAVFrame->data[0], 1, y_size, outFile);
                fwrite(yuvAVFrame->data[1], 1, y_size / 4, outFile);
                fwrite(yuvAVFrame->data[2], 1, y_size / 4, outFile);
                frame_count++;
                FFLOGE("解码第%d帧", frame_count);
            }
        }
        av_free_packet(avPacket);
    }
    fclose(outFile);
    av_frame_free(&avFrame);
    avcodec_close(avCodecContext);
    avformat_close_input(&avFormatContext);
    return -1;
}
extern "C" JNIEXPORT jint
Java_com_zhangteng_myapplication_MainActivity_JniCppAdd(JNIEnv *env, jobject obj, jint a, jint b) {
    int ia = a;
    int ib = b;
    printf("==================%d", a + b);
    return a + b;
}
extern "C" JNIEXPORT jint
Java_com_zhangteng_myapplication_MainActivity_JniCppSub(JNIEnv *env, jobject obj, jint a, jint b) {
    return a - b;
}

extern "C" JNIEXPORT jint JNICALL
Java_com_zhangteng_myapplication_MainActivity_getStringC(JNIEnv *env, jobject jobj) {
    jclass cls = env->GetObjectClass(jobj);
    jfieldID fid = env->GetFieldID(cls, "key", "I");
    jint jstr = env->GetIntField(jobj, fid);
    printf("c++:%d", jstr);
    env->SetIntField(jobj, fid, jstr + 1);
    return jstr;
}
